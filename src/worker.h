#ifndef FILES_H
#define FILES_H
#include "network.h"
#include "qicon.h"
#include "qpixmap.h"
#include "quazip/quazip.h"
#include "quazip/quazipfile.h"
#include <QCoreApplication>
#include <QThread>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QDebug>
#include <QStandardPaths>
#include <QDirIterator>
#include <QDir>
#include <QFileInfo>
#include <QFile>
#include <QCryptographicHash>
#include <QStringList>
#include <QDebug>
#include <QString>
#include <QSqlError>
class GameFileList
{
public:
    /**
     * @class GameFileList
     * @brief Game List of file names and md5sum
     * @param name Name of game release or game level
     * @details 2 Qt lists of file names and md5sum, that share index number. \name
     * When a file is added to the collection of lists. It makes sure the attributes share the index.
     */
    GameFileList() {}
    GameFileList(const QString name) : name(name) {}
    
    /**
     * @brief get md5sum based on path
     * @param path File name relative to game directory
     */
    const QString getMd5sum(QString& path)
    {
        int index = fileList.indexOf(path);
        if (index != -1)
            return md5List.at(index);
        return "";
    }
    /**
     * @brief get index form path 
     * @param path File name relative to game directory
     */
    size_t getIndex(QString& path)
    {
        return fileList.indexOf(path);
    }
    /**
     * @brief get size of the index
     */
    size_t getSize()
    {
        return fileList.size();
    }
    /**
     * @brief Adds file attributes to the list
     * @param md5sum Recorded md5sum from the file
     * @param path File name relative to game directory
     * @details Adds file name and md5sum to 2 QList only return true if both gets added
     */
    bool addFile(const QString md5sum, const QString path)
    {
        int fileListSize = fileList.size();
        fileList.append(path);
        int md5sumSize  = md5List.size(); 
        md5List.append(md5sum);
        if(fileListSize+1 == fileListSize)
            if(md5sumSize+1 == md5sumSize)
                return true;
        return false;
    }
    /**
     * @brief Get filename
     * @param index a number from 0 to size
     * @details The filename and path from the current game directory. \n
     * For example /tomb.exe or /pix/screen1.jpg without the dot prefix
     */
    const QString& getPath(int index) const
    {
        if (index != -1)
            return fileList.at(index);
        return emptySpace;
    }
    /**
     * @brief Get md5sum
     * @details The md5sum is used for file integrity monitoring
     */
    const QString& getMd5sum(int index) const
    {
        if (index != -1)
            return md5List.at(index);
        return emptySpace;
    }
private:
    const QString name;
    QList<QString> fileList;
    QList<QString> md5List;
    const QString emptySpace = "";
};
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

struct PictureData
{
    /**
     * @struct PictureData
     * @brief
     * @param
     * @details
     */
    PictureData( QString name, QByteArray imageData ):
        name(name)
    {
        QPixmap pixmap;
        pixmap.loadFromData(imageData, "JPG");
        QIcon data(pixmap);
    }
    QString name;
    QIcon data;
};

struct ZipData
{
    /**
     * @struct ZipData
     * @brief
     * @param
     * @details
     */
    ZipData( QString zipName, float zipSize, QString md5sum ):
        name(zipName),
        megabyteSize(zipSize),
        md5sum(md5sum)
    {}
    QString name;
    float megabyteSize;
    QString md5sum;
};

struct InfoData
{
    /**
     * @struct InfoData
     * @brief
     * @param
     * @details
     */
    InfoData( QString title, QString author, QString type,
              QString class_, QString releaseDate, QString difficulty,
              QString duration ):
        title(title),
        author(author),
        type(type),
        class_(class_),
        releaseDate(releaseDate),
        difficulty(difficulty),
        duration(duration)
    {}
    QString title;
    QString author;
    QString type; // like TR4
    QString class_;
    QString releaseDate;
    QString difficulty;
    QString duration;
};

struct LevelData
{
    /**
     * @struct LevelData
     * @brief Game List of file names and md5sum
     * @param name Name of game release or game level
     * @details
     */
    LevelData(
        InfoData info,
        ZipData zip,
        PictureData picture,
        QString screen,
        QString screenLarge,
        QString body,
        QString walkthrough ):
        info(info),
        zip(zip),
        picture(picture),
        screen(screen),
        screensLarge(screenLarge),
        body(body),
        walkthrough(walkthrough)
    {}
    InfoData info;
    ZipData zip;
    PictureData picture;
    QString screen;
    QString screensLarge; // separated by ,
    QString body; //html
    QString walkthrough; //html
};

class pool
{
public:
    const QString levelUrl = "https://www.trle.net/sc/levelfeatures.php?lid={id}";
    const QString walkthroughUrl = "https://www.trle.net/sc/Levelwalk.php?lid={id}";
    const QString screensUrl = "https://www.trle.net/screens/{id}.jpg";
    const QString screensLargeUrl = "https://www.trle.net/screens/large/{id}[a-z].jpg";
    const QString downloadUrl = "https://www.trle.net/levels/levels/{year}/{id}/{zipName}";
/**
 *
 *
 *
 */
    pool(const QString path)
    {
        // Add SQLite database driver
        QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");

        // Set the database name and file mode (ReadOnly)
        db.setDatabaseName(path+"/tombll.db");
        db.setConnectOptions("QSQLITE_OPEN_READONLY");

        // Open the database
        if (db.open()) {
            qDebug() << "Database opened in read-only mode.";

            QSqlQuery query;
            query.prepare("SELECT Level.*, Info.*, Zip.*, Picture.* "
                          "FROM Level "
                          "JOIN Info ON Level.infoID = Info.InfoID "
                          "JOIN Zip ON Level.zipID = Zip.ZipID "
                          "JOIN Picture ON Level.pictureID = Picture.PictureID "
                          "WHERE Level.LevelID = :id");
            query.bindValue(":id", 1); // Set the ID

            // Execute the query
            if (query.exec()) {
                // Iterate over the result set
                while (query.next()) {
                    InfoData info(
                        query.value("Info.title").toString(),
                        query.value("Info.author").toString(),
                        query.value("Info.release").toString(),
                        query.value("Info.difficulty").toString(),
                        query.value("Info.duration").toString(),
                        query.value("Info.type").toString(),
                        query.value("Info.class").toString());

                    // Extract data from Zip table
                    ZipData zip(
                        query.value("Zip.name").toString(),
                        query.value("Zip.size").toFloat(),
                        query.value("Zip.md5sum").toString());

                    // Extract data from Picture table
                    // Assuming PictureData is a class with a constructor that takes name and data
                    PictureData picture(query.value("Picture.name").toString(), query.value("Picture.data").toByteArray());

                    data.push_back(
                        LevelData(info, zip, picture, QString(""), QString(""), QString(""), QString(""))
                        );
                }
            } else {
                qDebug() << "Error executing query:" << query.lastError().text();
            }
            // Close the database when done
            db.close();
            qDebug() << "Database closed.";
        } else {
            // Handle the case when the database cannot be opened
            qDebug() << "Failed to open the database.";
        }
    }


private:
    QList<LevelData> data;
};
// WorkerThread class for hard drive and database operations
    // Create an instance of the worker thread
    // WorkerThread workerThread;

    // Start the worker thread
    // workerThread.start();

    // Wait for the worker thread to finish
    // workerThread.wait();

class WorkerThread : public QThread
{
    Q_OBJECT
    uint id;
    bool original;
    QString folderPath;
    QString directoryPath;
    GameFileList TR3FileList = QString("TR3.Original");

public:
    /**
     * 
     */
    WorkerThread(uint id, bool original, QString folderPath="", QString directoryPath="")
    : id(id), original(original), folderPath(folderPath), directoryPath(directoryPath)
    {
        TR3FileList.addFile("61f1377c2123f1c971cbfbf9c7ce8faf", "DATA.TAG");
        TR3FileList.addFile("d09677d10864caa509ab2ffbd7ad37e7", "DEC130.DLL");
        TR3FileList.addFile("4ee5d4026f15c967ed3ae599885018b0", "WINPLAY.DLL");
        TR3FileList.addFile("af1d8d9435cb10fe2f4b4215eaf6bec4", "os.dat");
        TR3FileList.addFile("3561c0dffdb90248fa1fc2d4fb86f08a", "data.bin");
        TR3FileList.addFile("4044dc2c58f02bfea2572e80dd8f2abb", "tomb3.exe");
        TR3FileList.addFile("d0f7f7eda9e692eac06b32813a86e0c3", "layout.bin");
        TR3FileList.addFile("26b1f5d031c67a0b4a1832d1b4e6422c", "EDEC.DLL");
        TR3FileList.addFile("bd9397ee53c1dbe34d1b4fc168d8025e", "WINSTR.DLL");
        TR3FileList.addFile("f220b7353e550c8e017ab2d90fd83ae3", "WINSDEC.DLL");
        TR3FileList.addFile("90e64689804b4f4b0197c07290965a3c", "lang.dat");
        TR3FileList.addFile("0e4643dc86f0228969c0e0c1b30c0711", "audio/cdaudio.wad");
        TR3FileList.addFile("7d88aa20642a41e9d68558456683f57b", "pix/TITLEUK.BMP");
        TR3FileList.addFile("ffb8d2d1cc6a4723e0c58a2fe548f94f", "pix/SOUTHPAC.BMP");
        TR3FileList.addFile("2e5453045748420e576806917e0a30e3", "pix/CREDIT07.BMP");
        TR3FileList.addFile("5511970cf790c81a351e19baab1b1068", "pix/CREDIT03.BMP");
        TR3FileList.addFile("17e2230f562c04d2f0c0afabcad60629", "pix/CREDIT05.BMP");
        TR3FileList.addFile("88332670006eb121a705c839288c0e36", "pix/CREDIT04.BMP");
        TR3FileList.addFile("05608fdd5355624192a2b95eeaf21bc5", "pix/CREDIT01.BMP");
        TR3FileList.addFile("8682420d6bdefcb87820065dd87e50f0", "pix/ANTARC.BMP");
        TR3FileList.addFile("d942b38b5ce68d4a96b8b484e38f486f", "pix/THEEND.BMP");
        TR3FileList.addFile("deb5c81fc134fb2207f16df45bf97352", "pix/CREDIT08.BMP");
        TR3FileList.addFile("f1928fd6d00e1aefff2338f681c5a229", "pix/CREDIT09.BMP");
        TR3FileList.addFile("7456ab850b9444fa622250ca70aac6d5", "pix/HOUSE.BMP");
        TR3FileList.addFile("7fa680415bbabe15b155632a722b2ea0", "pix/CREDIT02.BMP");
        TR3FileList.addFile("92756534d2ac4dc9e118c906c7c16098", "pix/INDIA.BMP");
        TR3FileList.addFile("abb63ed2a790b5795082324b14a1fea7", "pix/CREDIT06.BMP");
        TR3FileList.addFile("b59fc9ad5f0bd64000c73bed4c2c6e21", "pix/LEGAL.BMP");
        TR3FileList.addFile("db7ae096fced4eaa630c0bec83114ee4", "pix/NEVADA.BMP");
        TR3FileList.addFile("67b5cab900f04c913f6bab241c4638ff", "pix/LONDON.BMP");
        TR3FileList.addFile("f56331cf11bf04dfef47492626cc9073", "pix/THEEND2.BMP");
        TR3FileList.addFile("9befdc5075fdb84450d2ed0533719b12", "data/JUNGLE.TR2");
        TR3FileList.addFile("69daad41e8a9ac9fad5aab6d22908de7", "data/CITY.TR2");
        TR3FileList.addFile("508ba45acbe4317e23daaf54ba919d04", "data/MAIN.SFX");
        TR3FileList.addFile("80f7907ded8a372bb87b1bcea178f94e", "data/ANTARC.TR2");
        TR3FileList.addFile("7a46c92685674a95024a9886152f8c2c", "data/TOWER.TR2");
        TR3FileList.addFile("8dc8bdc53dc53e1ec7943fac3b680a7c", "data/NEVADA.TR2");
        TR3FileList.addFile("28395720a88971b6dc590489ff47d9e3", "data/TITLE.TR2");
        TR3FileList.addFile("070d4a7b486c234d3e84ebaba904d48a", "data/TONYBOSS.TR2");
        TR3FileList.addFile("01e6f703493807dfd513d4fcd7f29ffd", "data/VICT.TR2");
        TR3FileList.addFile("3ae21d4e98daf1692a1eaf0acd9d6958", "data/TOMBPC.DAT");
        TR3FileList.addFile("18af2d4384904bf48c6941fb51382672", "data/TEMPLE.TR2");
        TR3FileList.addFile("ee80c9522dffc40aef5576de09ad5ded", "data/QUADCHAS.TR2");
        TR3FileList.addFile("59dc31d9020943e5ef85942df0a88c58", "data/AREA51.TR2");
        TR3FileList.addFile("438cd76e0e7be12464c3bef35d0216f5", "data/CHAMBER.TR2");
        TR3FileList.addFile("d2f6ef3fbd87a86f9c2d561765a19d89", "data/SEWER.TR2");
        TR3FileList.addFile("7b064a9d5b7cb17bd4e16261242bc940", "data/SHORE.TR2");
        TR3FileList.addFile("538f602ac876cee837a07760a3dbe3aa", "data/MINES.TR2");
        TR3FileList.addFile("9b3f54902d526472008408949f23032b", "data/ROOFS.TR2");
        TR3FileList.addFile("1630b3f25a226d51d7f4875300133e8e", "data/COMPOUND.TR2");
        TR3FileList.addFile("ab8b5f6f568432666aaf5c4d83b9f6f2", "data/CRASH.TR2");
        TR3FileList.addFile("ba54a5782912a4ef83929f687009377e", "data/OFFICE.TR2");
        TR3FileList.addFile("0275cb33c94e840859a622763865a2e9", "data/STPAUL.TR2");
        TR3FileList.addFile("c9c011b71964426ecd269c314ad5f4c1", "data/TRIBOSS.TR2");
        TR3FileList.addFile("f080de24577654474fa1ebd6d07673e2", "data/RAPIDS.TR2");
        TR3FileList.addFile("5e11d251ddb12b98ebead1883dc12d2a", "data/HOUSE.TR2");
        TR3FileList.addFile("ee2d0aa76754fe0744c47f4ad9fcd607", "fmv/Crsh_Eng.rpl");
        TR3FileList.addFile("6947a8a13b70235f9fc1aa3ea4db1da9", "fmv/Sail_Eng.rpl");
        TR3FileList.addFile("20be937db48b25b57bc88e9d47517905", "fmv/logo.rpl");
        TR3FileList.addFile("3afa8b9903af0c8dabee5f6f1eb396d0", "fmv/Intr_Eng.rpl");
        TR3FileList.addFile("24b90bdd3219facfdde240c954164a59", "fmv/Endgame.rpl");
        TR3FileList.addFile("e54ce1ac0106a76f72432db8e02c8dbf", "cuts/CUT1.TR2");
        TR3FileList.addFile("81ff9f99044738510cccacc3646fc347", "cuts/CUT2.TR2");
        TR3FileList.addFile("e93435fb9577ed5da27b8cb95e6a85f0", "cuts/CUT6.TR2");
        TR3FileList.addFile("a75d14b398ffbbca13bee7bc3ff0c080", "cuts/CUT11.TR2");
        TR3FileList.addFile("28180b6e049b439413cd657870bf8474", "cuts/CUT9.TR2");
        TR3FileList.addFile("4a061d14750d36c236ae4e2c22e75aa4", "cuts/CUT4.TR2");
        TR3FileList.addFile("19b04538646d2603308f37cea64d8e66", "cuts/CUT7.TR2");
        TR3FileList.addFile("0bfe24996a41984434de13470e359b05", "cuts/CUT12.TR2");
        TR3FileList.addFile("86290b1ac08dfb0d500357d9e861c072", "cuts/CUT5.TR2");
        TR3FileList.addFile("3135f022bceccc129c43997c2e53320c", "cuts/CUT3.TR2");
        TR3FileList.addFile("ab459301b03aab6c35327284cacbd0bd", "cuts/CUT8.TR2");
        TR3FileList.addFile("72890403f686d0a199cfcfed5296fed0", "support/info/Readme.txt");
        TR3FileList.addFile("746c9a7ab8cd2bd375f6520b5cf4ae3a", "support/info/ninjahead.gif");
        TR3FileList.addFile("3f463cc01e32b8de7542b1f9a2aa9e91", "support/info/support.htm");
        TR3FileList.addFile("bb72ad0b84f5d6cad35b6a3a7cb28ab9", "support/info/slegal.gif");
        TR3FileList.addFile("cf6df27145b94333dffa488e52a905ff", "support/info/eidos.gif");
        TR3FileList.addFile("cfaa679cd484b0709c36c8324dbf0960", "support/info/Ninja.jpg");
        TR3FileList.addFile("a78b2f1ec41c1443d04592714b9ab36c", "support/info/core.jpg");
        TR3FileList.addFile("a1030488e87c0edbdfc5b8ebeba53235", "support/info/tr3logo.jpg");


    }
    /**
     * 
     */
    void run() override {
        // Perform hard drive operations

        if (original == true && id == 3)
        {
            packOriginalGame(3, folderPath, directoryPath);
        }
        else if (original == false && id == 3573)
        {
            FileDownloader downloader;
            QUrl url("https://www.trle.net/levels/levels/2023/3573/Jonson-TheInfadaCult.zip");
            QString localFilePath = "Jonson-TheInfadaCult.zip";
            downloader.downloadFile(url, localFilePath);
            //TRLE3573FileList
            //TRLE3573
        }
    }
private:
    /**
     * 
     */
    void extractZip(const QString& zipFilename, const QString& extractPath)
    {
        QuaZip zip(zipFilename);
        if (!zip.open(QuaZip::mdUnzip))
        {
            qDebug() << "Error opening ZIP file" << Qt::endl;
            return;
        }

        QuaZipFileInfo info;
        QuaZipFile file(&zip);

        for (bool more = zip.goToFirstFile(); more; more = zip.goToNextFile())
        {
            if (zip.getCurrentFileInfo(&info) && file.open(QIODevice::ReadOnly))
            {
                QByteArray data = file.readAll();
                file.close();

                // Create directories if they don't exist
                QString filePath = extractPath + QDir::separator() + info.name;
                QString directory = filePath.left(filePath.lastIndexOf(QDir::separator()));

                QDir().mkpath(directory);

                // Create a new file on disk and write the data
                QFile outFile(filePath);
                if (outFile.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Unbuffered))
                {
                    outFile.write(data);
                    outFile.close();
                }
                else
                {
                    qDebug() << "Error opening file for writing: " << filePath << Qt::endl;
                }
            }
            else
            {
                qDebug() << "Error reading file info from ZIP archive" << Qt::endl;
            }
        }
        zip.close();
    }
    /**
     * 
     */
    const QString calculateMD5(const QString& filename)
    {
        QFile file(filename);
        if (!file.open(QIODevice::ReadOnly))
        {
            qDebug() << "Error opening file for reading: " << file.errorString();
            return "";
        }

        QCryptographicHash md5(QCryptographicHash::Md5);

        char buffer[1024];
        qint64 bytesRead;

        while ((bytesRead = file.read(buffer, sizeof(buffer))) > 0)
        {
            md5.addData(buffer, static_cast<int>(bytesRead));
        }

        file.close();
        return QString(md5.result().toHex());
    }
    /**
     * 
     */
    void packOriginalGame(int game , const QString folderPath, const QString directoryPath )
    {
        const size_t s = TR3FileList.getSize();
    

        // Create the directory if it doesn't exist
        if (!QDir(directoryPath).exists())
        {
            if (QDir().mkpath(directoryPath))
            {
                qDebug() << "Directory created successfully.";
            }
            else
            {
                qDebug() << "Error creating directory.";
                return;
            }
        }
        else
        {
            qDebug() << "Directory already exists.";
        }

        for (size_t i = 0; i < s; i++)
        {
            const QString fMd5sum = TR3FileList.getMd5sum(i);
            const QString fPath = TR3FileList.getPath(i);
            const QString sourcePath = folderPath + "/" + fPath;
            const QString destinationPath = directoryPath + "/" + fPath;

            const QString  calculated = calculateMD5(sourcePath);
            if(fMd5sum == calculated)
            {
                // Ensure the destination directory exists
                const QFileInfo destinationFileInfo(destinationPath);
                QDir destinationDir(destinationFileInfo.absolutePath());
                if (!destinationDir.exists())
                {
                    if (!QDir().mkpath(destinationDir.absolutePath()))
                    {
                        qDebug() << "Error creating destination directory.";
                        return;
                    }
                }

                // Copy the file
                if (QFile::copy(sourcePath, destinationPath))
                {
                    qDebug() << "File copy successfully.";
                }
                else
                {
                    if(QFile::exists(destinationPath))
                    {
                        if(fMd5sum == calculateMD5(destinationPath))
                            qDebug() << "File exist";
                    }
                    else
                    {
                        qDebug() << "Failed to copy file " << fPath << Qt::endl;
                        return;
                    }
                }
            }
            else
            {
                qDebug() << "Original file was modified, had" << fMd5sum
                    << " got " << calculated << " for file " << fPath << Qt::endl;
                QDir directory(directoryPath);
                if (directory.exists())
                {
                    if (directory.removeRecursively())
                    {
                        qDebug() << "Working Directory removed successfully.";
                    }
                    else
                    {
                        qDebug() << "Error removing directory.";
                    }
                }
            else
            {
                qDebug() << "Directory does not exist.";
            }
                return;
            }
        }
        QDir directory(folderPath);

        if (directory.exists())
        {
            if (directory.removeRecursively())
            {
                qDebug() << "Copied Directory removed successfully.";
            }
            else
            {
                qDebug() << "Error removing directory.";
            }
        }
        else
        {
            qDebug() << "Directory does not exist.";
        }

        // Create a symbolic link
        if (QFile::link(directoryPath, folderPath))
        {
            qDebug() << "Symbolic link created successfully.";
        }
        else
        {
            qDebug() << "Failed to create symbolic link.";
        }

    }
};




#endif
