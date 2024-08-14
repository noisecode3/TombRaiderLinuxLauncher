#ifndef DOWNLOADER_H
#define DOWNLOADER_H

#include <QObject>
#include <QUrl>
#include <QFile>
#include <QDir>
#include <QDebug>
#include <curl/curl.h>

class Downloader : public QObject
{
    Q_OBJECT
public:
    static Downloader& getInstance()
    {
        static Downloader instance;
        return instance;
    };
    void run();
    bool setUpCamp(const QString& levelDir);
    void setUrl(QUrl url);
    void setSaveFile(const QString& file);
    static size_t write_callback(void* contents, size_t size, size_t nmemb, void* userData);

signals:
    void networkWorkTickSignal();

private:
    void saveToFile(const QByteArray& data, const QString& filePath);
    QUrl url_m;
    QString file_m;
    QDir levelDir_m;

    qint64 oneTick = 0;
    qint64 remainderTick = 0;
    int timesSent = 0;

    Downloader(QObject *parent = nullptr) : QObject(parent),
        oneTick(0), remainderTick(0), timesSent(0)
    {
        // Initialize libcurl
        curl_global_init(CURL_GLOBAL_DEFAULT);
    };
    ~Downloader()
    {
        // Clean up resources
        curl_global_cleanup();
    };

    Q_DISABLE_COPY(Downloader)
};

#endif
