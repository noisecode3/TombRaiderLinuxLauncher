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
private:
    void saveToFile(const QByteArray& data, const QString& filePath);
    QUrl url_m;
    QString file_m;
    QDir levelDir_m;
    Downloader(QObject *parent = nullptr) : QObject(parent) {};
    ~Downloader() {};

    Q_DISABLE_COPY(Downloader)
};

#endif
