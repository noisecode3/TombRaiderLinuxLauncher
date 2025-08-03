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
#include "../src/Path.hpp"
#include "../src/settings.hpp"

Path::Path(Root root) {
    if (root == programFiles) {
        m_path.setFile(m_programFilesPath.path());
    } else if (root == resource) {
        m_path.setFile(m_resourcePath.path());
    }
}

bool Path::validate(const QFileInfo& fileInfo) {
    bool status = true;

    if (!fileInfo.isAbsolute()) {
        qDebug() << "Path: Validate faild is not absolute path!";
        status = false;
    }

    if (status == true) {
        const QString clean = QDir::cleanPath(fileInfo.absoluteFilePath());
        status = inHome(clean);
    }

    return status;
}

bool Path::setProgramFilesPath() {
    bool status = false;
    const QFileInfo fileInfo(
            getSettingsInstance().value("gamePath").toString());
    if (validate(fileInfo)) {
        status = true;
        m_programFilesPath = QDir(fileInfo.absoluteFilePath());
    }

    qDebug() << "setProgramFilesPath :" << m_programFilesPath;
    return status;
}

bool Path::setResourcePath() {
    bool status = false;
    const QFileInfo fileInfo(
            getSettingsInstance().value("levelPath").toString());
    if (validate(fileInfo)) {
        status = true;
        m_resourcePath = QDir(fileInfo.absoluteFilePath());
    }

    qDebug() << "setProgramFilesPath :" << m_resourcePath;
    return status;
}

QString Path::get() {
    return QDir::cleanPath(m_path.absoluteFilePath());
}

QString Path::getDir() {
    return QDir::cleanPath(m_path.absolutePath());
}

bool Path::inHome(QString absoluteFilePath) {
    bool status = true;
    const QString clean = QDir::cleanPath(absoluteFilePath);
    if (!clean.startsWith(QDir::homePath())) {
        qDebug() << "Path: Validation faild Path don't start with "
                                                << QDir::homePath();
        qDebug() << "Path: absoluteFilePath " << absoluteFilePath;
        qDebug() << "Path: clean " << clean;
        status = false;
    }

    return status;
}

bool Path::exists() {
    return m_path.exists();
}

bool Path::isFile() {
    return m_path.isFile();
}

bool Path::isDir() {
    return m_path.isDir();
}

bool Path::isSymLink() {
    return m_path.isSymLink();
}

bool Path::validateSymLink() {
    bool status = false;
    if (m_path.isSymLink()) {
        const QFileInfo target(m_path.symLinkTarget());
        if (validate(target)) {
            status = true;
        }
    }

    return status;
}
