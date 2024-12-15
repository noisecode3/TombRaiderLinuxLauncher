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

#ifndef SRC_BINARY_HPP_
#define SRC_BINARY_HPP_

#include <QString>
#include <QFile>
#include <QByteArray>
#include <QDebug>

/**
 * @brief Look for a bit pattern that set 16:9 aspect ratio for Tomb Raider 4.
 * @param[in] Open file object reference.
 * @retval 0 Success.
 * @retval 1 Pattern was not found in the file.
 * @retval 2 Could not open the file.
 * @retval 3 Could not write to the file.
 * @return error qint64.
 */
qint64 findReplacePattern(QFile* const file) {
    qint64 status = 0;
    QByteArray fileContent = file->readAll();
    file->close();

    // Define the pattern to find and replace
    QByteArray pattern = QByteArray::fromHex("abaaaa3f0ad7a33b");
    QByteArray replacement = QByteArray::fromHex("398ee33f0ad7a33b");

    // Find the pattern in the file content
    int index = fileContent.indexOf(pattern);
    if (index != -1) {
        // Replace the pattern
        fileContent.replace(index, pattern.size(), replacement);

        // Reopen the file for writing
        if (!file->open(QIODevice::WriteOnly)) {  // flawfinder: ignore
            qCritical() << "Error opening file for writing!";
            status = 2;
        } else if (file->write(fileContent) == -1) {
            qCritical() << "Error writing to file!";
            status = 3;
        } else {
            qDebug() << "Widescreen patch applied successfully!";
        }
    } else {
        qDebug() << "Pattern not found in the file.";
        status = 1;
    }
    file->close();
    return status;
}

/**
 * @brief Widescreen in binary set function.
 * @param[in] File path to windows exe.
 * @retval 0 Success.
 * @retval 1 Path was not an safe file regular file.
 * @retval 2 Could not preform the first read only opening of the file.
 * @return error qint64.
 */
qint64 widescreen_set(const QString& path) {
    qint64 status = 0;
    QFileInfo fileInfo(path);
    QFile file(path);

    // Open the file
    if (!fileInfo.exists() || !fileInfo.isFile()) {
        qCritical() << "Error: The exe path is not a regular file: " << path;
        status = 1;  // Invalid file path
    } else if (!file.open(QIODevice::ReadOnly)) {  // flawfinder: ignore
        qCritical() << "Error opening file for reading!";
        status = 2;  // File open error
    } else {
        // Perform the pattern replacement and propagate the status
        status = findReplacePattern(&file);
    }
    return status;
}

#endif  // SRC_BINARY_HPP_
