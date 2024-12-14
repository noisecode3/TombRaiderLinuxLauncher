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

#ifndef SRC_GAMETREE_HPP_
#define SRC_GAMETREE_HPP_

#include <QDir>
#include <QFileInfo>
#include <QVector>
#include <QString>
#include <iostream>

// A struct to represent a file or directory
struct FileNode {
    QString name;
    bool isDirectory;
    QVector<FileNode> children;  // Only populated if it's a directory
};

// Recursive function to build the file tree
FileNode buildFileTree(const QDir& dir) {
    FileNode node;
    node.name = dir.dirName();  // Current directory name
    node.isDirectory = true;

    // List all entries in the directory (excluding "." and "..")
    QFileInfoList entries =
        dir.entryInfoList(QDir::AllEntries | QDir::NoDotAndDotDot, QDir::Name);

    for (const QFileInfo& entry : entries) {
        if (entry.isDir()) {
            // Recursively process subdirectories
            node.children.append(buildFileTree(QDir(entry.absoluteFilePath())));
        } else {
            // Add file as a child
            node.children.append({entry.fileName(), false, {}});
        }
    }

    return node;
}

// Function to print the tree (for debugging or visualization)
void printFileTree(const FileNode& node, int depth = 0) {
    for (int i = 0; i < depth; ++i) std::cout << "  ";
    std::cout << (node.isDirectory ? "[Dir] " : "[File] ")
        << node.name.toStdString() << "\n";

    for (const auto& child : node.children) {
        printFileTree(child, depth + 1);
    }
}

int test(const QString& path) {
    // Build the file tree
    FileNode fileTree = buildFileTree(path);

    // Print the file tree to the console
    printFileTree(fileTree);

    return 0;
}

#endif  // SRC_GAMETREE_HPP_
