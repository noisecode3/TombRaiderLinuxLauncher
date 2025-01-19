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

#ifndef SRC_NETWORK_HPP_
#define SRC_NETWORK_HPP_

#include <QObject>
#include <QUrl>
#include <QFile>
#include <QDir>
#include <QtCore>
#include <QDebug>
#include <curl/curl.h>

class Downloader : public QObject {
    Q_OBJECT

 public:
    static Downloader& getInstance() {
        // cppcheck-suppress threadsafety-threadsafety
        static Downloader instance;
        return instance;
    }

    void run();
    bool setUpCamp(const QString& levelDir);
    void setUrl(QUrl url);
    int getStatus();
    void setSaveFile(const QString& file);

 signals:
    void networkWorkTickSignal();
    void networkWorkErrorSignal(int status);

 private:
    void saveToFile(const QByteArray& data, const QString& filePath);
    void connect(QFile *file, const char*);
    QUrl m_url;
    QString m_file;
    QDir m_levelDir;
    qint32 m_status;
    int m_lastEmittedProgress;

    Downloader() :
        m_url(""),
        m_file(""),
        m_levelDir(""),
        m_status(0),
        m_lastEmittedProgress(0) {
        curl_global_init(CURL_GLOBAL_DEFAULT);
    }

    ~Downloader() {
        curl_global_cleanup();
    }

    Q_DISABLE_COPY(Downloader)
};

#endif  // SRC_NETWORK_HPP_
