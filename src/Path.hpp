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
#ifndef SRC_PATH_HPP_
#define SRC_PATH_HPP_

#include <QDebug>
#include <QString>
#include <QDir>
#include <QFile>

/**
 * @class Path
 * @brief Manage path string operations.
 */
class Path {
 public:
    enum Root {
        programFiles,
        resource,
    };
    explicit Path(Root root);

    /**
     * @brief Sets the Program Files launch directory.
     *
     * - Installed game files, the classic "Program Files (x86)".
     * - This function ensures that all file operations are contained
     * Within the set directory inside the home filer of the user.
     *
     * @return `true` if pass validation, `false` otherwise.
     */
    static bool setProgramFilesPath();

    /**
     * @brief Sets the Resource directory.
     *
     * - Downloaded program files and index database.
     * - This function ensures that all file operations are contained
     * Within the set directory inside the home filer of the user.
     *
     * @return `true` if pass validation, `false` otherwise.
     */
    static bool setResourcePath();
    QString get();
    bool exists();
    bool isFile();
    bool isDir();

    Path& operator<<(const QString& subdir) {
        const QString s = QString("%1%2%3").arg(m_path.filePath(), m_sep, subdir);
        m_path.setFile(s);
        return *this;
    }

 private:
    static bool validate(const QDir& dir);
    static QString getCleanCanonical(const  QDir& dir);

    QFileInfo m_path;
    inline static QDir m_programFilesPath;
    inline static QDir m_resourcePath;
    inline static QChar m_sep = QDir::separator();
};

#endif  // SRC_PATH_HPP_
