#include "fileop.h"
#include "quazip/quazip.h"
#include "quazip/quazipfile.h"

FileManager::FileManager(QObject *parent){}

const QString FileManager::calculateMD5(const QString& filename)
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
void FileManager::extractZip(const QString& zipFilename, const QString& extractPath)
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
