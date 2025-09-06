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
            QVector<GameFileTree>{  // [1] TR1
                GameFileTree(QStringList{
                    "TR1X.EXE",
                    "CFG",
                    "DATA",
                    "SHADERS",
                }),
                GameFileTree(QStringList{
                    QString("TOMBRAID%1TOMB.EXE").arg(QDir::separator()),
                    "DOSBOX.EXE"
                }),
                GameFileTree(QStringList{
                    "TOMB1MAIN.EXE",
                    "CFG",
                    "DATA",
                    "SHADERS"
                }),
            },
            QVector<GameFileTree>{  // [2] TR2
                GameFileTree(QStringList{
                    "TR2MAIN.JSON",
                    "DATA",
                }),
                GameFileTree(QStringList{
                    "TOMB2.EXE",
                    "DATA",
                }),
            },
            QVector<GameFileTree>{  // [3] TR3
                GameFileTree(QStringList{
                    "TOMB3_CONFIGTOOL.JSON",
                    "TOMB3.EXE",
                    "AUDIO",
                    "DATA",
                    "PIX",
                }),
                GameFileTree(QStringList{
                    "TOMB3.EXE",
                    "AUDIO",
                    "DATA",
                    "PIX",
                }),
                GameFileTree(QStringList{
                    "TOMB3.EXE",
                    "AUDIO",
                    "DATA",
                }),
                GameFileTree(QStringList{
                    "TOMB3.EXE",
                    "AUDIO",
                    "DATA",
                    "PIX",
                }),
            },
            QVector<GameFileTree>{  // [4] TR4
                GameFileTree(QStringList{
                    "TOMB4.EXE",
                    "AUDIO",
                    "DATA",
                }),
                GameFileTree(QStringList{
                    "TOMB4.EXE",
                    "AUDIO",
                    "DATA",
                }),
                GameFileTree(QStringList{
                    "TOMB4.EXE",
                    "AUDIO",
                    "DATA",
                    "PIX",
                }),
                GameFileTree(QStringList{
                    "AUDIO",
                    "DATA",
                    "PIX",
                }),
            },
            QVector<GameFileTree>{  // [5] TR5
                GameFileTree(QStringList{
                    "PCTOMB5.EXE",
                    "AUDIO",
                    "DATA",
                    "PIX",
                }),
                GameFileTree(QStringList{
                    "TOMBENGINE.EXE",
                }),
            },
            QVector<GameFileTree>{},  // [6] Placeholder (empty)
        }}
    {}
    const std::array<QVector<GameFileTree>, 7> data;
};


#endif  // SRC_GAMEFILETREEDATA_HPP_
