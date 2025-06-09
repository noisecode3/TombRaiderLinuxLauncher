/* TombRaiderLinuxLauncher
 * Martin Bångens Copyright (C) 2025
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


/**
 * @struct File
 * @brief Files object to keep track of files.
 *
 */
struct File {
    QString path;
    QString md5sum;
};

/**
 * @struct FolderNames
 * @brief Folder names game used on Windows.
 *
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

/**
 * @struct ExecutableNames
 * @brief Executable game file names used on Windows used to start the games.
 *
 * These are searched for together with rootless tree patterns. Also used as a default
 * starter by using a symbolic link to the executable that links to ddraw, directx11 or opengl.
 */
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

/**
 * @struct ZipData
 * @brief Holds download information for a Tomb Raider Level.
 *
 * This struct is designed to store a TRLE download file record for trle.net or trcustoms.org.
 * Each file record contains data for downloading and file name etc for the application.
 */
struct ZipData {
    /**
     * @brief Default constructor for `ZipData`.
     *
     * Initializes an empty instance of `ZipData`.
     */
    ZipData() {}
    /**
     * @brief Parameterized constructor for `ZipData`.
     *
     * This constructor initializes a `ZipData` object with metadata.
     *
     * @param zipName TRLE level zip file name.
     * @param zipSize Size of the file form TRLE in MiB.
     * @param md5sum checksum of the archive.
     * @param url String of download address.
     * @param version File version from trcustoms.org
     * @param type Level type.
     * @param release date of file release.
     */
    ZipData(
        const QString& zipName,
        float zipSize,
        const QString& md5sum,
        const QString& url,
        int version,
        int type,
        const QString& release) :
        name(zipName),
        mebibyteSize(zipSize),
        md5sum(md5sum),
        url(url),
        version(version),
        type(type),
        release(release) {}

    QString name;        ///< The archive file name.
    float mebibyteSize;  ///< The archive file size in MiB.
    QString md5sum;      ///< The archive md5sum.
    QString url;         ///< The URL of the TRLE level download.
    int version;         ///< The Version of trcustoms archive file.
    int type;            ///< The TRLE type used to identify a executable.
    QString release;     ///< The The date of file release from trcustoms.
};

/**
 * @struct ListItemData
 * @brief Represents a Tomb Raider Level Entry Card Info.
 *
 * This struct is designed to store a single TRLE (Tomb Raider Level Editor) level record.
 * Each record contains metadata and a cover image displayed as a card in the application.
 * The struct includes properties to facilitate searching, filtering, and sorting.
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
     * @param m_cover The cover image as a `m_cover`.
     */
    ListItemData(
            qint64 id, const QString& title, const QStringList& authors,
            qint64 type, qint64 classInput, const QString& releaseDate,
            qint64 difficulty, qint64 duration) :
            m_trle_id(id), m_title(title), m_authors(authors), m_type(type),
            m_class(classInput), m_releaseDate(releaseDate),
            m_difficulty(difficulty), m_duration(duration) {}

    void addPicture(const QByteArray& imageData) {
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

        // Store the resulting pixmap in m_picture
        m_cover = centeredPixmap;
    }
    // Data members
    qint64 m_trle_id;        ///< The TRLE level id.
    QString m_title;         ///< The TRLE level title.
    QStringList m_authors;   ///< The TRLE author(s), as a string list.
    qint64 m_type;           ///< ID of the type of level.
    qint64 m_class;          ///< ID of the class of the level.
    QString m_releaseDate;   ///< The release date in "YYYY-MM-DD" format.
    qint64 m_difficulty;     ///< ID of the difficulty of the level.
    qint64 m_duration;       ///< ID of the estimated duration of the level.
    QPixmap m_cover;         ///< The TRLE cover image pointer.
};

/**
 * @struct InfoData
 * @brief Store HTML data and a list of level pictures generated from image WEBP data.
 *
 * This struct is designed to store a body of HTML and convert a list of image data 
 * (provided as `QByteArray`) into `QPixmap` objects. The `QPixmap` objects can then 
 * be used in Qt-based applications with QtWebEngine and QPixmap in a split view manner.
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
     * Converts each image in the provided `QVector<QByteArray>` to a `QPixmap` object
     * using the "WEBP" format and stores them in the icon list.
     *
     * @param body A string representing the main textual content in HTML.
     * @param imageList A vector of image data in `QByteArray` format.
     */
    InfoData(const QString& body, const QVector<QByteArray>& imageList)
        : m_body(body) {
        for (const QByteArray& image : imageList) {
            QPixmap pixmap;

            // Load image data into a QPixmap
            if (!pixmap.loadFromData(image, "WEBP")) {
                qDebug() << "Could not load webp data to QPixmap.";
            }

            m_imageList.push_back(pixmap);
        }
    }

    QString m_body;  ///< The textual content associated with this object.
    QVector<QPixmap> m_imageList;  ///< A list of level large screen image data.
};

/**
 * @class Data
 * @brief Data component connected to the Model in the MVC pattern.
 *
 * It handles database connection(s) and reads and write scraped data.
 * Its a singlton in global space that packs data into struct's and passes them
 * to and from the Model.
 *
 */
class Data : public QObject {
    Q_OBJECT

 public:
    /**
     * Mayers thread safe singleton pattern.
     */
    static Data& getInstance() {
        // cppcheck-suppress threadsafety-threadsafety
        static Data instance;
        return instance;
    }

    /**
     * @brief Connect to the main database.
     *
     * Once we setup camp in home, we can use the path to camp.
     * That is usually in ~/.local/share/AppName
     *
     * @param path Full path to the datbase without the file name.
     */
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
            if (db.open() == true) {  // flawfinder: ignore
                status = true;
            } else {
                qDebug() << "Error opening database:" << db.lastError().text();
                status = false;
            }
        }
        return status;
    }

    /**
     * @brief Close the connection to the main database.
     *
     * Dont need to be called before the application exit.
     */
    void releaseDatabase() {
        db.close();
    }

    /**
     * @brief Count all the Level records in the database
     * @return The number of Level records
     */
    qint64 getListRowCount();
    /**
     * @brief Get the main list of levels, without the picture
     * @return The QVector ListItemData is all the level metadata
     */
    QVector<ListItemData> getListItems();
    /**
     * @brief Add Cover Pictures to ListItemData pointers
     * @param Cache like used QVector for holding ListItemData pointers
     */
    void getCoverPictures(QVector<ListItemData*>* items);
    /**
     * @brief Get the info page, this is HTML and picture data you see on trel.net
     * @param trle.net lid
     * @return The InfoData is a struct of QString and QVector<QPixmap>
     */
    InfoData getInfo(int id);
    /**
     * @brief Get the walkthrough HTML page
     * @param trle.net lid
     * @return HTML and Qt picture data
     */
    QString getWalkthrough(int id);
    /**
     * @brief Get the type of a level
     * @param trle.net lid
     * @return Id number of type
     */
    int getType(int id);

    /**
     * @brief Get a filelist for the original games
     * @param Id number for original game
     * @return File vector list (path and md5sum)
     */
    QVector<File> getFileList(const int id);
    /**
     * @brief Get download info for a level
     * @param trle.net lid
     * @return zip data, url, filename, size, etc...
     */
    ZipData getDownload(const int id);
    /**
     * @brief Record new md5sum to database.
     * @param trle.net lid
     * @param New zip file md5sum
     */
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
