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
#include "../miniz/miniz.h"
#include "../miniz/miniz_zip.h"
#include "../src/gameFileTreeData.hpp"
#include "../src/binary.hpp"


/**
 * @brief Sets the game launch directory and level directory.
 *
 * This function ensures that all file operations are contained within two directories:
 * - One for installed game files, similar to "Program Files (x86)".
 * - One for program files and downloaded levels.
 *
 * If the directories do not exist, it attempts to create them.
 *
 * @param levelDir The path to the level directory.
 * @param gameDir The path to the game directory.
 * @return `true` if both directories exist or were successfully created, `false` otherwise.
 */
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

/**
 * @brief Constructs the full path to a file in either the game or level directory.
 *
 * This function generates a full file path by appending the given file name
 * to either the game directory or the level directory, depending on the `lookGameDir` flag.
 *
 * @param file The name of the file.
 * @param lookGameDir If `true`, the path is constructed using the game directory;
 *                    otherwise, the level directory is used.
 * @return The full path to the file as a `QString`.
 */
inline const QString FileManager::getFullPath(
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

/**
 * @brief Computes the MD5 hash of a given file.
 *
 * This function calculates the MD5 checksum of a file located in either the game 
 * or level directory. If the file does not exist or is not a regular file, an 
 * error message is logged, and an empty string is returned.
 *
 * @param fileName The name of the file whose MD5 checksum is to be calculated.
 * @param lookGameDir If `true`, the function searches for the file in the game directory;
 *                    otherwise, it looks in the level directory.
 * @return The MD5 hash of the file as a hexadecimal `QString`, or an empty string
 *         if an error occurs.
 */
const QString FileManager::calculateMD5(
        const QString& fileName, bool lookGameDir) {
    const QString path = FileManager::getFullPath(fileName, lookGameDir);
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

/**
 * @brief Extracts the contents of a ZIP archive into a specified output folder.
 *
 * This function unzips a given archive file from the level directory and extracts 
 * its contents to a specified output folder. It ensures that the output directory
 * exists and handles errors related to file extraction.
 *
 * @param zipFilename The name of the ZIP archive to extract.
 * @param outputFolder The name of the destination folder where files will be extracted.
 * @param executable The name of the game launch executable.
 * @return `true` if extraction is successful, otherwise `false`.
 *
 * @note This function uses `miniz` for ZIP operations.
 * @warning If extraction fails at any point, the function will attempt to clean up
 *          and terminate extraction, potentially leaving incomplete files.
 * @signal fileWorkTickSignal() is emitted to indicate extraction progress.
 */
bool FileManager::extractZip(
    const QString& zipFilename,
    const QString& outputFolder,
    const QString& executable) {
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
                    createLinkToExe(outputPath, executable);
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
 * @brief Checks if a given directory exists.
 *
 * This function constructs the full path of a directory based on the given
 * filename and the specified directory type (game directory or level directory).
 * It then verifies whether the directory exists.
 *
 * @param file The name of the directory to check.
 * @param lookGameDir If `true`, the directory is checked in the game directory;
 *                    otherwise, it is checked in the level directory.
 * @return `true` if the directory exists, otherwise `false`.
 *
 * @note This function does not check if the path is actually a directory.
 */
bool FileManager::checkDir(const QString& file, bool lookGameDir) {
    const QString path = FileManager::getFullPath(file, lookGameDir);
    QDir directory(path);
    return directory.exists();
}

/**
 * @brief Checks if a given file exists.
 *
 * This function constructs the full path of a file based on the given filename 
 * and the specified directory type (game directory or level directory).
 * It then verifies whether the file exists.
 *
 * @param file The name of the file to check.
 * @param lookGameDir If `true`, the file is checked in the game directory;
 *                    otherwise, it is checked in the level directory.
 * @return `true` if the file exists, otherwise `false`.
 *
 * @note This function does not check if the path is a regular file.
 */
bool FileManager::checkFile(const QString& file, bool lookGameDir) {
    const QString path = FileManager::getFullPath(file, lookGameDir);
    QFile fFile(path);
    return fFile.exists();
}

/**
 * @brief Checks the type of a file path and returns a status code.
 *
 * This function determines whether the given path corresponds to a directory, 
 * a symbolic link, or a regular file. It logs debug messages and returns a 
 * status code indicating the type.
 *
 * @param file The file or directory name to check.
 * @param lookGameDir If `true`, the path is checked in the game directory;
 *                    otherwise, it is checked in the level directory.
 * @return An integer status code:
 *         - `1` if the path is a symbolic link.
 *         - `2` if the path is a directory but not a symbolic link.
 *         - `3` if the path is neither a directory nor a symbolic link.
 */
int FileManager::checkFileInfo(const QString& file, bool lookGameDir) {
    int status = 0;
    const QString path = FileManager::getFullPath(file, lookGameDir);
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

/**
 * @brief Determines an additional path to the executable within a level directory.
 *
 * This function constructs a path to a level directory and searches for
 * an extra executable path by comparing the directory structure with predefined
 * static trees.
 *
 * @param levelDir The relative path of the level directory.
 * @return A QString containing the path, including the extra executable path
 *         if a match is found.
 */
QString FileManager::getExtraPathToExe(const QString& levelPath) {
    qDebug() << "levelPath :" << levelPath;
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

/**
 * @brief Creates a symbolic link to an executable file if it does not exist.
 *
 * This function checks whether the expected executable file is present at the given path.
 * If it is missing, the function attempts to determine the actual executable file and
 * creates a symbolic link pointing to it.
 *
 * @param path The directory where the executable file is expected to be found.
 * @param expectedFileName The name of the expected executable file.
 *
 * @note If the expected executable is not found, the function attempts to locate
 *       an alternative executable and create a relative symbolic link to it.
 */
void FileManager::createLinkToExe(
        const QString& path,
        const QString& expectedFileName) {
    const QString pathToKnownPattern = getExtraPathToExe(path);
    const QString pathToExpectedExe = QString("%1/%2")
        .arg(pathToKnownPattern, expectedFileName);

    qDebug() << "pathToExpectedExe :" << pathToExpectedExe;
    qDebug() << "pathToKnownPattern :" << pathToKnownPattern;

    if (!QFile(pathToExpectedExe).exists()) {
        QDir dir(pathToKnownPattern);
        const QString finalExe = decideExe(dir);
        qDebug() << "finalExe :" << finalExe;

        if (finalExe.isEmpty() == true) {
            qDebug() << "error: could not find the exe file for the game!";
        } else {
            (void)makeRelativeLink(
                pathToKnownPattern,
                finalExe,
                expectedFileName);
        }
    }
}

/**
 * @brief Creates a symbolic link from a level directory to a game directory.
 *
 * This function attempts to create a symbolic link from the resolved level path
 * to the specified game directory. If a link already exists, it is removed
 * and recreated.
 *
 * @param levelDir The relative path of the level directory.
 * @param gameDir The relative path of the game directory.
 * @return True if the symbolic link is successfully created, false otherwise.
 */
bool FileManager::linkGameDir(const QString& levelDir, const QString& gameDir) {
    bool status = false;
    const QString gamePath =  QString("%1/%2")
        .arg(m_gameDir.absolutePath(), gameDir);
    const QString levelPath = getExtraPathToExe(
        m_levelDir.absolutePath() + levelDir);

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

/**
 * @brief Creates a symbolic link within a level directory.
 *
 * This function attempts to create a symbolic link from the `from` path to the `to` path,
 * both relative to the specified `levelDir`. If a symlink already exists, it is removed
 * and recreated.
 *
 * @param levelDir The base level directory.
 * @param from The source file or directory to link from.
 * @param to The destination link path.
 * @return True if the symbolic link was successfully created, false otherwise.
 */
bool FileManager::makeRelativeLink(
        const QString& levelDir,
        const QString& from,
        const QString& to) {
    bool status = false;

    const QString fromPath = QString("%1/%2")
        .arg(levelDir, from);
    const QString toPath = QString("%1/%2")
        .arg(levelDir, to);

    qDebug() << "calling makeRelativeLink";
    qDebug() << "levelDir :" << levelDir;
    qDebug() << "fromPath :" << fromPath;
    qDebug() << "toPath :" << toPath;

    if (QFile::link(fromPath, toPath) == true) {
        qDebug() << "Symbolic link created successfully.";
        status = true;
    } else {
        QFileInfo i(toPath);
        if (i.isSymLink() == true) {
            (void)QFile::remove(toPath);
            if (QFile::link(fromPath, to) == true) {
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

/**
 * @brief Removes a file or directory, including its contents if applicable.
 *
 * This function attempts to delete a file or directory at the specified path.
 * If the path points to a directory, it is removed recursively along with its contents.
 * If the path points to a file, the file is deleted. If neither exists, an appropriate
 * status code is returned.
 *
 * @param fileName The relative or absolute path of the file or directory to be removed.
 * @param lookGameDir If true, the function retrieves the full path within the game directory.
 *
 * @return qint64 Status code:
 *         - 0: File or directory removed successfully.
 *         - 1: Failed to remove file or directory.
 *         - 2: File or directory does not exist.
 */
qint64 FileManager::removeFileOrDirectory(
        const QString &fileName,
        bool lookGameDir) {
    qint64 status = 0;
    const QString path = FileManager::getFullPath(fileName, lookGameDir);
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

/**
 * @brief Creates a directory at the specified path if it does not already exist.
 *
 * This function checks whether the given directory exists. If it does not, the function
 * attempts to create it. If the directory already exists, no action is taken.
 *
 * @param file The relative or absolute path of the directory to be created.
 * @param gameDir If true, the function retrieves the full path within the game directory.
 *
 * @return int Status code:
 *         - 0: Directory created successfully or already exists.
 *         - 1: Failed to create directory.
 */
int FileManager::createDirectory(const QString &file, bool gameDir) {
    qint64 status = 0;
    const QString path = FileManager::getFullPath(file, gameDir);
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

/**
 * @brief Copies a file between the game directory and the level directory.
 *
 * This function copies a file from either the game directory to the level directory
 * or vice versa, depending on the value of `fromGameDir`. It ensures that the 
 * destination directory exists before attempting the copy operation.
 *
 * @param gameFile The relative path of the file in the game directory.
 * @param levelFile The relative path of the file in the level directory.
 * @param fromGameDir If true, the file is copied from the game directory to the level directory.
 *                    If false, the file is copied from the level directory to the game directory.
 *
 * @return int Status code:
 *         - 0: File copied successfully.
 *         - 1: Failed to create the destination directory.
 *         - 2: Target file already exists.
 *         - 3: Failed to copy the file.
 */
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

/**
 * @brief Removes the specified working directory and its contents.
 *
 * This function deletes the specified working directory within the level directory.
 * If the directory exists, it is removed recursively along with all its contents.
 *
 * @param levelDir The relative path of the working directory to be cleaned.
 *
 * @return int Status code:
 *         - 0: Directory removed successfully.
 *         - 1: Failed to remove the directory.
 *         - (No specific code for non-existent directory, but logs an error message.)
 */
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

/**
 * @brief Creates a backup of the specified game directory by renaming it.
 *
 * This function renames the given game directory by appending ".old" to its name.
 * If the renaming is successful, the backup is considered complete.
 *
 * @param gameDir The relative path of the game directory to be backed up.
 *
 * @return bool Status indicating success or failure:
 *         - true: Directory successfully renamed (backup created).
 *         - false: Failed to rename the directory.
 */
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

bool FileManager::isFullPath(const QString& path) {
    return path.startsWith(QDir::homePath());
}
