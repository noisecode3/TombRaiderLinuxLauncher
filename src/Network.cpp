/* TombRaiderLinuxLauncher
 * Martin Bångens Copyright (C) 2024
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include "Network.hpp"
#include <curl/curl.h>
#include <cstdio>
#include <iostream>
#include <string>
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>

namespace ssl = boost::asio::ssl;
using tcp = boost::asio::ip::tcp;

std::string get_ssl_certificate(const std::string& host) {
    boost::asio::io_context io_context;

    // Use SSLv23 context (it's compatible with all versions of SSL/TLS)
    ssl::context ssl_context(ssl::context::sslv23);

    // Restrict supported protocols to TLSv1.3 and TLSv1.2, these are no no
    ssl_context.set_options(ssl::context::no_sslv2 | ssl::context::no_sslv3);
    ssl_context.set_options(ssl::context::no_tlsv1 | ssl::context::no_tlsv1_1);

    // Resolver for HTTPS (default port 443)
    tcp::resolver resolver(io_context);
    tcp::resolver::results_type endpoints = resolver.resolve(host, "443");

    ssl::stream<tcp::socket> stream(io_context, ssl_context);
    SSL_set_tlsext_host_name(stream.native_handle(), host.c_str());

    try {
        boost::asio::connect(stream.lowest_layer(), endpoints);
        stream.handshake(ssl::stream_base::client);
    } catch (const boost::system::system_error& e) {
        std::cerr << "SSL handshake failed: " << e.what() << std::endl;
        return "";
    }

    // Get certificate
    X509* cert = SSL_get_peer_certificate(stream.native_handle());
    if (!cert) {
        std::cerr << "No certificate found." << std::endl;
        return "";
    }

    // Verify the certificate matches the host
    if (X509_check_host(cert, host.c_str(), host.length(), 0, nullptr) != 1) {
        std::cerr << "Hostname verification failed." << std::endl;
        X509_free(cert);
        return "";
    }

    BIO* bio = BIO_new(BIO_s_mem());
    PEM_write_bio_X509(bio, cert);
    char* cert_str = nullptr;
    qint64 cert_len = BIO_get_mem_data(bio, &cert_str);
    std::string cert_buffer(cert_str, cert_len);

    // Clean up
    BIO_free(bio);
    X509_free(cert);

    return cert_buffer;
}

struct WriteData {
    Downloader* downloader;
    FILE* file;
};

bool Downloader::setUpCamp(const QString& levelDir) {
    QFileInfo levelPathInfo(levelDir);
    if (levelPathInfo.isDir()) {
        levelDir_m.setPath(levelDir);
        return true;
    }
    return false;
}

size_t Downloader::write_callback(
    void* contents,
    size_t size,
    size_t nmemb,
    void* userData) {
    WriteData* data = reinterpret_cast<WriteData*>(userData);
    return fwrite(contents, size, nmemb, data->file);
}

int Downloader::progress_callback(
    void* clientp,
    curl_off_t dltotal,
    curl_off_t dlnow,
    curl_off_t ultotal,
    curl_off_t ulnow) {
    if (dltotal > 0) {
        double progress = static_cast<double>(dlnow)
            / static_cast<double>(dltotal) * 50.0;

        // Emit signal only if progress has increased by at least 1%
        static int lastEmittedProgress = 0;
        if (static_cast<int>(progress) == 0) lastEmittedProgress = 0;
        if (static_cast<int>(progress) > lastEmittedProgress) {
            static Downloader& instance = Downloader::getInstance();
            emit instance.networkWorkTickSignal();
            QCoreApplication::processEvents();
            lastEmittedProgress = static_cast<int>(progress);
        }
    }
    return 0;
}

void Downloader::setUrl(QUrl url) {
    url_m = url;
}

void Downloader::setSaveFile(const QString& file) {
    file_m = file;
}

int Downloader::getStatus() {
    return status_m;
}

void Downloader::saveToFile(const QByteArray& data, const QString& filePath) {
    QFileInfo fileInfo(filePath);

    if (fileInfo.exists() && !fileInfo.isFile()) {
        qDebug() << "Error: The zip path is not a regular file." << filePath;
        return;
    }

    QFile file(filePath);

    if (file.open(QIODevice::WriteOnly)) {  // flawfinder: ignore
        file.write(data);
        file.close();
        qDebug() << "Data saved to file:" << filePath;
    } else {
        qDebug() << "Error saving data to file:" << file.errorString();
    }
}

void Downloader::run() {
    if (url_m.isEmpty() || file_m.isEmpty() || levelDir_m.isEmpty())
        return;

    QString urlString = url_m.toString();
    QByteArray byteArray = urlString.toUtf8();
    const char* url_cstring = byteArray.constData();

    const QString filePath = levelDir_m.absolutePath() +
            QDir::separator() + file_m;

    QFileInfo fileInfo(filePath);

    if (fileInfo.exists() && !fileInfo.isFile()) {
        qDebug() << "Error: The zip path is not a regular file." << filePath;
        return;
    }

    FILE* file = fopen(filePath.toUtf8(), "wb");  // flawfinder: ignore
    if (!file) {
        qDebug() << "Error opening file for writing:" << filePath;
        return;
    }

    WriteData writeData = { this, file };

    CURL* curl = curl_easy_init();
    if (curl) {
        std::string cert_buffer = get_ssl_certificate("www.trle.net");

        // Set up the in-memory blob for curl to use
        curl_blob blob;
        blob.data = cert_buffer.data();
        blob.len = cert_buffer.size();
        blob.flags = CURL_BLOB_COPY;

        curl_easy_setopt(curl, CURLOPT_URL, url_cstring);
        curl_easy_setopt(curl, CURLOPT_CAINFO_BLOB, &blob);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION,
                Downloader::write_callback);

        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &writeData);

        // Follow redirects
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

        // Enable progress meter
        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
        curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION,
                Downloader::progress_callback);

        curl_easy_setopt(curl, CURLOPT_XFERINFODATA, nullptr);

        // Perform the download
        CURLcode res = curl_easy_perform(curl);

        if (res != CURLE_OK) {
            status_m = 1;
            qDebug() << "CURL failed:" << curl_easy_strerror(res);
            // we need to catch any of those that seem inportant here to the GUI
            // and reset GUI state
            // https://curl.se/libcurl/c/libcurl-errors.html
            if (res == 6 || res == 7 || res == 28 || res == 35) {
                emit this->networkWorkErrorSignal(1);
                QCoreApplication::processEvents();
            } else if (res == CURLE_PEER_FAILED_VERIFICATION) {
                emit this->networkWorkErrorSignal(2);
                QCoreApplication::processEvents();
            } else {
                emit this->networkWorkErrorSignal(3);
                QCoreApplication::processEvents();
            }
        } else {
            status_m = 0;
            qDebug() << "Downloaded successfully";
        }
        curl_easy_cleanup(curl);
    }
    fclose(file);
}
