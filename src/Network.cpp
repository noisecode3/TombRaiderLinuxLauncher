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

#include <curl/curl.h>
#include <cstdio>
#include <iostream>
#include <string>
#include "../src/Network.hpp"
#include "../src/Path.hpp"

void Downloader::setUrl(QUrl url) {
    m_url = url;
}

void Downloader::setSaveFile(Path file) {
    m_saveFile = file;
}

int Downloader::getStatus() {
    return m_status;
}

void Downloader::run() {
    if (m_url.isEmpty() || m_saveFile.getRoot() != Path::resource) {
        m_status = 3;  // object error
    } else {
        m_status = 0;
        qDebug() << "m_url: " << m_url.toString();
        qDebug() << "m_filePath: " << m_saveFile.get();

        if (m_saveFile.isSymLink()) {
            m_status = 5;  // symlink error
        } else if (m_saveFile.exists() && !m_saveFile.isFile()) {
            qDebug()
                    << "Error: The zip path is not a regular file :"
                                                            << m_saveFile.get();
            m_status = 2;  // file error

        } else {
            QFile file(m_saveFile.get());
            if (!file.open(QIODevice::WriteOnly)) {  // flawfinder: ignore
                qDebug()
                        << "Error opening file for writing:"
                                                            << m_saveFile.get();
                m_status = 4;  // opening error
            } else {
                QByteArray byteArray = m_url.toString().toUtf8();
                const char* url_cstring = byteArray.constData();
                runConnect(&file, url_cstring);
                file.close();
            }
        }
    }
}

void Downloader::runConnect(QFile *file, const std::string& url) {
    CURL* curl = curl_easy_init();
    if (!curl) {
        std::cerr << "Failed to initialize CURL\n";
    } else {
        CURLcode status = CURLE_OK;
        const std::string trle_domain = "https://www.trle.net";
        const std::string trcustoms_domain = "https://trcustoms.org";

        // Set the URL securely
        if (status == CURLE_OK) {
            status = curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        }

        // Enable SSL verification
        if (status == CURLE_OK) {
            status = curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
        }

        // Set TLS v1.3 version
        if (status == CURLE_OK) {
            status = curl_easy_setopt(curl, CURLOPT_SSLVERSION,
                                                CURL_SSLVERSION_TLSv1_3);
        }

        // Secure Public Key Pinning
        if (status == CURLE_OK) {
            if (url.compare(0, trle_domain.size(), trle_domain) == 0) {
                status = curl_easy_setopt(curl, CURLOPT_PINNEDPUBLICKEY,
                    "sha256//Cf9LIgksE6bCchPPBNor8kpbfymBIOHhGgmxMZea57U=");
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
