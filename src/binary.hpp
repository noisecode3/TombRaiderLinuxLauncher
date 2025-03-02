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
#include <QFileInfo>
#include <QByteArray>
#include <QDebug>
#include <QDataStream>

// Define structures for PE headers
#pragma pack(push, 1)  // Set 1-byte alignment
struct DosHeader {
    std::array<char, 2> magic;          // Magic number ("MZ")
    quint16 lastPageBytes;
    quint16 totalPages;
    quint16 numRelocations;
    quint16 headerSizeInParagraphs;
    quint16 minExtraParagraphs;
    quint16 maxExtraParagraphs;
    quint16 initialSS;
    quint16 initialSP;
    quint16 checksum;
    quint16 initialIP;
    quint16 initialCS;
    quint16 relocationTableOffset;
    quint16 overlayNumber;
    quint8 reserved[8];
    quint16 oemIdentifier;
    quint16 oemInformation;
    quint8 reserved2[20];
    quint32 e_lfanew;                  // Offset to PE header
};

struct PEHeader {
    std::array<char, 4> signature;     // Signature ("PE\0\0")
    quint16 machine;
    quint16 numSections;
    quint32 timeDateStamp;
    quint32 pointerToSymbolTable;
    quint32 numberOfSymbols;
    quint16 sizeOfOptionalHeader;
    quint16 characteristics;
};

struct ExportDirectory {
    quint32 characteristics;
    quint32 timeDateStamp;
    quint16 majorVersion;
    quint16 minorVersion;
    quint32 nameRVA;          // RVA of the DLL name
    quint32 ordinalBase;      // Starting ordinal number
    quint32 addressTableRVA;  // RVA of the Export Address Table
    quint32 namePointerRVA;   // RVA of the array of names
    quint32 ordinalTableRVA;  // RVA of the Ordinal Table
    quint32 numExportAddresses;  // Number of entries in Export Address Table
    quint32 numNamePointers;    // Number of entries in Name Pointer Table
};

#pragma pack(pop)

QString decideExe(const QDir& dir);
void analyzeImportTable(const std::string& peFilePath);
void readPEHeader(const QString &filePath);
void readExportTable(const QString &filePath);
qint64 findReplacePattern(QFile* const file);
qint64 widescreen_set(const QString& path);

#endif  // SRC_BINARY_HPP_
