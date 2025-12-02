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

#ifndef SRC_GAMEFILETREEDATA_HPP_
#define SRC_GAMEFILETREEDATA_HPP_

#include "../src/GameFileTree.hpp"
#include <array>
#include <QDir>
#include <QStringList>
#include <QVector>

struct StaticTrees {
    StaticTrees()
        : data{{
            {},  // [0] Null
            QVector<GameFileTree*>{  // [1] TR1
                new GameFileTree(QStringList{
                    "CFG",
                    "DATA",
                    "SHADERS",
                }),
                new GameFileTree(QStringList{
                    QString("TOMBRAID"),
                }),
                new GameFileTree(QStringList{
                    "CFG",
                    "DATA",
                    "SHADERS"
                }),
            },
            QVector<GameFileTree*>{  // [2] TR2
                new GameFileTree(QStringList{
                    "DATA",
                }),
            },
            QVector<GameFileTree*>{  // [3] TR3
                new GameFileTree(QStringList{
                    "AUDIO",
                    "DATA",
                }),
            },
            QVector<GameFileTree*>{  // [4] TR4
                new GameFileTree(QStringList{
                    "AUDIO",
                    "DATA",
                }),
            },
            QVector<GameFileTree*>{  // [5] TR5
                new GameFileTree(QStringList{
                    "AUDIO",
                    "DATA",
                    "PIX",
                }),
                new GameFileTree(QStringList{
                    "DATA",
                }),
            },
            QVector<GameFileTree*>{  // [6] TEN
                new GameFileTree(QStringList{
                    "TOMBENGINE.EXE",
                }),
            },
        }}
    {}
    ~StaticTrees() {
        for (auto &vec : data)
            for (auto ptr : vec)
                delete ptr;
    }
    const std::array<QVector<GameFileTree*>, 7> data;
};

inline StaticTrees staticTrees;

#endif  // SRC_GAMEFILETREEDATA_HPP_
