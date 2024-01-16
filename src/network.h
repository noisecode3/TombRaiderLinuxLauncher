#ifndef DOWNLOADER_H
#define DOWNLOADER_H

#include <QObject>
#include <QUrl>
#include <QFile>
#include <QDebug>
#include <curl/curl.h>

class Downloader : public QObject
{
    Q_OBJECT
public:
    explicit Downloader(QObject *parent = 0);
    void run();
    void setUrl(QUrl url);
    void setSavePath(QString path);

private:
    void saveToFile(const QByteArray& data, const QString& filePath);
    QUrl url_m;
    QString path_m;

};

#endif
