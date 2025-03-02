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

struct StaticTrees {
    QList<GameFileTree*> data;
    StaticTrees() {
        data.append(new GameFileTree(QStringList{
            "TOMBRAID/tomb.exe",
            "dosbox.exe",
        }));
        data.append(new GameFileTree(QStringList{
            "Tomb1Main.exe",
            "cfg",
            "data",
            "shaders",
        }));
        data.append(new GameFileTree(QStringList{
            "TR1X.exe",
            "cfg",
            "data",
            "shaders",
        }));
        data.append(new GameFileTree(QStringList{
            "TR2Main.json",
            "data",
        }));
        data.append(new GameFileTree(QStringList{
            "Tomb2.exe",
            "data",
        }));
        data.append(new GameFileTree(QStringList{
            "tomb3_ConfigTool.json",
            "tomb3.exe",
            "audio",
            "data",
            "pix",
        }));
        data.append(new GameFileTree(QStringList{
            "tomb3.exe",
            "audio",
            "data",
            "Pix",
        }));
        data.append(new GameFileTree(QStringList{
            "tomb3.exe",
            "audio",
            "Data",
        }));
        data.append(new GameFileTree(QStringList{
            "tomb3.exe",
            "audio",
            "data",
            "pix",
        }));
        data.append(new GameFileTree(QStringList{
            "tomb4.exe",
            "audio",
            "Data",
        }));
        data.append(new GameFileTree(QStringList{
            "tomb4.exe",
            "audio",
            "data",
            "pix",
        }));
        data.append(new GameFileTree(QStringList{
            "audio",
            "data",
            "pix",
        }));
        data.append(new GameFileTree(QStringList{
            "PCTomb5.exe",
            "audio",
            "data",
            "pix",
        }));
        data.append(new GameFileTree(QStringList{
            "TombEngine.exe",
        }));
    }
    ~StaticTrees() {
        for (GameFileTree* tree : data) {
            delete tree;
        }
        data.clear();
    }
};




#endif  // SRC_GAMEFILETREEDATA_HPP_
