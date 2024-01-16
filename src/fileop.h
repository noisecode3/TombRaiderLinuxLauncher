#ifndef FILEOP_H
#define FILEOP_H

#include <QObject>
#include <QFile>
#include <QDir>
#include <QCryptographicHash>
#include <QDebug>

class FileManager : public QObject
{
    Q_OBJECT
public:
    explicit FileManager(QObject *parent = 0);
    const QString calculateMD5(const QString& filename);
    void extractZip(const QString& zipFilename, const QString& extractPath);

private:

};
#endif // FILEOP_H
