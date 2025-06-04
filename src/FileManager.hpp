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

#ifndef SRC_FILEMANAGER_HPP_
#define SRC_FILEMANAGER_HPP_

#include <QString>
#include <QObject>
#include <QFile>
#include <QDir>
#include <QByteArray>
#include <QCryptographicHash>
#include <QDebug>

class FileManager : public QObject {
    Q_OBJECT

 public:
    static FileManager& getInstance() {
        // cppcheck-suppress threadsafety-threadsafety
        static FileManager instance;
        return instance;
    }
    const QString getFullPath(const QString& file, bool lookGameDir);
    const QString calculateMD5(const QString& file, bool lookGameDir);
    bool extractZip(
        const QString& zipFilename,
        const QString& outputFolder,
        const QString& executable);

    bool checkDir(const QString& file, bool lookGameDir);
    bool checkFile(const QString& file, bool lookGameDir);
    int checkFileInfo(const QString& file, bool lookGameDir);
    qint64 removeFileOrDirectory(const QString &file, bool lookGameDir);

    int createDirectory(const QString &file, bool gameDir);
    int copyFile(
        const QString &gameFile,
        const QString &levelFile,
        bool fromGameDir);

    bool makeRelativeLink(
        const QString& levelDir,
        const QString& from,
        const QString& to);

    int cleanWorkingDir(const QString &levelDir);
    bool backupGameDir(const QString &gameDir);
    bool linkGameDir(const QString& levelDir, const QString& gameDir);
    void createLinkToExe(const QString& path, const QString& expectedFileName);
    QString getExtraPathToExe(const QString& levelPath);
    bool ensureDirectoryExists(const QString& dirPath, const QDir& dir);
    bool setUpCamp(const QString& levelDir, const QString& gameDir);


 signals:
    void fileWorkTickSignal();

 private:
    FileManager() {}
    bool isFullPath(const QString& path);

    QDir m_levelDir;
    QDir m_gameDir;
    const QString m_sep = QDir::separator();
    Q_DISABLE_COPY(FileManager)
};

#endif  // SRC_FILEMANAGER_HPP_
