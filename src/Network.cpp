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

size_t write_callback(void* contents, size_t size, size_t nmemb, FILE* file) {
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




void Downloader::run() {
    if (url_m.isEmpty() || file_m.isEmpty() || levelDir_m.isEmpty())
        return;

    QString urlString = url_m.toString();
    QByteArray byteArray = urlString.toUtf8();
    const char* url_cstring = byteArray.constData();

    QString filePath = levelDir_m.absolutePath() + QDir::separator() + file_m;

    // Open file for writing
    FILE* file = fopen(filePath.toUtf8().constData(), "wb");
    if (!file) {
        qDebug() << "Error opening file for writing:" << filePath;
        return;
    }

    // Initialize libcurl
    curl_global_init(CURL_GLOBAL_DEFAULT);

    // Initialize libcurl easy handle
    CURL* curl = curl_easy_init();
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

    // Clean up resources
    curl_global_cleanup();
    fclose(file);
}

