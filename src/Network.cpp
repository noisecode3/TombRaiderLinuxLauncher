/* TombRaiderLinuxLauncher
 * Martin Bångens Copyright (C) 2025
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include "../src/Network.hpp"
#include <curl/curl.h>
#include <cstdio>
#include <iostream>
#include <vector>
#include <string>
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>

namespace ssl = boost::asio::ssl;
using tcp = boost::asio::ip::tcp;

std::string get_ssl_certificate(const std::string& host) {
    bool status = true;
    std::string cert_buffer;
    boost::asio::io_context io_context;

    // Use SSLv23 context (it's compatible with all versions of SSL/TLS)
    ssl::context ssl_context(ssl::context::sslv23);

    // Restrict supported protocol to TLSv1.3
    ssl_context.set_options(ssl::context::no_sslv2 | ssl::context::no_sslv3);
    ssl_context.set_options(ssl::context::no_tlsv1 | ssl::context::no_tlsv1_1);
    ssl_context.set_options(ssl::context::no_tlsv1_2);

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
        status = false;
    }

    if (status) {
        // Get certificate
        X509* cert = SSL_get_peer_certificate(stream.native_handle());
        if (!cert) {
            std::cerr << "No certificate found." << std::endl;
            status = false;
        }
        if (status) {
            // Verify the certificate matches the host
            if (X509_check_host(
                        cert, host.c_str(), host.length(), 0, nullptr) != 1) {
                std::cerr << "Hostname verification failed." << std::endl;
                status = false;
            }
            if (status) {
                BIO* bio = BIO_new(BIO_s_mem());
                PEM_write_bio_X509(bio, cert);
                char* cert_str = nullptr;
                qint64 cert_len = BIO_get_mem_data(bio, &cert_str);
                cert_buffer = std::string(cert_str, cert_len);
                BIO_free(bio);
            }
        }
        X509_free(cert);
    }
    return cert_buffer;
}

bool Downloader::setUpCamp(const QString& levelDir) {
    bool status = false;
    QFileInfo levelPathInfo(levelDir);
    if (levelPathInfo.isDir() == true) {
        m_levelDir.setPath(levelDir);
        status = true;
    }
    return status;
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

void Downloader::run() {
    if (m_url.isEmpty() || m_file.isEmpty() || m_levelDir.isEmpty()) {
        m_status = 3;  // object error
    } else {
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
            qDebug() << "Error: The zip path is not a regular file :"
                << filePath;
            m_status = 2;  // file error
        } else {
            QFile file(filePath);
            if (!file.open(QIODevice::WriteOnly)) {  // flawfinder: ignore
                qDebug() << "Error opening file for writing:" << filePath;
                m_status = 2;
            } else {
                connect(&file, url_cstring);
            }

            file.close();
        }
    }
}

void Downloader::connect(QFile *file, const std::string& url) {
    CURL* curl = curl_easy_init();
    if (!curl) {
        std::cerr << "Failed to initialize CURL\n";
    } else {
        CURLcode status = CURLE_OK;
        const std::string trle_domain = "https://www.trle.net";
        const std::string trcustoms_domain = "https://trcustoms.org";

        // Securely determine which domain is being accessed
        if (url.compare(0, trle_domain.size(), trle_domain) == 0) {
            std::string cert_buffer = get_ssl_certificate("www.trle.net");
            std::vector<char>
                cert_buffer_vec(cert_buffer.begin(), cert_buffer.end());

            curl_blob blob;
            blob.data = cert_buffer_vec.data();
            blob.len = cert_buffer_vec.size();
            blob.flags = CURL_BLOB_COPY;

            status = curl_easy_setopt(curl, CURLOPT_CAINFO_BLOB, &blob);
        }

        // Set the URL securely
        if (status == CURLE_OK) {
            status = curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        }

        // Enable SSL verification
        if (status == CURLE_OK) {
            status = curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
        }

        // Secure Public Key Pinning
        if (status == CURLE_OK) {
            if (url.compare(0, trle_domain.size(), trle_domain) == 0) {
                status = curl_easy_setopt(curl, CURLOPT_PINNEDPUBLICKEY,
                    "sha256//87UOkLPD3rxuss0AWbDbdDI0NPZvT6xpQHkQGxHpWE8=");
            } else if (url.compare(
                    0, trcustoms_domain.size(), trcustoms_domain) == 0) {
                qDebug() << "trcustoms dont pinn key.";
            } else {
                status = CURLE_SSL_CERTPROBLEM;
                qDebug() << "CURL: Unknown host, have no publick key for it.";
            }
        }

        if (status == CURLE_OK) {
            status = curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION,
                +[](const void* buf, size_t size, size_t nmemb, void* data)
                -> size_t {
                    size_t writtenSize = 0;
                    QFile* file = static_cast<QFile*>(data);
                    if (file->isOpen() == true) {
                        writtenSize = file->write(static_cast<const char*>(buf),
                                size * nmemb);
                    }
                    // cppcheck-suppress misra-c2012-15.5
                    return writtenSize;
            });
        }

        // The file object to save to
        if (status == CURLE_OK) {
            status = curl_easy_setopt(curl, CURLOPT_WRITEDATA, file);
        }

        // Follow redirects
        if (status == CURLE_OK) {
            status = curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        }

        // Enable progress meter
        if (status == CURLE_OK) {
            status = curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
        }

        if (status == CURLE_OK) {
            status = curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION,
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
                    // cppcheck-suppress misra-c2012-15.5
                    return 0;
            });
        }

        if (status == CURLE_OK) {
            status = curl_easy_setopt(curl, CURLOPT_XFERINFODATA, nullptr);
        }

        // Perform the download
        if (status == CURLE_OK) {
            status = curl_easy_perform(curl);
        }

        if (status != CURLE_OK) {
            m_status = 1;  // curl error
            qDebug() << "CURL failed:" << curl_easy_strerror(status);
            // https://curl.se/libcurl/c/libcurl-errors.html
            if ((status == 6) || (status == 7) ||
                (status == 28) || (status == 35)) {
                emit this->networkWorkErrorSignal(1);
                QCoreApplication::processEvents();
            } else if (status == CURLE_PEER_FAILED_VERIFICATION) {
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
}
