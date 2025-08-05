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
    : m_fileName(fileName), m_parentItem(parent)
{}

GameFileTree::GameFileTree(const QStringList& pathList)
    : m_parentItem(nullptr) {
    addPathList(pathList);
}

GameFileTree::GameFileTree(Path dirPath)
    : m_parentItem(nullptr) {
    if (dirPath.exists() == true && dirPath.isDir()) {
        QStringList pathList;
        QQueue<QString> dirQueue;
        const QString base = dirPath.get();  // Inside home and absolute
        const QDir baseDir(base);
        dirQueue.enqueue(base);

        while (!dirQueue.isEmpty()) {
            QString currentDirPath = dirQueue.dequeue();
            QDir currentDir(currentDirPath);

            QFileInfoList fileList = currentDir.entryInfoList(
                QDir::Files |
                QDir::Dirs |
                QDir::NoDotAndDotDot |
                QDir::Hidden,
                QDir::Name);

            for (const QFileInfo& fileInfo : fileList) {
                pathList << baseDir
                        .relativeFilePath(fileInfo.absoluteFilePath());

                if (fileInfo.isDir() == true) {
                    dirQueue.enqueue(fileInfo.absoluteFilePath());
                }
            }
        }
        addPathList(pathList);
    } else {
        QTextStream(stdout) << "Directory does not exist: "
            << dirPath.get() << Qt::endl;
    }
}

GameFileTree::~GameFileTree() {
    for (GameFileTree* child : m_childItems) {
        delete child;
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
                [&component](const GameFileTree* child) {
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
    bool status = true;

    if (other == nullptr) {
        status = false;
    }

    // Check if first node name match
    if (status && (m_fileName.toUpper() != other->m_fileName.toUpper())) {
        status = false;
    }

    /* Check other recursively from 'this'
     * It matches the branches like an Enarmad bandit machine
     * throught all child nodes at this point. This builds up the
     * matching "tower" on the stack by trying all Lego pieces
     * available at this node until ONE Lego is not found
     * it cancels the branch.
     */
    if (status) {
        for (const GameFileTree* childOther : other->m_childItems) {
            bool matched = std::any_of(
                m_childItems.begin(), m_childItems.end(),
                [childOther](const GameFileTree* childThis) {
                    return childThis->matchesSubtree(childOther);
                });

            if (matched == false) {
                status = false;
                break;
            }
        }
    }

    return status;
}

QStringList GameFileTree::matchesFromAnyNode(const GameFileTree* other) {
    QStringList result;

    if (matchesSubtree(other)) {
        result.append(m_fileName);
    } else {
        for (GameFileTree* child : m_childItems) {
            QStringList childPath = child->matchesFromAnyNode(other);
            if (!childPath.isEmpty()) {
                childPath.prepend(m_fileName);  // its a bit backwards later
                result = childPath;
                break;
            }
        }
    }

    return result;
}
