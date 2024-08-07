#include "Network.h"

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

//FIXME I hate to use globals like this..
qint64 oneTick=0;
qint64 remainderTick=0;
int timesSent=0;

size_t Downloader::write_callback(void* contents, size_t size, size_t nmemb, FILE* file)
{
    qint64 bars = (nmemb+remainderTick)/oneTick;
    remainderTick = (nmemb+remainderTick)%oneTick;
    while(bars>0)
    {
        emit Downloader::getInstance().networkWorkTickSignal();
        timesSent+=1;
        bars-=1;
    }
    return fwrite(contents, size, nmemb, file);
}

void Downloader::setUrl(QUrl url)
{
    url_m=url;
}

void Downloader::setSaveFile(const QString& file)
{
    file_m=file;
}

void Downloader::saveToFile(const QByteArray& data, const QString& filePath) {
    QFile file(filePath);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(data);
        file.close();
        qDebug() << "Data saved to file:" << filePath;
    } else {
        qDebug() << "Error saving data to file:" << file.errorString();
    }
}

void Downloader::run()
{
    if (url_m.isEmpty() || file_m.isEmpty() || levelDir_m.isEmpty())
        return;

    size_t zipFileSize=0; // I need to read filesize of the download link into this variable

    QString urlString = url_m.toString();
    QByteArray byteArray = urlString.toUtf8();
    const char* url_cstring = byteArray.constData();

    QString filePath = levelDir_m.absolutePath() + QDir::separator() + file_m;

    CURL* curl = curl_easy_init();
    if (curl)
    {
        // Set options to retrieve header only
        curl_easy_setopt(curl, CURLOPT_URL, url_cstring);
        curl_easy_setopt(curl, CURLOPT_NOBODY, 1L); // Header only
        curl_easy_setopt(curl, CURLOPT_HEADER, 1L); // Include header in the output

        // Perform the request to get header information
        CURLcode res = curl_easy_perform(curl);

        if (res == CURLE_OK)
        {
            res = curl_easy_getinfo(curl, CURLINFO_CONTENT_LENGTH_DOWNLOAD_T, &zipFileSize);
            if (res == CURLE_OK && zipFileSize != -1)
            {
                // Use zipFileSize as needed before downloading
            }
            else
            {
                qDebug() << "Failed to retrieve file size from header";
                // handle ssl error
                return;
            }
        }
        else
        {
            qDebug() << "curl_easy_perform() failed:" << curl_easy_strerror(res);
            return;
        }
    }

    oneTick = (zipFileSize/100)*2;
    remainderTick = 0;
    int timesSent = 0;

    // Open file for writing
    FILE* file = fopen(filePath.toUtf8().constData(), "wb");
    if (!file) {
        qDebug() << "Error opening file for writing:" << filePath;
        return;
    }

    // Initialize libcurl easy handle
    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, file);
        curl_easy_setopt(curl, CURLOPT_URL, url_cstring);

        // Perform the download
        CURLcode res = curl_easy_perform(curl);

        if (res != CURLE_OK) {
            qDebug() << "curl_easy_perform() failed:" << curl_easy_strerror(res);
        } else {
            qDebug() << "Downloaded successful";
        }

        // Clean up libcurl
        curl_easy_cleanup(curl);
    }
    fclose(file);
}

void networkWorkTickSignal(){};
