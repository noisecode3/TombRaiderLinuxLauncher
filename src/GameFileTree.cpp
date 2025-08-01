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

/* An GameFileTree object can contain
 * one tree of game files and can match another whole tree or branch.
 * It can print out the tree or a specific branch by index.
 * This object it used to recognise game files from TRLE's or
 * any distribution of the original Tomb Raider Games
 * a tree pattern can be matched from "root or branch"
 * This mean we have default tree objects to match with
 * like how we know if there is an exe file and ./data + ./audio
 * from there we can link to the correct game directory for the
 * launcher. We also do binary recognition for the Tomb Raider exe
 * and maybe other files. We compare/match and verify game files here.
 */

#include "../src/GameFileTree.hpp"
#include <QDir>
#include <QFileInfo>
#include <QQueue>
#include <QTextStream>

GameFileTree::GameFileTree(
        const QString &fileName, GameFileTree *parent)
    // cppcheck-suppress misra-c2012-12.3
    : m_fileName(fileName), m_parentItem(parent)
{}

GameFileTree::~GameFileTree() {
    for (GameFileTree* child : m_childItems) {
        delete child;
    }
}

GameFileTree::GameFileTree(const QStringList& pathList)
    // cppcheck-suppress misra-c2012-12.3
    : m_parentItem(nullptr) {
    addPathList(pathList);
}

GameFileTree::GameFileTree(const QDir &dir)
    // cppcheck-suppress misra-c2012-12.3
    : m_parentItem(nullptr) {
    if (dir.exists() == true) {
        QStringList pathList;
        // Skapa en kö för att hantera kataloger iterativt
        QQueue<QString> dirQueue;
        const QString base = dir.absolutePath();
        dirQueue.enqueue(base);

        while (!dirQueue.isEmpty()) {
            QString currentDirPath = dirQueue.dequeue();
            QDir currentDir(currentDirPath);

            // Hämta alla filer och mappar i den aktuella katalogen
            QFileInfoList fileList = currentDir.entryInfoList(
                QDir::Files |
                QDir::Dirs |
                QDir::NoDotAndDotDot |
                QDir::Hidden,
                QDir::Name);

            for (const QFileInfo& fileInfo : fileList) {
                pathList << QString(fileInfo.absoluteFilePath().split(base)[1]);

                // Om objektet är en katalog, lägg till den i kön
                if (fileInfo.isDir() == true) {
                    dirQueue.enqueue(fileInfo.absoluteFilePath());
                }
            }
        }
        addPathList(pathList);
    } else {
        QTextStream(stdout) << "Directory does not exist: "
            << dir.absolutePath() << Qt::endl;
    }
}

void GameFileTree::addPathList(const QStringList& pathList) {
    for (const QString& path : pathList) {
        QStringList components = QDir::toNativeSeparators(path).split(
            QDir::separator(), Qt::SkipEmptyParts);
        GameFileTree* current = this;

        for (const QString& component : components) {
            // Check if a child node with this name already exists
            auto it = std::find_if(
                current->m_childItems.begin(), current->m_childItems.end(),
                [&component](GameFileTree* child) {
                    return child->m_fileName == component;
            });

            if (it == current->m_childItems.end()) {
                // Node doesn't exist, create it
                GameFileTree* newNode = new GameFileTree(component, current);
                current->m_childItems.append(newNode);
                current = newNode;
            } else {
                // Move to the existing node
                current = *it;
            }
        }
    }
}


void GameFileTree::printTree(int level) const {
    // Build the current node's path by traversing up to the root
    QString fullPath;
    const GameFileTree* currentNode = this;

    while (currentNode != nullptr) {
        if (!currentNode->m_fileName.isEmpty()) {
            if (fullPath.isEmpty()) {
                fullPath = currentNode->m_fileName;
            } else {
                fullPath = QString("%1%2%3")
                    .arg(currentNode->m_fileName)
                    .arg(QDir::separator())
                    .arg(fullPath);
            }
        }
        currentNode = currentNode->m_parentItem;
    }

    // Print the full path for this node
    QTextStream(stdout) << fullPath << Qt::endl;

    // Recursively call printTree on all child nodes
    for (const GameFileTree* child : m_childItems) {
        if (child != nullptr) {
            child->printTree(level + 1);
        }
    }
}

bool GameFileTree::matchesSubtree(const GameFileTree* other) const {
    if (!other) return false;  // If the other tree is null, no match is possible.

    QQueue<QPair<const GameFileTree*, const GameFileTree*>> queue;
    queue.enqueue({this, other});  // Start with the root nodes of both trees.

    while (!queue.isEmpty()) {
        QPair<const GameFileTree*, const GameFileTree*> current = queue.dequeue();

        const GameFileTree* nodeThis = current.first;
        const GameFileTree* nodeOther = current.second;

        // Check if all children in `nodeOther` match at least one child in `nodeThis`.
        for (const GameFileTree* childOther : nodeOther->m_childItems) {
            bool found = false;
            for (const GameFileTree* childThis : nodeThis->m_childItems) {
                if (childThis->m_fileName.toUpper() == childOther->m_fileName.toUpper()) {
                    // Enqueue the matching child nodes for further comparison.
                    queue.enqueue({childThis, childOther});
                    found = true;
                    break;
                }
            }

            // If no matching child is found, the subtree does not match.
            if (!found) {
                return false;
            }
        }
    }

    return true;  // All nodes and structures matched successfully.
}

QString GameFileTree::matchesFromAnyNode(const GameFileTree* other) {
    // If this node matches the subtree, return its name
    if (matchesSubtree(other)) {
        return m_fileName;
    }

    // Otherwise, check children recursively
    for (GameFileTree* child : m_childItems) {
        QString childPath = child->matchesFromAnyNode(other);
        if (!childPath.isEmpty()) {
            // Build the path backwards
            return QString("%1%2%3")
                .arg(m_fileName)
                .arg(QDir::separator())
                .arg(childPath);
        }
    }

    // No match found
    return QString("\0");
}

