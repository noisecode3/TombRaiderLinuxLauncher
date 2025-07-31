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

bool Path::validate(const QDir& dir) {
    bool status = true;

    if (!dir.exists()) {
        qDebug() << "Path: Validate faild does not exist!";
        status = false;
    }

    if (!dir.isAbsolute()) {
        qDebug() << "Path: Validate faild is not absolute path!";
        status = false;
    }

#ifdef Q_OS_WIN
    const QString home = QString("C:%1Users%1").arg(m_sep);
#else
    const QString home = QString("%1home%1").arg(m_sep);
#endif
    const QString cleanCanonical = getCleanCanonical(dir);
    if (!cleanCanonical.startsWith(home)) {
        qDebug() << "Path: Validation faild"
            << " canonicalFilePath don't start with " << home;
        status = false;
    }

    return status;
}

bool Path::setProgramFilesPath() {
    bool status = false;
    const QDir dir(getSettingsInstance().value("gamePath").toString());
    qDebug() << "setProgramFilesPath :" << dir;
    if (validate(dir)) {
        status = true;
        m_programFilesPath = dir;
    }
    return status;
}

bool Path::setResourcePath() {
    bool status = false;
    const QDir dir(getSettingsInstance().value("levelPath").toString());
    qDebug() << "setProgramFilesPath :" << dir;
    if (validate(dir)) {
        status = true;
        m_resourcePath = dir;
    }
    return status;
}

QString Path::get() {
    QString result;

    if (m_path.isDir()) {
        QDir dir(m_path.path());
        if (validate(dir)) {
            result = getCleanCanonical(dir);
        }
    } else if (m_path.isFile()) {
        QDir dir(m_path.absoluteDir());
        if (validate(dir)) {
            result = getCleanCanonical(dir) + m_sep + m_path.fileName();
        }
    }

    return result;
}

QString Path::getCleanCanonical(const QDir& dir) {
    const QString canonical = dir.canonicalPath();
    return canonical.isEmpty() ? QString() : QDir::cleanPath(canonical);
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
