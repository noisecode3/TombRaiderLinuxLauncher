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
    WriteData* data = static_cast<WriteData*>(userData);
    Downloader* instance = data->downloader;
    FILE* file = data->file;

    qint64 bars = (nmemb + instance->remainderTick) / instance->oneTick;
    instance->remainderTick =
            (nmemb + instance->remainderTick) % instance->oneTick;

    while (bars > 0)
    {
        emit instance->networkWorkTickSignal();
        instance->timesSent += 1;
        bars -= 1;
    }

    return fwrite(contents, size, nmemb, file);
}

void Downloader::setUrl(QUrl url)
{
    url_m = url;
}

void Downloader::setSaveFile(const QString& file)
{
    file_m = file;
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

    int zipFileSize = 0;

    QString urlString = url_m.toString();
    QByteArray byteArray = urlString.toUtf8();
    const char* url_cstring = byteArray.constData();

    QString filePath = levelDir_m.absolutePath() + QDir::separator() + file_m;

    CURL* curl = curl_easy_init();
    if (curl)
    {
        // Set options to retrieve header only
        curl_easy_setopt(curl, CURLOPT_URL, url_cstring);
        curl_easy_setopt(curl, CURLOPT_NOBODY, 1L);
        curl_easy_setopt(curl, CURLOPT_HEADER, 1L);

        // Perform the request to get header information
        CURLcode res = curl_easy_perform(curl);

        if (res == CURLE_OK)
        {
            res = curl_easy_getinfo(
                curl,
                CURLINFO_CONTENT_LENGTH_DOWNLOAD_T,
                &zipFileSize);
            if (res == CURLE_OK && zipFileSize != -1)
            {
                // Use zipFileSize as needed before downloading
            }
            else
            {
                qDebug() << "Failed to retrieve file size from header";
                // handle SSL error, no connection, restore to previous state
                // with an error on the GUI
                return;
            }
        }
        else
        {
            qDebug() << "curl_easy_perform() failed:"
                << curl_easy_strerror(res);
            return;
        }
    }

    oneTick = (zipFileSize / 100) * 2;
    remainderTick = 0;
    timesSent = 0;

    FILE* file = fopen(filePath.toUtf8().constData(), "wb");
    if (!file)
    {
        qDebug() << "Error opening file for writing:" << filePath;
        return;
    }

    WriteData writeData = { this, file };

    curl = curl_easy_init();
    if (curl)
    {
        curl_easy_setopt(
            curl,
            CURLOPT_WRITEFUNCTION,
            Downloader::write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &writeData);
        curl_easy_setopt(curl, CURLOPT_URL, url_cstring);

        // Perform the download
        CURLcode res = curl_easy_perform(curl);

        if (res != CURLE_OK)
        {
            qDebug() << "curl_easy_perform() failed:"
                << curl_easy_strerror(res);
        }
        else
        {
            qDebug() << "Downloaded successfully";
        }

        curl_easy_cleanup(curl);
    }
    fclose(file);
}
