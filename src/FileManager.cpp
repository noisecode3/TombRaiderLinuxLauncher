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
#include "GameFileTree.hpp"
#include "miniz.h"
#include "miniz_zip.h"

struct StaticTrees {
    QList<GameFileTree*> data;
    StaticTrees() {
        data.append(new GameFileTree(QStringList{
            "TOMBRAID/tomb.exe",
            "dosbox.exe",
        }));
        data.append(new GameFileTree(QStringList{
            "Tomb1Main.exe",
            "cfg",
            "data",
            "shaders",
        }));
        data.append(new GameFileTree(QStringList{
            "TR1X.exe",
            "cfg",
            "data",
            "shaders",
        }));
        data.append(new GameFileTree(QStringList{
            "TR2Main.json",
            "data",
        }));
        data.append(new GameFileTree(QStringList{
            "Tomb2.exe",
            "data",
        }));
        data.append(new GameFileTree(QStringList{
            "tomb3_ConfigTool.json",
            "tomb3.exe",
            "audio",
            "data",
            "pix",
        }));
        data.append(new GameFileTree(QStringList{
            "tomb3.exe",
            "audio",
            "data",
            "Pix",
        }));
        data.append(new GameFileTree(QStringList{
            "tomb3.exe",
            "audio",
            "Data",
        }));
        data.append(new GameFileTree(QStringList{
            "tomb3.exe",
            "audio",
            "data",
            "pix",
        }));
        data.append(new GameFileTree(QStringList{
            "tomb4.exe",
            "audio",
            "Data",
        }));
        data.append(new GameFileTree(QStringList{
            "tomb4.exe",
            "audio",
            "data",
            "pix",
        }));
        data.append(new GameFileTree(QStringList{
            "audio",
            "data",
            "pix",
        }));
        data.append(new GameFileTree(QStringList{
            "PCTomb5.exe",
            "audio",
            "data",
            "pix",
        }));
    }
    ~StaticTrees() {
        for (GameFileTree* tree : data) {
            delete tree;
        }
        data.clear();
    }
};


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

const QString FileManager::calculateMD5(
        const QString& fileName, bool lookGameDir) {
    const QString path = FileManager::lookGameDir(fileName, lookGameDir);
    QFileInfo fileInfo(path);
    QString result;

    if (fileInfo.exists() && !fileInfo.isFile()) {
        qDebug() << "Error: The path is not a regular file." << path;
    } else {
        QFile file(path);
        bool status = file.open(QIODevice::ReadOnly);  // flawfinder: ignore
        if (!status) {
            qDebug()
                << "Error opening file for reading: " << file.errorString();
        } else {
            QCryptographicHash md5(QCryptographicHash::Md5);
            if (!md5.addData(&file)) {
                qWarning() << "Failed to process file for MD5 hash.";
            } else {
                result = QString(md5.result().toHex());
            }
            file.close();
        }
    }
    return result;
}

bool FileManager::extractZip(
    const QString& zipFilename,
    const QString& outputFolder) {
    bool status = false;
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
    (void)memset(&zip, 0, sizeof(zip));
    if (mz_zip_reader_init_file(
        &zip, zipPath.toUtf8().constData(), 0) == true) {
        // Extract each file in the zip archive
        quint64 numFiles = mz_zip_reader_get_num_files(&zip);
        qDebug() << "Zip file contains" << numFiles << "files";

        quint64 gotoPercent = 50;  // Percentage of total work
        quint64 lastPrintedPercent = 0;  // Last printed percentage

        for (quint64 i = 0; i < numFiles; i++) {
            mz_zip_archive_file_stat file_stat;
            if (!mz_zip_reader_file_stat(&zip, i, &file_stat)) {
                qWarning() << "Failed to get file info for file" << i
                << "in zip file" << zipPath;
                mz_zip_reader_end(&zip);
                break;
            }

            QString filename = QString::fromUtf8(file_stat.m_filename);
            if (filename.endsWith('/') == true) {
                continue;  // Skip directories
            }

            QString outFile = QString("%1/%2").arg(outputPath, filename);
            qDebug() << "Extracting" << filename;

            if (!QDir().mkpath(QFileInfo(outFile).path())) {
                qWarning() << "Failed to create directory for file" << outFile;
                mz_zip_reader_end(&zip);
                break;
            }

            if (!mz_zip_reader_extract_to_file(
                    &zip,
                    i,
                    outFile.toUtf8().constData(),
                    0)) {
                qWarning() << "Failed to extract file" << filename
                        << "from zip file" << zipPath;
                mz_zip_reader_end(&zip);
                break;
            }

            quint64 currentPercent =
                ((i + quint64(1)) * gotoPercent) / numFiles;

            if (currentPercent != lastPrintedPercent) {
                for (quint64 j = lastPrintedPercent + quint64(1);
                                j <= currentPercent; j++) {
                    emit this->fileWorkTickSignal();
                    QCoreApplication::processEvents();
                }
                lastPrintedPercent = currentPercent;
                if (currentPercent == gotoPercent) {
                    status = true;
                }
            }
        }
    } else {
        qWarning() << "Failed to open zip file" << zipPath;
    }
    // Clean up
    mz_zip_reader_end(&zip);
    qDebug() << "Unzip complete";
    return status;
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
    int status = 0;
    const QString path = FileManager::lookGameDir(file, lookGameDir);
    QFileInfo fileInfo(path);
    if (fileInfo.isDir() == true) {
        qDebug() << "The path is a directory.";
        if (fileInfo.isSymLink() == true) {
            qDebug() << "return value 1:The path is a symbolic link.";
            status = 1;
        } else {
            qDebug() << "return value 2:The path is not a symbolic link.";
            status = 2;
        }
    } else {
        qDebug() << "value 3:The path is not a directory.";
        qDebug() << "filePath " << path;
        status = 3;
    }
    return status;
}

QString FileManager::getExtraPath(const QString& levelDir) {
    const QString levelPath = QString("%1%2")
        .arg(m_levelDir.absolutePath(), levelDir);
    StaticTrees staticTrees;
    QDir dir(levelPath);
    GameFileTree tree(dir);
    tree.printTree(1);
    QString extraPath;

    for (const GameFileTree* stree : staticTrees.data) {
        extraPath = tree.matchesFromAnyNode(stree);
        if ((extraPath != QString("\0")) && (!extraPath.isEmpty())) {
            QTextStream(stdout)
                << "game tree matches: " << extraPath << Qt::endl;
            break;
        }
    }
    return levelPath + extraPath;
}

bool FileManager::linkGameDir(const QString& levelDir, const QString& gameDir) {
    bool status = false;
    const QString gamePath =  QString("%1/%2")
        .arg(m_gameDir.absolutePath(), gameDir);

    const QString levelPath = getExtraPath(levelDir);
    qDebug() << "levelPath: " << levelPath;
    qDebug() << "gamePath: " << gamePath;

    if (QFile::link(levelPath, gamePath) == true) {
        qDebug() << "Symbolic link created successfully.";
        status = true;
    } else {
        QFileInfo fileInfo(gamePath);
        if (fileInfo.isSymLink() == true) {
            (void)QFile::remove(gamePath);
            if (QFile::link(levelPath, gamePath) == true) {
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
    bool status = false;
    const QString levelPath = QString("%1%2")
        .arg(m_levelDir.absolutePath(), levelDir);
    const QString fromPath = QString("%1%2")
        .arg(levelPath, from);
    const QString toPath = QString("%1%2")
        .arg(levelPath, to);

    if (QFile::link(fromPath, toPath) == true) {
        qDebug() << "Symbolic link created successfully.";
        status = true;
    } else {
        QFileInfo i(toPath);
        if (i.isSymLink() == true) {
            (void)QFile::remove(toPath);
            if (QFile::link(fromPath, toPath) == true) {
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
        if (QDir().mkpath(path) == true) {
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

int FileManager::copyFile(const QString &gameFile,
        const QString &levelFile, bool fromGameDir) {
    int status = 0;

    const QString gamePath = QString("%1%2")
        .arg(m_gameDir.absolutePath(), gameFile);

    const QString levelPath = QString("%1%2")
        .arg(m_levelDir.absolutePath(), levelFile);

    const QString sourceFile = fromGameDir ? gamePath : levelPath;
    const QString destinationFile = fromGameDir ? levelPath : gamePath;

    // Ensure the destination directory exists
    const QFileInfo destinationFileInfo(destinationFile);
    QDir destinationDir(destinationFileInfo.absolutePath());
    if (!destinationDir.exists() == true) {
        if (!QDir().mkpath(destinationDir.absolutePath())) {
            qDebug() << "Error creating destination directory.";
            status = 1;
        }
    } else if (QFile::copy(sourceFile, destinationFile) == true) {
        qDebug() << "File copy to " << destinationFile << " successfully.";
        status = 0;
    } else {
        if (QFile::exists(destinationFile) == true) {
            qDebug() << "Target file already exist.";
            status = 2;
        } else {
            qDebug() << "Failed to copy file: " << destinationFile;
            status = 3;
        }
    }
    return status;
}

int FileManager::cleanWorkingDir(const QString &levelDir) {
    int status = 0;
    const QString directoryPath = QString("%1%2%3")
        .arg(m_levelDir.absolutePath(), m_sep, levelDir);

    QDir directory(directoryPath);
    if (directory.exists() == true) {
        if (directory.removeRecursively() == true) {
            qDebug() << "Working Directory removed successfully.";
            status = 0;
        } else {
            qDebug() << "Error removing working directory.";
            status = 1;
        }
    } else {
        qDebug() << "Error working directory seems to not exist";
    }
    return status;
}

bool FileManager::backupGameDir(const QString &gameDir) {
    bool status = false;
    const QString source = QString("%1%2")
        .arg(m_gameDir.absolutePath(), gameDir.chopped(1));
    const QString destination = QString("%1%2").arg(source, ".old");
    QDir directory;
    if (directory.rename(source, destination) == true) {
        qDebug() << "Directory renamed successfully. New path:" << destination;
        status = true;
    } else {
        qWarning() << "Failed to rename directory:" << source;
        status = false;
    }
    return status;
}

bool FileManager::moveFilesToDirectory(
    const QString& fromLevelDirectory,
    const QString& toLevelDirectory) {

    const QString directoryFromPath = QString("%1%2%3")
        .arg(m_levelDir.absolutePath(), m_sep, fromLevelDirectory);

    const QString directoryToPath = QString("%1%2%3")
        .arg(m_levelDir.absolutePath(), m_sep, toLevelDirectory);

    QDir dir(directoryFromPath);

    // Get list of all entries (files and directories) excluding '.' and '..'
    QStringList entryFileList =
        dir.entryList(QDir::Files | QDir::NoDotAndDotDot);

    // Move files and recursively move directories
    return std::all_of(
            entryFileList.cbegin(),
            entryFileList.cend(),
            [&](const QString& entry) -> bool {
        bool status = true;

        QString entryPath = QString("%1%2%3")
            .arg(directoryFromPath, m_sep, entry);
        QString destinationPath = QString("%1%2%3")
            .arg(directoryToPath, m_sep, entry);

        if (!QFile::rename(entryPath, destinationPath)) {
            qWarning() << "Failed to move file from:"
                << entryPath << "to:" << destinationPath;
            status = false;
        }
        // cppcheck-suppress misra-c2012-15.5
        return status;
    });
}

