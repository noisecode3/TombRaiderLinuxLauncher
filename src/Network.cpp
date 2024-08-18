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

#include "Network.h"

struct WriteData
{
    Downloader* downloader;
    FILE* file;
};

bool Downloader::setUpCamp(const QString& levelDir)
{
    QFileInfo levelPathInfo(levelDir);
    if (levelPathInfo.isDir())
    {
        levelDir_m.setPath(levelDir);
        return true;
    }
    return false;
}

size_t Downloader::write_callback(
    void* contents,
    size_t size,
    size_t nmemb,
    void* userData)
{
    WriteData* data = reinterpret_cast<WriteData*>(userData);
    return fwrite(contents, size, nmemb, data->file);
}

int Downloader::progress_callback(
    void* clientp,
    curl_off_t dltotal,
    curl_off_t dlnow,
    curl_off_t ultotal,
    curl_off_t ulnow)
{
    if (dltotal > 0)
    {
        double progress = static_cast<double>(dlnow)
            / static_cast<double>(dltotal) * 50.0;

        // Emit signal only if progress has increased by at least 1%
        static int lastEmittedProgress = 0;
        if (static_cast<int>(progress) > lastEmittedProgress)
        {
            static Downloader& instance = Downloader::getInstance();
            emit instance.networkWorkTickSignal();
            lastEmittedProgress = static_cast<int>(progress);
        }
    }
    return 0;
}

void Downloader::setUrl(QUrl url)
{
    url_m = url;
}

void Downloader::setSaveFile(const QString& file)
{
    file_m = file;
}

int Downloader::getStatus()
{
    return status_m;
}

void Downloader::saveToFile(const QByteArray& data, const QString& filePath)
{
    QFile file(filePath);
    if (file.open(QIODevice::WriteOnly))
    {
        file.write(data);
        file.close();
        qDebug() << "Data saved to file:" << filePath;
    }
    else
    {
        qDebug() << "Error saving data to file:" << file.errorString();
    }
}

void Downloader::run()
{
    if (url_m.isEmpty() || file_m.isEmpty() || levelDir_m.isEmpty())
        return;

    QString urlString = url_m.toString();
    QByteArray byteArray = urlString.toUtf8();
    const char* url_cstring = byteArray.constData();

    QString filePath = levelDir_m.absolutePath() + QDir::separator() + file_m;

    FILE* file = fopen(filePath.toUtf8().constData(), "wb");
    if (!file)
    {
        qDebug() << "Error opening file for writing:" << filePath;
        return;
    }

    WriteData writeData = { this, file };

    CURL* curl = curl_easy_init();
    if (curl)
    {
        curl_easy_setopt(curl, CURLOPT_URL, url_cstring);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, Downloader::write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &writeData);

        // Follow redirects
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

        // Enable progress meter
        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
        curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, Downloader::progress_callback);
        curl_easy_setopt(curl, CURLOPT_XFERINFODATA, nullptr);

        // Perform the download
        CURLcode res = curl_easy_perform(curl);

        if (res != CURLE_OK)
        {
            status_m = 1;
            qDebug() << "curl_easy_perform() failed:" << curl_easy_strerror(res);
            // we need to catch any of those that seem inportant here to the GUI
            // and reset GUI state
            // https://curl.se/libcurl/c/libcurl-errors.html
            if(res == 6 || res == 7 || res == 28 || res == 35)
            {
                emit this->networkWorkErrorSignal(1);
            }
            else if(res == CURLE_PEER_FAILED_VERIFICATION)
            {
                emit this->networkWorkErrorSignal(2);
            }
            else
            {
                emit this->networkWorkErrorSignal(3);
            }
        }
        else
        {
            status_m = 0;
            qDebug() << "Downloaded successfully";
        }
        curl_easy_cleanup(curl);
    }
    fclose(file);
}
