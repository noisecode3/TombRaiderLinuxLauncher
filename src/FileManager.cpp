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
#include "../src/FileManager.hpp"
#include <QFile>
#include <QIODevice>
#include <QDir>
#include <QDebug>
#include <QDirIterator>
#include <QtCore>
#include <QByteArray>
#include <QDataStream>
#include "../src/GameFileTree.hpp"
#include "../miniz/miniz.h"  // IWYU pragma: keep
#include "../miniz/miniz_zip.h"
#include "../src/gameFileTreeData.hpp"
#include "../src/binary.hpp"
#include "../src/Path.hpp"

bool FileManager::backupGameDir(Path path) {
    bool status = false;
    Path backupPath = path;
    backupPath << ".old";

    if (QDir().rename(path.get(), backupPath.get()) == true) {
        qDebug() << "Directory renamed successfully. New path:"
                                                    << backupPath.get();
        status = true;
    } else {
        qWarning() << "Failed to rename directory:" << path.get();
        status = false;
    }
    return status;
}

const QString FileManager::calculateMD5(Path path) {
    QString result;

    if (path.exists() && !path.isFile()) {
        qDebug() << "Error: The path is not a regular file." << path.get();
    } else {
        QFile file(path.get());
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

int FileManager::checkFileInfo(Path path) {
    int status = 0;

    if (path.exists() == true) {
        qDebug() << "The path is a directory.";
        if (path.isSymLink() == true) {
            qDebug() << "return value 1:The path is a symbolic link.";
            status = 1;
        } else {
            qDebug() << "return value 2:The path is not a symbolic link.";
            status = 2;
        }
        if (!path.isDir()) {
            qDebug() << "value 3:The path is not a directory.";
            qDebug() << "filePath " << path.get();
            status = 3;
        }
    }
    return status;
}

int FileManager::cleanWorkingDir(Path path) {
    int status = 0;

    if (path.isDir() == true) {
        if (QDir(path.get()).removeRecursively() == true) {
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

int FileManager::copyFile(Path from, Path to) {
    int status = 0;

    // Ensure the destination directory exists
    if (!QDir(to.getDir()).exists() == true) {
        if (!QDir().mkpath(to.getDir())) {
            qDebug() << "FileManager: Error creating destination directory.";
            status = 1;
        }
    }

    // Ensure the destination file/dir dose NOT exists
    if ((status == 0) &&  to.exists() == true) {
        qDebug() << "FileManager: Target file already exist.";
        status = 2;
    }

    if ((status == 0) && (QFile::copy(from.get(), to.get()) == true)) {
        qDebug() << "File copy to " << to.get() << " successfully.";
    } else {
        qDebug() << "FileManager Failed to copy the file.";
        status = 3;
    }

    return status;
}

int FileManager::createDirectory(Path path) {
    qint64 status = 0;

    // Create the directory if it doesn't exist
    if (!path.exists()) {
        if (QDir().mkpath(path.get()) == true) {
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

void FileManager::createLinkToExe(Path path, const QString& expectedFileName) {
    path = getExtraPathToExe(path);
    path << expectedFileName;

    qDebug() << "path :" << path.get();
    qDebug() << "pathDir :" << path.getDir();

    if (!path.exists()) {
        QDir dir(path.getDir());
        const QString finalExe = decideExe(dir);
        qDebug() << "finalExe :" << finalExe;

        if (finalExe.isEmpty() == true) {
            qDebug() << "error: could not find the exe file for the game!";
        } else {
            (void)makeRelativeLink(
                path,
                finalExe,
                expectedFileName);
        }
    }
}

bool FileManager::extractZip(
    const QString& zipFilename,
    const QString& outputFolder,
    const QString& executable) {
    bool status = false;
    const QString& zipPath =
        QString("%1%2%3").arg(m_levelDir.absolutePath(), m_sep, zipFilename);
    const QString& outputPath =
        QString("%1%2%3").arg(m_levelDir.absolutePath(), m_sep, outputFolder);

    Path oPath = (Path(Path::resource) << outputFolder);

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
                    createLinkToExe(oPath, executable);
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

Path FileManager::getExtraPathToExe(Path path) {
    qDebug() << "levelPath :" << path.get();

    StaticTrees staticTrees;
    GameFileTree tree(path);
    tree.printTree(1);

    QStringList extraPath;

    for (const GameFileTree* stree : staticTrees.data) {
        extraPath = tree.matchesFromAnyNode(stree);
        if (!extraPath.isEmpty()) {
            QString joined = extraPath.join(QDir::separator());
            QTextStream(stdout) << "game tree matches: " << joined << Qt::endl;

            for (int i = extraPath.size() - 1; i >= 0; --i) {
                path << extraPath[i];
            }

            break;
        }
    }

    return path;
}

bool FileManager::linkGameDir(Path from, Path to) {
    bool status = false;

    qDebug() << "levelPath: " << from.get();
    qDebug() << "gamePath: " << to.get();

    if (QFile::link(from.get(), to.get()) == true) {
        qDebug() << "Symbolic link created successfully.";
        status = true;
    } else {
        if (to.isSymLink() == true) {
            (void)QFile::remove(to.get());
            if (QFile::link(from.get(), to.get()) == true) {
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
        const Path& levelDir,
        const QString& from,
        const QString& to) {
    bool status = false;

    Path fromPath = levelDir;
    fromPath << from;
    Path toPath = levelDir;

    if (QFile::link(fromPath.get(), to) == true) {
        qDebug() << "Symbolic link created successfully.";
        status = true;
    } else {
        if (toPath.isSymLink() == true) {
            (void)QFile::remove(toPath.get());
            if (QFile::link(fromPath.get(), to) == true) {
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

qint64 FileManager::removeFileOrDirectory(Path path) {
    qint64 status = 2;

    if (path.isDir() == true) {
        // Remove directory and its contents
        if (QDir(path.get()).removeRecursively() == true) {
            qDebug() << "Directory removed successfully:" << path.get();
            status = 0;
        } else {
            qWarning() << "Failed to remove directory:" << path.get();
            status = 1;
        }
    } else if (path.isFile()) {
        QFile file(path.get());
        // Check if the file exists before attempting to remove it
        if (file.remove() == true) {
            qDebug() << "File removed successfully:" << path.get();
            status = 0;
        } else {
            qWarning() << "Failed to remove file:" << path.get();
            status = 1;
        }
    } else {
        qDebug() << "File or directory does not exist:" << path.get();
    }
    return status;
}

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

