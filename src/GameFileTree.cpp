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
#include <QStack>
#include <QPair>

GameFileTree::GameFileTree(
        const QString &fileName, GameFileTree *parent)
    : m_fileName(fileName), m_parentItem(parent) {
    if (parent != nullptr) {
        parent->m_childNames.insert(fileName);
    }
}

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

bool GameFileTree::matcheTrees(
            const GameFileTree* subTree,
            const GameFileTree* other) const {
    bool status = true;
    QStack<QPair<const GameFileTree*, const GameFileTree*>> stack;
    stack.push(qMakePair(subTree, other));

    while (!stack.isEmpty() || (status == true)) {
        QPair<const GameFileTree*, const GameFileTree*> pair = stack.pop();
        const GameFileTree* subTreeCurrent = pair.first;
        const GameFileTree* otherCurrent = pair.second;


        //  QSet<T> &QSet::intersect(const QSet<T> &other)
            //  Removes all items from this set that are not
            //  contained in the other set. A reference to this set is returned.

        // Push child pairs in reverse order (for left-to-right DFS)
        for (int i = childrenA.size() - 1; i >= 0; --i) {
            stack.push(qMakePair(childrenA[i], childrenB[i]));
        }
    }


    if (other == nullptr) {
        status = false;
    }

    // Check if first node name match
    // if we have an empty fileName we are at root
    if (status && (subTree->m_fileName.isEmpty() || other->m_fileName.isEmpty())) {
        if ((subTree->m_fileName.toUpper() != other->m_fileName.toUpper())) {
            status = false;
        }
    }
    // --------------------------------------------------

    if (status) {
        for (const GameFileTree* childOther : other->m_childItems) {
            bool matched = std::any_of(
                subTree->m_childItems.begin(), subTree->m_childItems.end(),
                [childOther](const GameFileTree* childThis) {
                    return childThis->matcheTrees(childOther, );
                });

            if (matched == false) {
                status = false;
                break;
            }
        }
    }

    return status;
}

QStringList GameFileTree::matchesFromAnyNode(const GameFileTree* other) const {
    QStringList result;
    QQueue<const GameFileTree*> directoryNodes;
    directoryNodes.enqueue(this);

    while (!directoryNodes.isEmpty()) {
        const GameFileTree* currentNode = directoryNodes.dequeue();
        if (matcheTrees(currentNode, other)) {
            result.append(currentNode->m_fileName);
        }
        for (const GameFileTree* childNode : currentNode->m_childItems) {
            if (!childNode->m_childItems.isEmpty()) {
                directoryNodes.enqueue(childNode);
            }
        }
    }

    return result;
}
