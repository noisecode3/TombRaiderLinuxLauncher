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

bool Downloader::setUpCamp(const QString& levelDir) {
    QFileInfo levelPathInfo(levelDir);
    if (levelPathInfo.isDir()) {
        m_levelDir.setPath(levelDir);
        return true;
    }
    return false;
}

void Downloader::setUrl(QUrl url) {
    m_url = url;
}

void Downloader::setSaveFile(const QString& file) {
    m_file = file;
}

int Downloader::getStatus() {
    return m_status;
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
    if (m_url.isEmpty() || m_file.isEmpty() || m_levelDir.isEmpty())
        return;

    qDebug() << "m_url: " << m_url.toString();
    qDebug() << "m_file: " << m_file;
    qDebug() << "m_levelDir: " << m_levelDir.absolutePath();

    QString urlString = m_url.toString();
    QByteArray byteArray = urlString.toUtf8();
    const char* url_cstring = byteArray.constData();

    const QString filePath = QString("%1%2%3")
        .arg(m_levelDir.absolutePath(), QDir::separator(), m_file);

    qDebug() << "filePath: " << filePath;

    QFileInfo fileInfo(filePath);

    if (fileInfo.exists() && !fileInfo.isFile()) {
        qDebug() << "Error: The zip path is not a regular file." << filePath;
        return;
    }

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {  // flawfinder: ignore
        qDebug() << "Error opening file for writing:" << filePath;
        return;
    }

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

        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
        curl_easy_setopt(curl, CURLOPT_PINNEDPUBLICKEY,
            "sha256//7WRPcNY2QpOjWiQSLbiBu/9Og69JmzccPAdfj2RT5Vw=");

        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION,
            +[](const void* buf, size_t size, size_t nmemb, void* data)
            -> size_t {
                QFile* file = static_cast<QFile*>(data);
                if (file->isOpen()) {
                    return file->write(static_cast<const char*>(buf),
                            size * nmemb);
                }
                return 0;
            });

        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &file);

        // Follow redirects
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

        // Enable progress meter
        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);

        curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION,
            +[](void* clientp, curl_off_t dltotal, curl_off_t dlnow,
                curl_off_t ultotal, curl_off_t ulnow) -> int {
                Downloader& downloader = Downloader::getInstance();

                if (dltotal > 0) {
                    int& lastEmittedProgress =
                        downloader.m_lastEmittedProgress;

                    double progress = static_cast<double>(dlnow)
                        / static_cast<double>(dltotal) * 50.0;

                    if (static_cast<int>(progress) == 0) {
                        lastEmittedProgress = 0;
                    }
                    if (static_cast<int>(progress) > lastEmittedProgress) {
                        emit downloader.networkWorkTickSignal();
                        QCoreApplication::processEvents();
                        lastEmittedProgress = static_cast<int>(progress);
                    }
                }
                return 0;
            });

        curl_easy_setopt(curl, CURLOPT_XFERINFODATA, nullptr);

        // Perform the download
        CURLcode res = curl_easy_perform(curl);

        if (res != CURLE_OK) {
            m_status = 1;
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
            m_status = 0;
            qDebug() << "Downloaded successfully";
        }
        curl_easy_cleanup(curl);
    }
    file.close();
}
