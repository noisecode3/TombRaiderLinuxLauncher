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

#ifndef SRC_GAMEFILETREE_HPP_
#define SRC_GAMEFILETREE_HPP_

#include <QString>
#include <QVector>
#include <QDir>
#include <QStringList>
#include <QDebug>
#include <QList>
#include "../src/Path.hpp"

class GameFileTree {
 public:
    explicit GameFileTree(Path dirPath);
    explicit GameFileTree(const QStringList& pathList);
    ~GameFileTree();

    void printTree(int level) const;

    bool matcheTrees(const GameFileTree* subTree,
                     const GameFileTree* other) const;
    QStringList matchesFromAnyNode(const GameFileTree* other) const;

 private:
    explicit GameFileTree(
        const QString &fileName, GameFileTree *parentItem);
    void addPathList(const QStringList& pathList);
    const GameFileTree* findChildByName(const QString& name) const;
    QVector<GameFileTree*> m_childItems;
    QVector<GameFileTree*> m_childDirItems;
    QSet<const QString> m_childNames;
    QString m_fileName;
    GameFileTree *m_parentItem;
};

#endif  // SRC_GAMEFILETREE_HPP_
