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

#include "binary.hpp"
#include <iostream>
#include <memory>
#include <string>
#include <LIEF/LIEF.hpp>

void analyzeImportTable(const std::string& binaryPath) {
    try {
        auto binary = LIEF::PE::Parser::parse(binaryPath);
        for (const auto& imp : binary->imports()) {
            std::cout << "DLL Name: " << imp.name() << std::endl;
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}

void readPEHeader(const QString &filePath) {
    QFile file(filePath);
    if (file.open(QIODevice::ReadOnly) == true) {
        QDataStream in(&file);
        in.setByteOrder(QDataStream::LittleEndian);

        // Read DOS Header
        DosHeader dosHeader;
        in.readRawData(reinterpret_cast<char*>(&dosHeader), sizeof(DosHeader));

        if (strncmp(dosHeader.magic.data(), "MZ", 2) == 0) {
            // Seek to PE Header
            file.seek(dosHeader.e_lfanew);

            // Read PE Header
            PEHeader peHeader;
            in.readRawData(
                reinterpret_cast<char*>(&peHeader), sizeof(PEHeader));

            if (strncmp(peHeader.signature.data(), "PE\0\0", 4) == 0) {
                // Print PE Header Information
                qDebug() << "Machine:" << QString("0x%1")
                    .arg(peHeader.machine, 4, 16, QLatin1Char('0')).toUpper();
                qDebug() << "Number of Sections:" << peHeader.numSections;
                qDebug() << "Timestamp:" << peHeader.timeDateStamp;
                qDebug() << "Size of Optional Header:"
                    << peHeader.sizeOfOptionalHeader;
                qDebug() << "Characteristics:"
                    << QString("0x%1")
                        .arg(peHeader.characteristics,
                            4, 16, QLatin1Char('0')).toUpper();
            } else {
                qCritical() << "Not a valid PE header (missing PE signature)";
            }
        } else {
            qCritical() << "Not a valid PE file (missing MZ signature)";
        }
        file.close();
    } else {
        qCritical() << "Failed to open file:" << filePath;
    }
}


void readExportTable(const QString &filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qCritical() << "Failed to open file:" << filePath;
        return;
    }

    QDataStream in(&file);
    in.setByteOrder(QDataStream::LittleEndian);

    // Read DOS Header
    DosHeader dosHeader;
    in.readRawData(reinterpret_cast<char*>(&dosHeader), sizeof(DosHeader));

    if (strncmp(dosHeader.magic.data(), "MZ", 2) != 0) {
        qCritical() << "Not a valid PE file (missing MZ signature)";
        return;
    }

    // Seek to PE Header
    file.seek(dosHeader.e_lfanew);
    PEHeader peHeader;
    in.readRawData(reinterpret_cast<char*>(&peHeader), sizeof(PEHeader));

    if (strncmp(peHeader.signature.data(), "PE\0\0", 4) != 0) {
        qCritical() << "Not a valid PE header (missing PE signature)";
        return;
    }

    // Seek to Optional Header (immediately after PEHeader)
    file.seek(dosHeader.e_lfanew + sizeof(PEHeader));

    // Read Optional Header
    QByteArray optionalHeaderData(peHeader.sizeOfOptionalHeader, Qt::Uninitialized);
    in.readRawData(optionalHeaderData.data(), peHeader.sizeOfOptionalHeader);

    // Get Data Directory
    struct DataDirectory {
        quint32 virtualAddress;
        quint32 size;
    };

    constexpr int IMAGE_DIRECTORY_ENTRY_EXPORT = 0;
    auto dataDirectory = reinterpret_cast<DataDirectory*>(optionalHeaderData.data() + 96); // Offset 96: start of Data Directory
    quint32 exportTableRVA = dataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].virtualAddress;

    if (exportTableRVA == 0) {
        qCritical() << "No Export Table found in this PE file.";
        return;
    }

    // Find the section that contains the Export Table
    file.seek(dosHeader.e_lfanew + sizeof(PEHeader) + peHeader.sizeOfOptionalHeader);
    quint16 numSections = peHeader.numSections;

    struct SectionHeader {
        char name[8];
        quint32 virtualSize;
        quint32 virtualAddress;
        quint32 sizeOfRawData;
        quint32 pointerToRawData;
        quint32 pointerToRelocations;
        quint32 pointerToLinenumbers;
        quint16 numRelocations;
        quint16 numLinenumbers;
        quint32 characteristics;
    };

    SectionHeader section;
    quint32 exportTableFileOffset = 0;

    for (quint16 i = 0; i < numSections; i++) {
        in.readRawData(reinterpret_cast<char*>(&section), sizeof(SectionHeader));
        quint32 sectionStart = section.virtualAddress;
        quint32 sectionEnd = sectionStart + section.virtualSize;

        if (exportTableRVA >= sectionStart && exportTableRVA < sectionEnd) {
            quint32 offsetWithinSection = exportTableRVA - sectionStart;
            exportTableFileOffset = section.pointerToRawData + offsetWithinSection;
            break;
        }
    }

    if (exportTableFileOffset == 0) {
        qCritical() << "Export Table not found in any section.";
        return;
    }

    // Read Export Directory
    file.seek(exportTableFileOffset);
    ExportDirectory exportDirectory;
    in.readRawData(reinterpret_cast<char*>(&exportDirectory), sizeof(ExportDirectory));

    // Read the DLL Name
    file.seek(section.pointerToRawData + (exportDirectory.nameRVA - section.virtualAddress));
    QByteArray dllName;
    char c;
    while (file.getChar(&c) && c != '\0') {
        dllName.append(c);
    }
    qDebug() << "DLL Name:" << dllName;

    // Read Exported Function Names
    if (exportDirectory.numNamePointers > 0) {
        qDebug() << "Exported Functions:";

        // Name Pointer Table
        quint32 namePointerOffset = section.pointerToRawData + (exportDirectory.namePointerRVA - section.virtualAddress);
        file.seek(namePointerOffset);

        QVector<quint32> nameRVAs(exportDirectory.numNamePointers);
        for (quint32 &nameRVA : nameRVAs) {
            in >> nameRVA;
        }

        for (const quint32 &nameRVA : nameRVAs) {
            quint32 nameOffset = section.pointerToRawData + (nameRVA - section.virtualAddress);
            file.seek(nameOffset);

            QByteArray functionName;
            while (file.getChar(&c) && c != '\0') {
                functionName.append(c);
            }
            qDebug() << functionName;
        }
    } else {
        qDebug() << "No exported functions.";
    }

    file.close();
}

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
