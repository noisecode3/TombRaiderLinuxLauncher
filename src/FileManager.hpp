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
#include "../src/Data.hpp"
#include "../src/gameFileTreeData.hpp"
#include "../src/Path.hpp"

    const StaticTrees m_staticTrees;
class FileManager : public QObject {
    Q_OBJECT

 public:
    static FileManager& getInstance() {
        // cppcheck-suppress threadsafety-threadsafety
        static FileManager instance;
        return instance;
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
    bool backupGameDir(Path path);

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
    const QString calculateMD5(Path path);

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
    int checkFileInfo(Path path);

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
    int cleanWorkingDir(Path path);

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
    int copyFile(Path from, Path to);

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
    int createDirectory(Path path);

    /**
     * @brief Extracts the contents of a ZIP archive into a specified output folder.
     *
     * This function unzips a given archive file from the level directory and extracts
     * its contents to a specified output folder. It ensures that the output directory
     * exists and handles errors related to file extraction.
     *
     * @param ZipData zip archive to extract.
     * @return `true` if extraction is successful, otherwise `false`.
     *
     * @note This function uses `miniz` for ZIP operations.
     * @warning If extraction fails at any point, the function will attempt to clean up
     *          and terminate extraction, potentially leaving incomplete files.
     * @signal fileWorkTickSignal() is emitted to indicate extraction progress.
     */
    bool extractZip(ZipData zipData);

    /**
     * @brief Determines an additional path to the executable within a level directory.
     *
     * This function adds to a path on a level directory by searching for an
     * extra executable path by comparing the directory structure with predefined
     * static trees.
     *
     * @param Path to a lid.TRLE level directory.
     * @param quint64 game type, Tomb Raider 1, 2, 3, 4, 5, 6.
     * @return bool status if a match is found.
     */
    bool getExtraPathToExe(Path &path, quint64 type);

    /**
     * @brief Creates a symbolic link from a Path to another.
     *
     * If a link already exists, it is removed and recreated.
     *
     * @param Path from The target path can be a file or directory.
     * @param Path to The file link we create, can't be a directory.
     * @return True if the symbolic link is successfully created, false otherwise.
     */
    bool linkPaths(Path from, Path to);

    /**
     * @brief Creates a case sensitive symbolic link to executable.
     *
     * The link is absolute full path and relative in position to the found executable.
     *
     * @param Path Level directory, lid.TRLE path.
     * @param quint64 The type of level is a number from 1-6.
     */
    void linkToExe(Path level, quint64 type);

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
    qint64 removeFileOrDirectory(Path path);

    void addLevelDir(Path& path, quint64 id);
    QStringList getSaveFiles(Path& path);

 signals:
    void fileWorkTickSignal();
    void fileWorkErrorSignal(int status);

 private:
    FileManager() :
        m_sep(QDir::separator())
    {}
    bool isHomePath(const QString& path) {
        return path.startsWith(QDir::homePath());
    }

    const QString m_sep;
    Q_DISABLE_COPY(FileManager)
};

#endif  // SRC_FILEMANAGER_HPP_
