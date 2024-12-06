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

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef SRC_NETWORK_H_
#define SRC_NETWORK_H_

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
        static Downloader instance;
        return instance;
    }

    void run();
    bool setUpCamp(const QString& levelDir);
    void setUrl(QUrl url);
    int getStatus();
    void setSaveFile(const QString& file);
    static size_t write_callback(
        void* contents,
        size_t size,
        size_t nmemb,
        void* userData);
    static int progress_callback(
        void* clientp,
        curl_off_t dltotal,
        curl_off_t dlnow,
        curl_off_t ultotal,
        curl_off_t ulnow);

 signals:
    void networkWorkTickSignal();
    void networkWorkErrorSignal(int status);

 private:
    void saveToFile(const QByteArray& data, const QString& filePath);
    QUrl url_m;
    QString file_m;
    QDir levelDir_m;
    qint32 status_m = 0;

    explicit Downloader(QObject* parent = nullptr) : QObject(parent),
        status_m(0) {
        curl_global_init(CURL_GLOBAL_DEFAULT);
    }

    ~Downloader() {
        curl_global_cleanup();
    }

    Q_DISABLE_COPY(Downloader)
};

#endif  // SRC_NETWORK_H_
