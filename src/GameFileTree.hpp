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

#ifndef SRC_GAMEFILETREE_HPP_
#define SRC_GAMEFILETREE_HPP_

#include <QString>
#include <QVector>
#include <QDir>
#include <QStringList>
#include <QDebug>

class GameFileTree {
 public:
    explicit GameFileTree(const QDir& fullPath);
    explicit GameFileTree(const QString& fileList);
    ~GameFileTree();

    void printTree(int level) const;

 private:
    explicit GameFileTree(
        const QString &fileName, GameFileTree *parentItem);
    void addPathList(const QStringList& pathList);
    void addNode(GameFileTree* node);  // maybe dont need this
    QVector<GameFileTree*> m_childItems;
    QString m_fileName;
    GameFileTree *m_parentItem;
};

#endif  // SRC_GAMEFILETREE_HPP_
