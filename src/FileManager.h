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

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef SRC_FILEMANAGER_H_
#define SRC_FILEMANAGER_H_

#include <QString>
#include <QObject>
#include <QFile>
#include <QDir>
#include <QCryptographicHash>
#include <QDebug>
#include "miniz.h"
#include "miniz_zip.h"

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
    bool extractZip(const QString& zipFile, const QString& extractPath);
    bool checkDir(const QString& file, bool lookGameDir);
    bool checkFile(const QString& file, bool lookGameDir);
    int checkFileInfo(const QString& file, bool lookGameDir = true);
    int removeFileOrDirectory(const QString &file, bool lookGameDir);
    bool moveFilesToDirectory(
        const QString& fromLevelDirectory,
        const QString& toLevelDirectory);
    bool moveFilesToParentDirectory(const QString& directoryPath, int levelsUp);
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
    bool setUpCamp(const QString& levelDir, const QString& gameDir);

 signals:
    void fileWorkTickSignal();

 private:
    explicit FileManager(QObject *parent = nullptr)  : QObject(parent) {}

    QDir levelDir_m;
    QDir gameDir_m;
    Q_DISABLE_COPY(FileManager)
};
#endif  // SRC_FILEMANAGER_H_
