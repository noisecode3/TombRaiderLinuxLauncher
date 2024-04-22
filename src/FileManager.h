#ifndef FILEMANAGER_H
#define FILEMANAGER_H

#include <QObject>
#include <QFile>
#include <QDir>
#include <QCryptographicHash>
#include <QDebug>

class FileManager : public QObject
{
    Q_OBJECT
public:
    static FileManager& getInstance()
    {
        static FileManager instance;
        return instance;
    }

    const QString calculateMD5(const QString& file, bool lookGameDir);
    void extractZip(const QString& zipFile, const QString& extractPath);
    bool checkDir(const QString& file, bool lookGameDir);
    bool checkFile(const QString& file, bool lookGameDir );
    int checkFileInfo(const QString& file, bool lookGameDir=true);
    int removeFileOrDirectory(const QString &file, bool lookGameDir);
    bool moveFilesToDirectory(const QString& fromLevelDirectory, const QString& toLevelDirectory);
    bool moveFilesToParentDirectory(const QString& directoryPath);
    int createDirectory(const QString &file, bool gameDir);
    int copyFile(const QString &gameFile, const QString &levelFile, bool fromGameDir);
    bool makeRelativeLink(const QString& levelDir ,const QString& from, const QString& to);
    int cleanWorkingDir(const QString &levelDir);
    bool backupGameDir(const QString &gameDir);
    bool linkGameDir(const QString& levelDir ,const QString& gameDir);
    bool setUpCamp(const QString& levelDir, const QString& gameDir);
private:

    FileManager(QObject *parent = nullptr)  : QObject(parent) {};
    Q_DISABLE_COPY(FileManager)

    QDir levelDir_m;
    QDir gameDir_m;
};
#endif // FILEMANAGER_H
