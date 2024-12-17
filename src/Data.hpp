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

#ifndef SRC_DATA_HPP_
#define SRC_DATA_HPP_

#include <QDebug>
#include <QFileInfo>
#include <QIcon>
#include <QObject>
#include <QPixmap>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>

/**
 * @struct FolderNames
 * @brief Folder names game used on windows
 * @details These names are used by steam and installed in the common folder on linux.
 */
struct FolderNames {
    const QString TR1 = "/Tomb Raider (I)";
    const QString TR2 = "/Tomb Raider (II)";
    const QString TR3 = "/TombRaider (III)";
    const QString TR4 = "/Tomb Raider (IV) The Last Revelation";
    const QString TR5 = "/Tomb Raider (V) Chronicles";
};

struct ZipData {
    /**
     * @struct ZipData
     * @brief
     * @param
     * @details
     */
    ZipData() {}
    ZipData(
        QString zipName,
        float zipSize,
        QString md5sum,
        QString url,
        int version,
        const QString& release) :
        name(zipName),
        megabyteSize(zipSize),
        md5sum(md5sum),
        url(url),
        version(version),
        release(release) {}
    QString name;
    float megabyteSize;
    QString md5sum;
    QString url;
    int version;
    QString release;
};

struct ListItemData {
    /**
     * @struct InfoData
     * @brief
     * @param
     * @details
     */
    ListItemData() {}
    ListItemData(
        QString title,
        QString author,
        qint64 type,
        qint64 classIn,
        QString releaseDate,
        qint64 difficulty,
        qint64 duration,
        QByteArray imageData):
        title(title),
        author(author),
        type(type),
        class_(classIn),
        releaseDate(releaseDate),
        difficulty(difficulty),
        duration(duration) {
        QPixmap pixmap;
        pixmap.loadFromData(imageData, "WEBP");
        picture.addPixmap(pixmap);
        // Scale the pixmap while maintaining aspect ratio
        QSize newSize = pixmap.size().scaled(640, 480, Qt::KeepAspectRatio);
        // Resize the pixmap to the scaled size
        pixmap = pixmap.scaled(
            newSize,
            Qt::KeepAspectRatio,
            Qt::SmoothTransformation);
        // Create QIcon and add the scaled pixmap
        picture.addPixmap(pixmap);
    }
    QString title;
    QString author;
    qint64  type;
    qint64  class_;
    QString releaseDate;
    qint64  difficulty;
    qint64  duration;
    QIcon   picture;
};

/**
 * @struct InfoData
 * @brief Store HTML data and a list of icons generated from image WEBP data.
 *
 * This struct is designed to store a body of HTML and convert a list of image data 
 * (provided as `QByteArray`) into `QIcon` objects. The `QIcon` objects can then 
 * be used in Qt-based applications with QtWebEngine and QIcons in a split view manner.
 */
struct InfoData {
    /**
     * @brief Default constructor for `InfoData`.
     *
     * Initializes an empty body and an empty list of icons.
     */
    InfoData() {}

    /**
     * @brief Constructs an `InfoData` object with the given body and image list.
     *
     * Converts each image in the provided `QVector<QByteArray>` to a `QIcon` object
     * using the "WEBP" format and stores them in the icon list.
     *
     * @param body A string representing the main textual content.
     * @param imageList A vector of image data in `QByteArray` format.
     */
    InfoData(const QString& body, const QVector<QByteArray>& imageList)
        : m_body(body) {
        for (const QByteArray& image : imageList) {
            QPixmap pixmap;
            QIcon finalIcon;

            // Load image data into a QPixmap and convert it to a QIcon
            if (pixmap.loadFromData(image, "WEBP") == true) {
                finalIcon.addPixmap(pixmap);
            }

            m_imageList.push_back(finalIcon);
        }
    }

    QString m_body;  ///< The textual content associated with this object.
    QVector<QIcon> m_imageList;  ///< A list of icons generated from image data.
};

class Data : public QObject {
    Q_OBJECT

 public:
    static Data& getInstance() {
        static Data instance;
        return instance;
    }

    bool initializeDatabase(const QString& path) {
        bool status = false;
        const QString filePath = QString("%1/%2").arg(path, "tombll.db");
        QFileInfo fileInfo(filePath);

        // Open the file
        if (!fileInfo.exists() || !fileInfo.isFile()) {
            qCritical()
                << "Error: The database path is not a regular file: " << path;
            status = false;
        } else {
            db = QSqlDatabase::addDatabase("QSQLITE");
            db.setDatabaseName(path + "/tombll.db");
            db.setConnectOptions("QSQLITE_OPEN_READONLY");
            if (db.open() == true) {  // flawfinder: ignore
                status = true;
            } else {
                qDebug() << "Error opening database:" << db.lastError().text();
                status = false;
            }
        }
        return status;
    }

    void releaseDatabase() {
        db.close();
    }

    QVector<ListItemData> getListItems();
    InfoData getInfo(int id);
    QString getWalkthrough(int id);
    int getType(int id);

    std::array<QVector<QString>, 2> getFileList(const int id, bool trleList);
    ZipData getDownload(int id);

 private:
    explicit Data(QObject *parent = nullptr) : QObject(parent) {
    }
    ~Data() {
        db.close();
    }

    QSqlDatabase db;
    Q_DISABLE_COPY(Data)
};

#endif  // SRC_DATA_HPP_
