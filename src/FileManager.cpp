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
 */

#include "FileManager.hpp"
#include <QFile>
#include <QIODevice>
#include <QDir>
#include <QDebug>
#include <QDirIterator>
#include <QtCore>
#include <QByteArray>
#include <QDataStream>
#include "gameTree.hpp"
#include "miniz.h"
#include "miniz_zip.h"

bool FileManager::setUpCamp(const QString& levelDir, const QString& gameDir) {
    bool status = true;

    QDir levelDirPath(levelDir);
    if (!levelDirPath.exists() && !levelDirPath.mkpath(levelDir)) {
        qWarning() << "Failed to create level directory:" << levelDir;
        status = false;
    } else {
        m_levelDir.setPath(levelDir);
    }

    QDir gameDirPath(gameDir);
    if (!gameDirPath.exists() && !gameDirPath.mkpath(gameDir)) {
        qWarning() << "Failed to create game directory:" << gameDir;
        status = false;
    } else {
        m_gameDir.setPath(gameDir);
    }

    return status;
}

inline const QString FileManager::lookGameDir(
        const QString& file,
        bool lookGameDir) {
    QString path;
    if (lookGameDir) {
        path = QString("%1%2%3").arg(m_gameDir.absolutePath(), m_sep, file);
    } else {
        path = QString("%1%2%3").arg(m_levelDir.absolutePath(), m_sep, file);
    }
    return path;
}

const QString FileManager::calculateMD5(const QString& file, bool lookGameDir) {
    const QString path = FileManager::lookGameDir(file, lookGameDir);
    QFileInfo fileInfo(path);

    if (fileInfo.exists() && !fileInfo.isFile()) {
        qDebug() << "Error: The path is not a regular file." << path;
        return"";
    }

    QFile f(path);
    if (!f.open(QIODevice::ReadOnly)) {  // flawfinder: ignore
        qDebug() << "Error opening file for reading: " << f.errorString();
        return "";
    }

    QCryptographicHash md5(QCryptographicHash::Md5);

    std::array<char, 1024> buffer;
    const int size = buffer.size();
    int bytesRead;

    // flawfinder: ignore
    while ((bytesRead = f.read(buffer.data(), size)) > 0) {
        md5.addData(buffer.data(), bytesRead);
    }

    f.close();
    return QString(md5.result().toHex());
}

bool FileManager::extractZip(
    const QString& zipFilename,
    const QString& outputFolder) {
    const QString& zipPath =
        QString("%1%2%3").arg(m_levelDir.absolutePath(), m_sep, zipFilename);
    const QString& outputPath =
        QString("%1%2%3").arg(m_levelDir.absolutePath(), m_sep, outputFolder);

    qDebug() << "Unzipping file" << zipFilename << "to" << outputPath;

    // Create output folder if it doesn't exist
    QDir dir(outputPath);
    if (!dir.exists()) {
        dir.mkpath(".");
    }

    // Open the zip file
    mz_zip_archive zip;
    memset(&zip, 0, sizeof(zip));
    mz_bool result =
        mz_zip_reader_init_file(&zip, zipPath.toUtf8().constData(), 0);
    if (!result) {
        qWarning() << "Failed to open zip file" << zipPath;
        return false;
    }

    // Extract each file in the zip archive
    mz_uint numFiles = mz_zip_reader_get_num_files(&zip);
    qDebug() << "Zip file contains" << numFiles << "files";

    unsigned int gotoPercent = 50;  // Percentage of total work
    unsigned int lastPrintedPercent = 0;  // Last printed percentage

    for (uint i = 0; i < numFiles; i++) {
        mz_zip_archive_file_stat file_stat;
        if (!mz_zip_reader_file_stat(&zip, i, &file_stat)) {
            qWarning() << "Failed to get file info for file" << i
            << "in zip file" << zipPath;
            mz_zip_reader_end(&zip);
            return false;
        }

        QString filename = QString::fromUtf8(file_stat.m_filename);
        if (filename.endsWith('/')) {
            continue;  // Skip directories
        }

        QString outFile = outputPath + "/" + filename;
        qDebug() << "Extracting" << filename;

        if (!QDir().mkpath(QFileInfo(outFile).path())) {
            qWarning() << "Failed to create directory for file" << outFile;
            mz_zip_reader_end(&zip);
            return false;
        }

        if (!mz_zip_reader_extract_to_file(
                &zip,
                i,
                outFile.toUtf8().constData(),
                0)) {
            qWarning() << "Failed to extract file" << filename
                    << "from zip file" << zipPath;
            mz_zip_reader_end(&zip);
            return false;
        }

        unsigned int currentPercent = ((i + 1) * gotoPercent) / numFiles;

        if (currentPercent != lastPrintedPercent) {
            for (unsigned int j = lastPrintedPercent + 1;
                            j <= currentPercent; j++) {
                emit this->fileWorkTickSignal();
                QCoreApplication::processEvents();
            }
            lastPrintedPercent = currentPercent;
        }
    }

    // Ensure any remaining progress is emitted
    for (unsigned int j = lastPrintedPercent + 1; j <= gotoPercent; j++) {
        emit this->fileWorkTickSignal();
        QCoreApplication::processEvents();
    }

    // Clean up
    mz_zip_reader_end(&zip);
    qDebug() << "Unzip complete";
    return true;
}

/**
 *
 */
bool FileManager::checkDir(const QString& file, bool lookGameDir) {
    const QString path = FileManager::lookGameDir(file, lookGameDir);
    QDir directory(path);
    return directory.exists();
}

bool FileManager::checkFile(const QString& file, bool lookGameDir) {
    const QString path = FileManager::lookGameDir(file, lookGameDir);
    QFile fFile(path);
    return fFile.exists();
}

int FileManager::checkFileInfo(const QString& file, bool lookGameDir) {
    const QString path = FileManager::lookGameDir(file, lookGameDir);
    QFileInfo fileInfo(path);
    if (fileInfo.isDir() == true) {
        qDebug() << "The path is a directory.";
        if (fileInfo.isSymLink() == true) {
            qDebug() << "return value 1:The path is a symbolic link.";
            return 1;
        } else {
            qDebug() << "return value 2:The path is not a symbolic link.";
            return 2;
        }
    } else {
        qDebug() << "value 3:The path is not a directory.";
        qDebug() << "filePath " << path;
        return 3;
    }
}

bool FileManager::linkGameDir(const QString& levelDir, const QString& gameDir) {
    bool status = false;
    const QString levelPath = QString("%1%2")
        .arg(m_levelDir.absolutePath(), levelDir);
    const QString gamePath =  QString("%1%2")
        .arg(m_gameDir.absolutePath(), gameDir);

    test(levelPath);  // here we just output the directory tree for now..

    if (QFile::link(levelPath, gamePath)) {
        qDebug() << "Symbolic link created successfully.";
        return 0;
    } else {
        QFileInfo fileInfo(gamePath);
        if (fileInfo.isSymLink() == true) {
            QFile::remove(gamePath);
            if (QFile::link(levelPath, gamePath)) {
                qDebug() << "Symbolic link created successfully.";
                status = true;
            } else {
                qDebug() << "Failed to create symbolic link.";
                status = false;
            }
        } else {
            qDebug() << "Failed to create symbolic link.";
            status = false;
        }
    }
    return status;
}

bool FileManager::makeRelativeLink(
        const QString& levelDir,
        const QString& from,
        const QString& to) {
    const QString& levelPath = m_levelDir.absolutePath() + levelDir;
    const QString& fromPath = levelPath + from;
    const QString& toPath = levelPath + to;

    if (QFile::link(fromPath, toPath)) {
        qDebug() << "Symbolic link created successfully.";
        return 0;
    } else {
        QFileInfo i(toPath);
        if (i.isSymLink()) {
            QFile::remove(toPath);
            if (QFile::link(fromPath, toPath)) {
                qDebug() << "Symbolic link created successfully.";
                return 0;
            } else {
                qDebug() << "Failed to create symbolic link.";
                return 1;
            }
        } else {
            qDebug() << "Failed to create symbolic link.";
            return 1;
        }
    }
}

qint64 FileManager::removeFileOrDirectory(
        const QString &fileName,
        bool lookGameDir) {
    qint64 status = 0;
    const QString path = FileManager::lookGameDir(fileName, lookGameDir);
    QDir dir(path);
    if (dir.exists() == true) {
        // Remove directory and its contents
        if (dir.removeRecursively() == true) {
            qDebug() << "Directory removed successfully:" << path;
            status = 0;
        } else {
            qWarning() << "Failed to remove directory:" << path;
            status = 1;
        }
    } else {
        QFile file(path);
        // Check if the file exists before attempting to remove it
        if (file.exists() == true) {
            if (file.remove() == true) {
                qDebug() << "File removed successfully:" << path;
                status = 0;
            } else {
                qWarning() << "Failed to remove file:" << path;
                status = 1;
            }
        } else {
            qDebug() << "File or directory does not exist:" << path;
            status = 2;
        }
    }
    return status;
}

int FileManager::createDirectory(const QString &file, bool gameDir) {
    qint64 status = 0;
    const QString path = FileManager::lookGameDir(file, gameDir);
    // Create the directory if it doesn't exist
    if (!QDir(path).exists()) {
        if (QDir().mkpath(path)) {
            qDebug() << "Directory created successfully.";
            status = 0;
        } else {
            qDebug() << "Error creating directory.";
            status = 1;
        }
    } else {
        qDebug() << "Directory already exists.";
        status = 0;
    }
    return status;
}

int FileManager::copyFile(
    const QString &gameFile,
    const QString &levelFile,
    bool fromGameDir) {

    const QString& gamePath = m_gameDir.absolutePath() + gameFile;
    const QString& levelPath = m_levelDir.absolutePath() + levelFile;

    const QString& g = fromGameDir ? gamePath : levelPath;
    const QString& l = fromGameDir ? levelPath : gamePath;

    // Ensure the destination directory exists
    const QFileInfo destinationFileInfo(l);
    QDir destinationDir(destinationFileInfo.absolutePath());
    if (!destinationDir.exists()) {
        if (!QDir().mkpath(destinationDir.absolutePath())) {
            qDebug() << "Error creating destination directory.";
            return 1;
        }
    }

    // Copy the file
    if (QFile::copy(g, l)) {
        qDebug() << "File copy to " + l +" successfully.";
        return 0;
    } else {
        if (QFile::exists(l)) {
            qDebug() << "File exist";
            return 2;
        } else {
            qDebug() << "Failed to copy file and dose not exist "
                << l << Qt::endl;
            return 3;
        }
    }
}

int FileManager::cleanWorkingDir(const QString &levelDir) {
    const QString& directoryPath =
        m_levelDir.absolutePath() + m_sep + levelDir;

    QDir directory(directoryPath);
    if (directory.exists()) {
        if (directory.removeRecursively()) {
            qDebug() << "Working Directory removed successfully.";
            return 0;
        } else {
            qDebug() << "Error removing working directory.";
            return 1;
        }
    }
    qDebug() << "Error working directory seems to not exist";
    return 3;
}

bool FileManager::backupGameDir(const QString &gameDir) {
    const QString& source = m_gameDir.absolutePath() + gameDir.chopped(1);
    const QString& des = source + ".old";
    QDir directory;
    if (directory.rename(source, des)) {
        qDebug() << "Directory renamed successfully. New path:" << des;
        return true;
    } else {
        qWarning() << "Failed to rename directory:" << source;
        return false;
    }
}

bool FileManager::moveFilesToDirectory(
    const QString& fromLevelDirectory,
    const QString& toLevelDirectory) {

    const QString& directoryFromPath =
        m_levelDir.absolutePath() + m_sep + fromLevelDirectory;

    const QString& directoryToPath =
        m_levelDir.absolutePath() + m_sep + toLevelDirectory;

    QDir dir(directoryFromPath);

    // Get list of all entries (files and directories) excluding '.' and '..'
    QStringList entryFileList =
        dir.entryList(QDir::Files | QDir::NoDotAndDotDot);

    // Move files and recursively move directories
    bool allMoved = std::all_of(
            entryFileList.cbegin(),
            entryFileList.cend(),
            [&](const QString& entry) {
        QString entryPath = directoryFromPath + m_sep + entry;
        if (!QFile::rename(entryPath, directoryToPath + m_sep + entry)) {
            qWarning() << "Failed to move file:" << entryPath;
            return false;
        }
        return true;
    });
    return allMoved;
}

bool FileManager::moveFilesToParentDirectory(
    const QString& levelDirectory,
    int levelsUp) {

    const QString& sep = QDir::separator();
    QDir levelDirectoryFullPath(m_levelDir.absolutePath() + sep +
        levelDirectory);

    if (!levelDirectoryFullPath.exists()) {
        qWarning() << "Directory does not exist:"
            << levelDirectoryFullPath.absolutePath();
        return false;
    }

    QDir levelDirectoryUpPath = levelDirectoryFullPath;

    for (int i = 0; i < levelsUp; ++i) {
        if (!levelDirectoryUpPath.cdUp()) {
            qWarning() << "Failed to access parent directory at level "
                << i + 1 << " from:" << levelDirectoryFullPath.absolutePath();
            return false;
        }
    }

    QFlags listFlags(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);
    QFileInfoList fileList = levelDirectoryFullPath.entryInfoList(listFlags);

    // Move all files and directories to the parent directory
    for (const QFileInfo& fileInfo : fileList) {
        QString srcPath = fileInfo.absoluteFilePath();
        QString destPath = levelDirectoryUpPath.absolutePath() + sep +
            fileInfo.fileName();

        if (fileInfo.isDir()) {
            // Move the directory recursively
            QDir srcDir(srcPath);
            if (!srcDir.rename(srcPath, destPath)) {
                qWarning() << "Failed to move directory:" << srcPath;
                return false;
            }
        } else {
            // Move the file
            if (!QFile::rename(srcPath, destPath)) {
                qWarning() << "Failed to move file:" << srcPath;
                return false;
            }
        }
    }

    QDir parentDir = levelDirectoryFullPath;  // Create a copy
    parentDir.cdUp();  // Move to the parent directory

    if (!parentDir.rmdir(levelDirectoryFullPath.dirName())) {
        qWarning() << "Failed to remove directory:"
            << levelDirectoryFullPath.absolutePath();
        return false;
    }
    return true;
}
