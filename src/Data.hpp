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
#include <QThread>
#include <QMutex>

#include "../src/assert.hpp"
#include "Path.hpp"

/**
 * @struct FileListItem
 * @brief Files object to keep track of files.
 *
 */
struct FileListItem {
    QString path;
    QString md5sum;
};

/**
 * @struct FolderNames
 * @brief Folder names game used on Windows.
 *
 * These names are used by steam and installed in the common folder on Linux.
 * 1-6 first folder name are used by steam
 *
 */
struct FolderNames {
    QMap<int, QStringList> data = {
            {0, {"null"}},
            {1, {
                    "Tomb Raider (I)",
                    "Tomb Raider 1",
                    "Tomb Raider I"
                }},
            {2, {
                    "Tomb Raider (II)",
                    "Tomb Raider 2",
                    "Tomb Raider II"
                }},
            {3, {
                    "TombRaider (III)",
                    "Tomb Raider 3",
                    "Tomb Raider III"
                }},
            {4, {
                    "Tomb Raider (IV) The Last Revelation",
                    "Tomb Raider 4",
                    "Tomb Raider - The Last Revelation"
                }},
            {5, {
                    "Tomb Raider (V) Chronicles",
                    "Tomb Raider 5",
                    "Tomb Raider - Chronicles"
                }},
            {6, {"Tomb Raider (VI) The Angel of Darkness"}},
            {7, {"Tomb Raider I Unfinished Business"}},
            {8, {"Tomb Raider II Gold"}},
            {9, {"Tomb Raider - The Lost Artifact"}},
            {10, {"The Times - Exclusive Tomb Raider Level"}},
        };
};

struct SteamAppIds {
    QMap<int, int> data = {
            {0, 0},
            {1, 224960},
            {2, 225300},
            {3, 225320},
            {4, 224980},
            {5, 225000},
            {6, 225020},
            {7, 224960},
            {8, 225300},
            {9, 225320},
            {10, 224980},
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
            {1, "dosbox.exe"},
            {2, "Tomb2.exe"},
            {3, "tomb3.exe"},
            {4, "tomb4.exe"},
            {5, "PCTOMB5.EXE"},
            {6, "TombEngine.exe"},
            {7, "tombub.exe"},
            {8, "tomb2.EXE"},
            {9, "tr3gold.exe"},
            {10, "tomb4.exe"},
        };
};

/**
 * @struct MoodFolderNames
 * @brief Folder names moods used on Windows.
 *
 * These made up names that are used installed in the common folder on Linux.
 */
struct MoodFolderNames {
    QMap<int, QString> data = {
            {0, "null"},
            {1, "TombEngine (TEN)"}
        };
};

/**
 * @struct MoodExecutableNames
 * @brief Executable game file names used by unofficial moods.
 *
 * These are used as a default starter by using a symbolic link to the executable
 * It can help simplify running it from steam or lutris.
 */
struct MoodExecutableNames {
    QMap<int, QString> data = {
            {0, "null"},
            {1, "TombEngine.exe"},
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
    ZipData() {
        m_mebibyteSize = 0.0;
        m_version = 0;
        m_type = 0;
    }

    /**
     * @brief This sets the file name metadata.
     * @param fileName TRLE level zip file name.
     */
    inline void setFileName(const QString& fileName) {
        m_fileName = fileName;
    }

    /**
     * @brief This sets the file size metadata.
     * @param size Size in MiB.
     */
    inline void setMebibyteSize(const float size) {
        m_mebibyteSize = size;
    }

    /**
     * @brief This sets the file md5sum metadata.
     * @param md5sum Checksum of the archive.
     */
    inline void setMD5sum(const QString& MD5sum) {
        m_MD5sum = MD5sum;
    }

    /**
     * @brief This sets the URL for the file.
     * @param url String of download address.
     */
    inline void setURL(const QString& URL) {
        m_URL = URL;
    }

    /**
     * @brief This sets the zip file name metadata.
     * @param version File version from trcustoms.org
     */
    inline void setVersion(const quint64 version) {
        m_version = version;
    }

    /**
     * @brief This sets the level type metadata.
     * @param type Level type.
     */
    inline void setType(const quint64 type) {
        m_type = type;
    }

    /**
     * @brief This sets the id metadata.
     * @param type Level is.
     */
    inline void setId(const quint64 id) {
        m_id = id;
    }

    /**
     * @brief This sets the zip file name metadata.
     * @param release date of file release.
     */
    inline void setRelease(const QString& release) {
        m_release = release;
    }

    QString m_fileName;    ///< The archive file name.
    float m_mebibyteSize;  ///< The archive file size in MiB.
    QString m_MD5sum;      ///< The archive md5sum.
    QString m_URL;         ///< The URL of the TRLE level download.
    quint64 m_version;     ///< The Version of trcustoms archive file.
    quint64 m_type;        ///< The TRLE type used to identify a executable.
    quint64 m_id;          ///< The Level id.
    QString m_host;        ///< The host TRLE or TRCUSTOMS.
    QString m_release;     ///< The The date of file release from trcustoms.
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
    ListItemData() {
        m_game_id = 0;
        m_trle_id = 0;
        m_type = 0;
        m_class = 0;
        m_difficulty = 0;
        m_duration = 0;
        m_installed = false;
    }


    void setGameId(const qint64 id) {
        Q_ASSERT_WITH_TRACE(id != 0);
        m_game_id = id;
    }

    void setLid(const qint64 id) {
        Q_ASSERT_WITH_TRACE(id != 0);
        m_trle_id = id;
    }

    void setTitle(const QString& title) {
        Q_ASSERT_WITH_TRACE(!title.isEmpty());
        m_title = title;
    }

    void setAuthors(const QStringList& authors) {
        Q_ASSERT_WITH_TRACE(!authors.isEmpty());
        m_authors = authors;
    }

    void setShortBody(const QString& shortBody) {
        Q_ASSERT_WITH_TRACE(!shortBody.isEmpty());
        m_shortBody = shortBody;
    }

    void setType(const qint64 type) {
        Q_ASSERT_WITH_TRACE(type != 0);
        m_type = type;
    }

    void setClass(const qint64 _class) {
        m_class = _class;
    }

    void setDifficulty(const qint64 difficulty) {
        m_difficulty = difficulty;
    }

    void setDuration(const qint64 duration) {
        m_duration = duration;
    }

    void setReleaseDate(const QString& releaseDate) {
        Q_ASSERT_WITH_TRACE(!releaseDate.isEmpty());
        m_releaseDate = releaseDate;
    }

    void setPicture(const QByteArray& imageData) {
        // Load the image from the byte array
        QPixmap pixmap;
        pixmap.loadFromData(imageData, "WEBP");

        centerPixmap(pixmap);
    }

    void setPicture(const QPixmap& pixmap) {
        centerPixmap(pixmap);
    }
    // Data members
    qint64 m_game_id;        ///< The Game id.
    qint64 m_trle_id;        ///< The TRLE level id.
    QString m_shortBody;     ///< The Game info text.
    QString m_title;         ///< The TRLE level title.
    QStringList m_authors;   ///< The TRLE author(s), as a string list.
    qint64 m_type;           ///< ID of the type of level.
    qint64 m_class;          ///< ID of the class of the level.
    QString m_releaseDate;   ///< The release date in "YYYY-MM-DD" format.
    qint64 m_difficulty;     ///< ID of the difficulty of the level.
    qint64 m_duration;       ///< ID of the estimated duration of the level.
    QPixmap m_cover;         ///< The TRLE cover image.
    bool m_installed;

 private:
    void centerPixmap(QPixmap pixmap) {
        // Define target dimensions and maintain aspect ratio
        QSize targetSize(160, 120);
        QSize newSize = pixmap.size().scaled(targetSize, Qt::KeepAspectRatio);

        // Scale the pixmap
        QPixmap scaledCover = pixmap.scaled(
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
        painter.drawPixmap(xOffset, yOffset, scaledCover);
        painter.end();

        // Store the resulting pixmap in m_picture
        m_cover = centeredPixmap;
    }
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
    bool initializeDatabase() {
        bool status = false;
        m_path = Path(Path::resource);
        m_path << "tombll.db";

        if (!m_path.exists() || !m_path.isFile()) {
            qCritical()
                << "Error: The database path is not a regular file: "
                << m_path.get();
            status = false;
        } else {
            status = true;
        }
        return status;
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
    QVector<QSharedPointer<ListItemData>> getListItems();

    /**
     * @brief Add Cover Pictures to ListItemData pointers
     * @param Cache like used QVector for holding ListItemData pointers
     */
    void getCoverPictures(QVector<QSharedPointer<ListItemData>> items);

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
    QVector<FileListItem> getFileList(const int id);

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
    ~Data() {}

    QSqlDatabase& getThreadDatabase() {
        static QMutex mutex;
        const quintptr tid = (quintptr)QThread::currentThreadId();

        QMutexLocker locker(&mutex);

        if (!m_connectionTable.contains(tid)) {
            const QString name = QString("conn_%1").arg(tid);
            QSqlDatabase db;
            if (QSqlDatabase::contains(name)) {
                db = QSqlDatabase::database(name);
            } else {
                db = QSqlDatabase::addDatabase("QSQLITE", name);
                db.setDatabaseName(m_path.get());
                db.open();
            }
            m_connectionTable.insert(tid, db);
        }

        return m_connectionTable[tid];
    }

    Path m_path = Path(Path::resource);
    QHash<quintptr, QSqlDatabase> m_connectionTable;
    Q_DISABLE_COPY(Data)
};

#endif  // SRC_DATA_HPP_
