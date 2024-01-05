#pragma once
#include <QCoreApplication>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QFile>
#include <QDebug>

/**
 * //Example usage
 * FileDownloader downloader;
 * QUrl url("https://example.com/sample.txt");
 * QString localFilePath = "downloaded_file.txt";
 * downloader.downloadFile(url, localFilePath);
 */
class FileDownloader : public QObject
{
    Q_OBJECT
public:
    /**
     * 
     */
    explicit FileDownloader(QObject *parent = nullptr) : QObject(parent) {}
    /**
     * 
     */
    void downloadFile(const QUrl &url, const QString &localFilePath)
    {
        // Create a QNetworkAccessManager
        QNetworkAccessManager *manager = new QNetworkAccessManager(this);

        // Create a QNetworkRequest with the specified URL
        QNetworkRequest request(url);

        // Perform the download
        QNetworkReply *reply = manager->get(request);

        // Connect signals to handle the download process
        connect(reply, &QNetworkReply::finished, this, [=]() {
            if (reply->error() == QNetworkReply::NoError) {
                // Save the downloaded data to a local file
                saveToFile(reply, localFilePath);
                qDebug() << "Download completed successfully.";
            } else {
                // Handle download error
                qDebug() << "Download failed. Error:" << reply->errorString();
            }

            // Clean up resources
            reply->deleteLater();
            manager->deleteLater();
        });
    }
private:
    /**
     * 
     */
    void saveToFile(QIODevice *data, const QString &filePath)
    {
        // Open the local file for writing
        QFile file(filePath);
        if (file.open(QIODevice::WriteOnly)) {
            // Write the downloaded data to the local file
            file.write(data->readAll());
            file.close();
        }
    }
};
