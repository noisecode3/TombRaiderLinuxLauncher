#ifndef DATA_H
#define DATA_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QDebug>
#include <QSqlError>
#include "qicon.h"
#include "qpixmap.h"
/**
 * @struct FolderNames
 * @brief Folder names game used on windows
 * @details These names are used by steam and installed in the common folder on linux.
 */
struct FolderNames
{
    const QString TR1 = "/Tomb Raider (I)";
    const QString TR2 = "/Tomb Raider (II)";
    const QString TR3 = "/TombRaider (III)";
    const QString TR4 = "/Tomb Raider (IV) The Last Revelation";
    const QString TR5 = "/Tomb Raider (V) Chronicles";
};


struct ZipData
{
    /**
     * @struct ZipData
     * @brief
     * @param
     * @details
     */
    ZipData() {};
    ZipData( QString zipName, float zipSize, QString md5sum, QString url ):
        name(zipName),
        megabyteSize(zipSize),
        md5sum(md5sum),
        url(url) {};
    QString name;
    float megabyteSize;
    QString md5sum;
    QString url;
};

struct ListItemData
{
    /**
     * @struct InfoData
     * @brief
     * @param
     * @details
     */
    ListItemData() {};
    ListItemData( QString title, QString author, QString type,
              QString class_, QString releaseDate, QString difficulty,
              QString duration, QByteArray imageData ):
        title(title),
        author(author),
        type(type),
        class_(class_),
        releaseDate(releaseDate),
        difficulty(difficulty),
        duration(duration)
    {
        QPixmap pixmap;
        pixmap.loadFromData(imageData, "JPG");
        picture.addPixmap(pixmap);
    };
    QString title;
    QString author;
    QString type; // like TR4
    QString class_;
    QString releaseDate;
    QString difficulty;
    QString duration;
    QIcon   picture;
};

struct InfoData
{
    /**
     * @struct InfoData
     * @brief
     * @param
     * @details
     */
    InfoData() {};
    InfoData( QString body, QVector<QByteArray> imageList ) : body(body)
    {
        for (const QByteArray &image : imageList)
        {
            QPixmap pixmap;
            QIcon final;
            pixmap.loadFromData(image, "JPG");
            final.addPixmap(pixmap);
            this->imageList.push_back(final);
        }
    };
    QString body;
    QVector<QIcon> imageList;
};

/**
 * Data& data = Data::getInstance();
 * if (data.initializeDatabase("/your/path")) {
 *     // Database is successfully opened, perform your read operations here
 *
 *     // ...
 *
 *     // Release the database when done
 *     data.releaseDatabase();
 * }
 *
 */
class Data : public QObject
{
    Q_OBJECT
public:
    static Data& getInstance()
    {
        static Data instance;
        return instance;
    }

    // Method to initialize the database connection
    bool initializeDatabase(QString path)
    {
        // Set the database name and file mode (ReadOnly)
        db.setDatabaseName(path + "/tombll.db");
        db.setConnectOptions("QSQLITE_OPEN_READONLY");

        // Open the database
        if (db.open()) {
            return true; // Successfully opened
        } else {
            qDebug() << "Error opening database:" << db.lastError().text();
            return false; // Failed to open
        }
    }

    // Method to release the database connection
    void releaseDatabase()
    {
        db.close();
    }

    std::array<QVector<QString>, 2> getFileList(const int id, bool trleList);
    QVector<ListItemData> getListItems();
    InfoData getInfo(int id);
    QString getWalkthrough(int id);
    ZipData getDownload(int id);

    int getType(int id);

private:
    Data(QObject *parent = nullptr) : QObject(parent) { }
    ~Data() { db.close(); }

    Q_DISABLE_COPY(Data)
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
};

#endif // DATA_H
