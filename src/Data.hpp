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
#include <QPainter>
#include <QPixmap>
#include <QSize>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>

struct FileList {
    QString path;
    QString md5sum;
};

/**
 * @struct FolderNames
 * @brief Folder names game used on Windows
 * These names are used by steam and installed in the common folder on Linux.
 * Except for `TombEngine (TEN)` I made that one up.
 */
struct FolderNames {
    QMap<int, QString> data = {
            {0, "null"},
            {1, "Tomb Raider (I)"},
            {2, "Tomb Raider (II)"},
            {3, "TombRaider (III)"},
            {4, "Tomb Raider (IV) The Last Revelation"},
            {5, "Tomb Raider (V) Chronicles"},
            {6, "TombEngine (TEN)"},
        };
};

struct ExecutableNames {
    QMap<int, QString> data = {
            {0, "null"},
            {1, "tomb.exe"},
            {2, "Tomb2.exe"},
            {3, "tomb3.exe"},
            {4, "tomb4.exe"},
            {5, "PCTOMB5.EXE"},
            {6, "TombEngine.exe"},
        };
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
        const QString& zipName,
        float zipSize,
        const QString& md5sum,
        const QString& url,
        int version,
        int type,
        const QString& release) :
        name(zipName),
        megabyteSize(zipSize),
        md5sum(md5sum),
        url(url),
        version(version),
        type(type),
        release(release) {}
    QString name;
    float megabyteSize;
    QString md5sum;
    QString url;
    int version;
    int type;
    QString release;
};

/**
 * @struct ListItemData
 * @brief Represents a Tomb Raider Level Entry Card Info.
 *
 * This struct is designed to store a single TRLE (Tomb Raider Level Editor) level record.
 * Each record contains metadata and a cover image displayed as a card in the application.
 * The struct includes properties to facilitate searching, filtering, and sorting without
 * the cover image.
 */
struct ListItemData {
    /**
     * @brief Default constructor for `ListItemData`.
     *
     * Initializes an empty instance of `ListItemData`.
     */
    ListItemData() {}

    /**
     * @brief Parameterized constructor for `ListItemData`.
     *
     * This constructor initializes a `ListItemData` object with metadata.
     *
     * @param id The TRLE numeric level ID.
     * @param title The TRLE title. Expected to contain a single name.
     * @param author The TRLE author(s). Can be a single name or multiple names.
     * @param type The TRLE type, represented by a numeric ID.
     * @param classInput The TRLE class, represented by a numeric ID.
     * @param releaseDate The release date in the format "YYYY-MM-DD" (e.g., "2000-01-01").
     * @param difficulty The TRLE difficulty, represented by a numeric ID.
     * @param duration The TRLE duration, represented by a numeric ID.
     */
    ListItemData(
        qint64 id, const QString& title, const QStringList& authors, qint64 type,
        qint64 classInput, const QString& releaseDate, qint64 difficulty,
        qint64 duration) :
        m_trle_id(id), m_title(title), m_authors(authors), m_type(type),
        m_class(classInput), m_releaseDate(releaseDate),
        m_difficulty(difficulty), m_duration(duration) {}

    // Data members
    qint64 m_trle_id;        ///< The TRLE level id.
    QString m_title;         ///< The TRLE level title.
    QStringList m_authors;     ///< The TRLE author(s), as a string list.
    qint64 m_type;           ///< ID of the type of level.
    qint64 m_class;          ///< ID of the class of the level.
    QString m_releaseDate;   ///< The release date in "YYYY-MM-DD" format.
    qint64 m_difficulty;     ///< ID of the difficulty of the level.
    qint64 m_duration;       ///< ID of the estimated duration of the level.
};

/**
 * @struct ListItemPicture
 * @brief Represents a Tomb Raider Level Entry Card Picture.
 *
 * This struct is designed to store a single TRLE (Tomb Raider Level Editor) level picture record.
 * Each record is a cover image displayed as a card in the application.
 */
struct ListItemPicture {
    /**
     * @brief Default constructor for `ListItemPicture`.
     *
     * Initializes an empty instance of `ListItemPicture`.
     */
    ListItemPicture() {}

    /**
     * @brief Parameterized constructor for `ListItemPicture`.
     *
     * This constructor initializes a `ListItemPicture` object with a cover image.
     * The image is converted from raw `QByteArray` to a `QIcon` after scaling it to
     * fit within 640x480 dimensions. The scaling maintains the aspect ratio and
     * smooths out pixels using `Qt::SmoothTransformation`. The image is centered
     * within a transparent background if its aspect ratio does not perfectly match the target.
     *
     * @param id The TRLE numeric level ID.
     * @param imageData The cover image as a `QByteArray`. Supported formats include JPEG, PNG, and WEBP.
     */
    ListItemPicture(
        qint64 id, QByteArray imageData) : m_trle_id(id) {
        // Load the image from the byte array
        QPixmap pixmap;
        pixmap.loadFromData(imageData, "WEBP");

        // Define target dimensions and maintain aspect ratio
        QSize targetSize(640, 480);
        QSize newSize = pixmap.size().scaled(targetSize, Qt::KeepAspectRatio);

        // Scale the pixmap
        QPixmap scaledPixmap = pixmap.scaled(
            newSize,
            Qt::KeepAspectRatio,
            Qt::SmoothTransformation);

        // Create a centered pixmap with a transparent background
        QPixmap centeredPixmap(targetSize);
        // Ensure a transparent background
        centeredPixmap.fill(Qt::transparent);

        // Calculate offsets for centering the scaled image
        qint64 xOffset = (targetSize.width() - newSize.width()) / 2;
        qint64 yOffset = (targetSize.height() - newSize.height()) / 2;

        // Draw the scaled image onto the centered pixmap
        QPainter painter(&centeredPixmap);
        painter.setRenderHint(QPainter::Antialiasing, true);
        painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
        painter.drawPixmap(xOffset, yOffset, scaledPixmap);
        painter.end();

        // Store the resulting pixmap in a QIcon
        m_picture.addPixmap(centeredPixmap);
    }

    // Data members
    qint64 m_trle_id;        ///< The TRLE level id.
    QIcon m_picture;         ///< The cover image.
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
        // cppcheck-suppress threadsafety-threadsafety
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
            db.setDatabaseName(QString("%1/tombll.db").arg(path));
            // db.setConnectOptions("QSQLITE_OPEN_READONLY");
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

    qint64 getListRowCount();
    QVector<ListItemData> getListItems();
    QVector<ListItemPicture> getPictures(QList<qint64> trle_ids);
    InfoData getInfo(int id);
    QString getWalkthrough(int id);
    int getType(int id);

    QVector<FileList> getFileList(const int id);
    ZipData getDownload(const int id);
    void setDownloadMd5(const int id, const QString& newMd5sum);

 private:
    Data() {}
    ~Data() {
        db.close();
    }

    QSqlDatabase db;
    Q_DISABLE_COPY(Data)
};

#endif  // SRC_DATA_HPP_
