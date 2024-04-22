#include "FileManager.h"
#include "quazip/quazip.h"
#include "quazip/quazipfile.h"

bool FileManager::setUpCamp(const QString& levelDir, const QString& gameDir)
{
    QDir levelDirPath(levelDir);
    if (levelDirPath.exists()) {
        levelDir_m.setPath(levelDir);
        QDir gameDirPath(gameDir);
        if (gameDirPath.exists()) {
            gameDir_m.setPath(gameDir);
            return true;
        }
    }
    return false;
}

const QString FileManager::calculateMD5(const QString& file, bool lookGameDir)
{
    const QString& path = lookGameDir ?
        gameDir_m.absolutePath() + QDir::separator()+file :
        levelDir_m.absolutePath() + QDir::separator()+file;

    QFile f(path);
    if (!f.open(QIODevice::ReadOnly))
    {
        qDebug() << "Error opening file for reading: " << f.errorString();
        return "";
    }

    QCryptographicHash md5(QCryptographicHash::Md5);

    char buffer[1024];
    qint64 bytesRead;

    while ((bytesRead = f.read(buffer, sizeof(buffer))) > 0)
    {
        md5.addData(buffer, static_cast<int>(bytesRead));
    }

    f.close();
    return QString(md5.result().toHex());
}
/**
 *
 */
void FileManager::extractZip(const QString& zipFile, const QString& extractDir)
{
    const QString& zipPath = levelDir_m.absolutePath() + QDir::separator()+zipFile;
    const QString& extractPath = levelDir_m.absolutePath() + QDir::separator()+extractDir;
    QuaZip zip(zipPath);
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
            else if (!filePath.endsWith('/'))
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
bool FileManager::checkDir(const QString& file, bool lookGameDir )
{
    QString path;
    if (!lookGameDir)
    {
        path = levelDir_m.absolutePath() + QDir::separator() + file;
    }
    else {
        path = gameDir_m.absolutePath() + QDir::separator() + file;
    }
    QDir directory(path);
    return directory.exists();
}

bool FileManager::checkFile(const QString& file, bool lookGameDir )
{
    QString path;
    if (!lookGameDir)
    {
        path = levelDir_m.absolutePath() + QDir::separator() + file;
    }
    else {
        path = gameDir_m.absolutePath() + QDir::separator() + file;
    }
    QFile fFile(path);
    return fFile.exists();

}



int FileManager::checkFileInfo(const QString& file, bool lookGameDir)
{
    const QString& path = lookGameDir ?
        gameDir_m.absolutePath() + QDir::separator()+file :
        levelDir_m.absolutePath() + QDir::separator()+file;

    QFileInfo fileInfo(path);
    if (fileInfo.isDir())
    {
        qDebug() << "The path is a directory.";
        if (fileInfo.isSymLink())
        {
            qDebug() << "return value 1:The path is a symbolic link.";
            return 1;
        }
        else
        {
            qDebug() << "return value 2:The path is not a symbolic link.";
            return 2;
        }
    }
    else
    {
        qDebug() << "value 3:The path is not a directory.";
        qDebug() << "filePath " << path;
        return 3;
    }
}

bool FileManager::linkGameDir(const QString& levelDir ,const QString& gameDir)
{
    const QString& l = levelDir_m.absolutePath() + levelDir;
    const QString& g = gameDir_m.absolutePath() + gameDir;
    if (QFile::link(l, g))
    {
        qDebug() << "Symbolic link created successfully.";
        return 0;
    }
    else
    {
        QFileInfo i(g);
        if (i.isSymLink())
        {
            QFile::remove(g);
            if (QFile::link(l, g))
            {
                qDebug() << "Symbolic link created successfully.";
                return 0;
            }
            else
            {
                qDebug() << "Failed to create symbolic link.";
                return 1;
            }
        }
        else
        {
            qDebug() << "Failed to create symbolic link.";
            return 1;
        }
    }
}

bool FileManager::makeRelativeLink(const QString& levelDir ,const QString& from ,const QString& to)
{
    const QString& l = levelDir_m.absolutePath() + levelDir;
    const QString& f = l + from;
    const QString& t = l + to;

    if (QFile::link(f, t))
    {
        qDebug() << "Symbolic link created successfully.";
        return 0;
    }
    else
    {
        QFileInfo i(t);
        if (i.isSymLink())
        {
            QFile::remove(t);
            if (QFile::link(f, t))
            {
                qDebug() << "Symbolic link created successfully.";
                return 0;
            }
            else
            {
                qDebug() << "Failed to create symbolic link.";
                return 1;
            }
        }
        else
        {
            qDebug() << "Failed to create symbolic link.";
            return 1;
        }
    }
}

int FileManager::removeFileOrDirectory(const QString &file, bool lookGameDir)
{
    const QString& path = lookGameDir ?
            gameDir_m.absolutePath() + QDir::separator()+file :
            levelDir_m.absolutePath() + QDir::separator()+file;

    QDir dir(path);
    if (dir.exists()) {
        // Remove directory and its contents
        if (dir.removeRecursively()) {
            qDebug() << "Directory removed successfully:" << path;
            return 0;
        }
        else
        {
            qWarning() << "Failed to remove directory:" << path;
            return 1;
        }
    }
    else
    {
        QFile file(path);
        // Check if the file exists before attempting to remove it
        if (file.exists())
        {
            if (file.remove())
            {
                qDebug() << "File removed successfully:" << path;
                return 0;
            }
            else
            {
                qWarning() << "Failed to remove file:" << path;
                return 1;
            }
        }
        else
        {
            qDebug() << "File or directory does not exist:" << path;
            return 2;
        }
    }
}


int FileManager::createDirectory(const QString &file, bool gameDir)
{
    const QString& path = gameDir ?
            gameDir_m.absolutePath() + QDir::separator()+file :
            levelDir_m.absolutePath() + QDir::separator()+file;

    // Create the directory if it doesn't exist
        if (!QDir(path).exists())
        {
            if (QDir().mkpath(path))
            {
                qDebug() << "Directory created successfully.";
                return 0;
            }
            else
            {
                qDebug() << "Error creating directory.";
                return 1;
            }
        }
        else
        {
            qDebug() << "Directory already exists.";
            return 0;
        }
}
/*
int FileManager::copyFile(const QString &gameFile, const QString &levelFile, bool fromGameDir)
{
    const QString sourcePath = (fromGameDir ? gameDir_m.path() : levelDir_m.path()) + (fromGameDir ? gameFile : levelFile);
    const QString destinationPath = (fromGameDir ? levelDir_m.path() : gameDir_m.path()) + (fromGameDir ? levelFile : gameFile);

    // Ensure the destination directory exists
    QDir destinationDir(QFileInfo(destinationPath).absolutePath());
    if (!destinationDir.exists() && !QDir().mkpath(destinationDir.absolutePath()))
    {
        qDebug() << "Error creating destination directory.";
        return 1;
    }

    // Copy the file
    if (QFile::copy(sourcePath, destinationPath))
    {
        qDebug() << "File copy to " + destinationPath + " successfully.";
        return 0;
    }
    else
    {
        if (QFile::exists(destinationPath))
        {
            qDebug() << "File already exists.";
            return 2;
        }
        else
        {
            qDebug() << "Failed to copy file " << destinationPath << Qt::endl;
            return 3;
        }
    }
}

*/



int FileManager::copyFile(const QString &gameFile, const QString &levelFile, bool fromGameDir)
{
    const QString g = fromGameDir  ? gameDir_m.absolutePath() + gameFile : levelDir_m.absolutePath() + levelFile;
    const QString l = !fromGameDir ? gameDir_m.absolutePath() + gameFile : levelDir_m.absolutePath() + levelFile;

    // Ensure the destination directory exists
    const QFileInfo destinationFileInfo(l);
    QDir destinationDir(destinationFileInfo.absolutePath());
    if (!destinationDir.exists())
    {
        if (!QDir().mkpath(destinationDir.absolutePath()))
        {
            qDebug() << "Error creating destination directory.";
            return 1;
        }
    }

    // Copy the file
    if (QFile::copy(g, l))
    {
        qDebug() << "File copy to " + l +" successfully.";
        return 0;
    }
    else
    {
        if(QFile::exists(l))
        {
            qDebug() << "File exist";
            return 2;
        }
        else
        {
            qDebug() << "Failed to copy file and dose not  exist " << l << Qt::endl;
            return 3;
        }
    }
}
int FileManager::cleanWorkingDir(const QString &levelDir)
{
    const QString& directoryPath = levelDir_m.absolutePath() + QDir::separator() + levelDir;
    QDir directory(directoryPath);
    if (directory.exists())
    {
        if (directory.removeRecursively())
        {
            qDebug() << "Working Directory removed successfully.";
            return 0;
        }
        else
        {
            qDebug() << "Error removing working directory.";
            return 1;
        }
    }
    qDebug() << "Error working directory seems to not exist";
    return 3;
}

bool FileManager::backupGameDir(const QString &gameDir)
{
    const QString& source = gameDir_m.absolutePath() + gameDir.chopped(1);
    const QString& des = source + ".old";
    QDir directory;
    if (directory.rename(source, des))
    {
        qDebug() << "Directory renamed successfully. New path:" << des;
        return true;
    }
    else
    {
        qWarning() << "Failed to rename directory:" << source;
        return false;
    }
}


bool FileManager::moveFilesToDirectory(const QString& fromLevelDirectory, const QString& toLevelDirectory)
{
    const QString& directoryFromPath = levelDir_m.absolutePath() + QDir::separator() + fromLevelDirectory;
    const QString& directoryToPath = levelDir_m.absolutePath() + QDir::separator() + toLevelDirectory;
    QDir dir(directoryFromPath);

    // Get list of all entries (files and directories) excluding '.' and '..'
    QStringList entryFileList = dir.entryList(QDir::Files | QDir::NoDotAndDotDot);

    // Move files and recursively move directories
    for (const QString& entry : entryFileList)
    {
        QString entryPath = directoryFromPath + QDir::separator() + entry;

        if (!QFile::rename(entryPath, directoryToPath + QDir::separator() + entry))
        {
            qWarning() << "Failed to move file:" << entryPath;
            return false;
        }
    }
    return true;
}

bool FileManager::moveFilesToParentDirectory(const QString& levelDirectory)
{
    const QString& directoryPath = levelDir_m.absolutePath() + QDir::separator() + levelDirectory;
    QRegExp regex("/[^/]+$"); // Matches the last component of the path starting with a slash
    int indexOfRegex = regex.indexIn(levelDirectory);

    QString tmp = levelDirectory;
    if (indexOfRegex == -1) {
        return false;
    }
    const QString& parentDirectory = tmp.remove(indexOfRegex, tmp.length() - indexOfRegex);
    const QString& directoryParentPath = levelDir_m.absolutePath() + QDir::separator() + parentDirectory;

    QDir dir(directoryPath);

    if(!moveFilesToDirectory(levelDirectory, parentDirectory))
    {
        return false;
    }

    // Get list of all entries (files and directories) excluding '.' and '..'
    QStringList entryDirList = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);

    //TODO: make it look recursively and delete empty dir

    // Move files and recursively move directories
    for (const QString& entry : entryDirList)
    {
        QString entryPath = directoryPath + QDir::separator() + entry;
        createDirectory(parentDirectory + QDir::separator() + entry, false);
        if (!moveFilesToDirectory(levelDirectory + QDir::separator() + entry, parentDirectory + QDir::separator() + entry))
        {
            qWarning() << "Failed to move directory:" << entryPath;
            return false;
        }
    }
    
    // Remove the directory if it's not the root directory
    /*
    if (dir != levelDir_m)
    {
        if (!dir.rmdir("."))
        {
            qWarning() << "Failed to remove directory:" << directoryPath;
            return false;
        }
    }
    */
    return true;
}

