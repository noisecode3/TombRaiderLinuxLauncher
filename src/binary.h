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

#ifndef SRC_BINARY_H_
#define SRC_BINARY_H_

#include <QString>
#include <QFile>
#include <QByteArray>
#include <QDebug>

int widescreen_set(const QString& path) {
    // Open the file for reading in binary mode
    QFileInfo fileInfo(path);
    if (fileInfo.exists() && !fileInfo.isFile()) {
        qDebug() << "Error: The exe path is not a regular file. " << path;
        return 1;
    }

    QFile file(path);

    if (!file.open(QIODevice::ReadOnly)) {  // flawfinder: ignore
        qCritical() << "Error opening file for reading!";
        return 1;
    }

    // Read the entire file into a QByteArray
    QByteArray fileContent = file.readAll();
    file.close();

    // Pattern to find and replace
    QByteArray pattern = QByteArray::fromHex("abaaaa3f0ad7a33b");
    QByteArray replacement = QByteArray::fromHex("398ee33f0ad7a33b");

    // Find the pattern in the file content
    int index = fileContent.indexOf(pattern);
    if (index != -1) {
        // Replace the pattern with the replacement
        fileContent.replace(index, pattern.size(), replacement);

        // Open the file for writing in binary mode
        if (!file.open(QIODevice::WriteOnly)) {  // flawfinder: ignore
            qCritical() << "Error opening file for writing!";
            return 1;
        }

        // Write the modified content back to the file
        if (file.write(fileContent) == -1) {
            qCritical() << "Error writing to file!";
            return 1;
        }

        file.close();
        qDebug() << "Widescreen patch applied successfully!";
        return 0;
    } else {
        qDebug() << "Pattern not found in the file.";
        return 1;
    }
}

#endif  // SRC_BINARY_H_
