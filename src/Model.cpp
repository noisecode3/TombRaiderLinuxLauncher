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

#include "Model.hpp"

// Those lambda should be in another header file
// I hate this and it should be able to recognize both the directory
// when linking and the game exe to make a symbolic link to automatically
Model::Model(QObject *parent) : QObject(parent), checkCommonFilesIndex_m(1) {
    instructionManager.addInstruction(4, [this](int id) {
        qDebug() << "Perform Operation A";
        const QString s = "/"+QString::number(id) + ".TRLE";
        fileManager.makeRelativeLink(s, "/The Rescue.exe", "/tomb4.exe");
    });
    instructionManager.addInstruction(5, [this](int id) {
        qDebug() << "Perform Operation B";
        const QString s = "/"+QString::number(id) + ".TRLE";
        fileManager.makeRelativeLink(s, "/War of the Worlds.exe", "/tomb4.exe");
    });
    instructionManager.addInstruction(11, [this](int id) {
        qDebug() << "Perform Operation C";
        const QString s = QString::number(id) + ".TRLE/TRBiohazard";
        fileManager.moveFilesToParentDirectory(s, 1);
    });
    /*
    instructionManager.addInstruction(12, [this](int id) {
        qDebug() << "Perform Operation C";
        const QString s = QString::number(id) + ".TRLE/Delca-KittenAdventureDemo/Engine";
        fileManager.moveFilesToParentDirectory(s, 2);
    });
    */
}

Model::~Model() {}

bool Model::setup(const QString& level, const QString& game) {
    bool status = false;
    if (setDirectory(level, game)) {
        status = true;
        checkCommonFiles();
    }
    return status;
}

bool Model::setDirectory(const QString& level, const QString& game) {
    if (fileManager.setUpCamp(level, game) &&
        downloader.setUpCamp(level) &&
        data.initializeDatabase(level)
    )
        return true;
    else
        return false;
}

// true if there was new original game
// false if there was no new original games
bool Model::checkCommonFiles() {
    int index = checkCommonFilesIndex_m;
    assert(index >= 1 && index <= 5);
    for (int i = index; i <= 5; i++) {
        if (checkGameDirectory(i) == 2) {
            // checkCommonFilesIndex_m this is becouse it should start here
            // becouse we use a return, we should only use 1 return...
            checkCommonFilesIndex_m = i+1;
            emit askGameSignal(i);
            QCoreApplication::processEvents();
            return true;
        }
    }
    emit generateListSignal();
    QCoreApplication::processEvents();
    return false;
}

QString Model::getGameDirectory(int id) {
    struct FolderNames folder;
    switch (id) {
    case 1:
        return folder.TR1;
    case 2:
        return folder.TR2;
    case 3:
        return folder.TR3;
    case 4:
        return folder.TR4;
    case 5:
        return folder.TR5;
    default:
        qDebug() << "Id number:"  << id << " is not a game.";
        return "";
    }
}

int Model::checkGameDirectory(int id) {
    const QString s = getGameDirectory(id);
    if (s != "")
        return fileManager.checkFileInfo(s, true);
    return -1;
}

void Model::getList(QVector<ListItemData>* list) {
    *list = data.getListItems();
}

int Model::getItemState(int id) {
    if (id < 0) {
        return 1;
    } else if (id > 0) {
        QString map(QString::number(id) + ".TRLE");
        if (fileManager.checkDir(map, false))
            return 2;
        else
            return 0;
    }
    return -1;
}

bool Model::setLink(int id) {
    bool status = false;
    if (id < 0) {
        id = -id;
        const QString s = "/Original.TR" + QString::number(id);
        if (fileManager.checkDir(s, false ))
            status = fileManager.linkGameDir(s, getGameDirectory(id));
    } else if (id > 0) {
        const QString s = "/"+QString::number(id) + ".TRLE";
        const int t = data.getType(id);

        if (fileManager.checkDir(s, false ))
            status = fileManager.linkGameDir(s, getGameDirectory(t));
    }
    return status;
}

void Model::setupGame(int id) {
    QVector<FileList> list = data.getFileList(id);
    const size_t s = list.size();
    assert(s != 0);

    const QString levelPath = "/Original.TR" + QString::number(id) +"/";
    const QString gamePath = getGameDirectory(id) + "/";

    for (size_t i = 0; i < s; i++) {
        const QString levelFile = QString("%1%2").arg(levelPath, list[i].path);
        const QString gameFile = QString("%1%2").arg(gamePath, list[i].path);
        const QString calculated = fileManager.calculateMD5(gameFile, true);

        if (list[i].md5sum == calculated) {
            fileManager.copyFile(gameFile, levelFile,  true);
        } else {
            qDebug() << "Original file was modified, had" << list[i].md5sum
                     << " got " << calculated << " for file "
                     << list[i].path << Qt::endl;
            fileManager.cleanWorkingDir(levelPath);
            break;
        }
    }
    if (fileManager.backupGameDir(gamePath)) {
        // remove the ending '/' and instantly link to
        // the game directory link to new game directory
        const QString src = levelPath.chopped(1);
        const QString des = gamePath.chopped(1);
        if (!fileManager.linkGameDir(src, des)) {
            checkCommonFiles();  // TODO(noisecode3): Remove this
            return;
        }
    }
    checkCommonFiles();  // TODO(noisecode3): Remove this
}

bool Model::getGame(int id) {
    assert(id > 0);
    if (id) {
        int status = 0;
        ZipData zipData = data.getDownload(id);
        downloader.setUrl(zipData.url);
        downloader.setSaveFile(zipData.name);

        if (fileManager.checkFile(zipData.name, false)) {
            qDebug() << "File exists:" << zipData.name;
            const QString sum = fileManager.calculateMD5(zipData.name, false);
            if (sum != zipData.md5sum) {
                downloader.run();
                status = downloader.getStatus();
            } else {
                // send 50% signal here
                for (int i=0; i < 50; i++) {
                    emit this->modelTickSignal();
                    QCoreApplication::processEvents();
                }
            }
        } else {
            qWarning() << "File does not exist:" << zipData.name;
            downloader.run();
            status = downloader.getStatus();
        }
        if (status == 0) {
            /* when the problem about updateing sum is solved it should verify
             * the download md5sum can just change on trle so it should just
             * update it
            const QString sum = fileManager.calculateMD5(zipData.name, false);
            if (sum == zipData.md5sum) {
                const QString directory = QString::number(id)+".TRLE";
                fileManager.extractZip(zipData.name, directory);
                instructionManager.executeInstruction(id);
                return true;
            }
            */
            const QString directory = QString::number(id)+".TRLE";
            fileManager.extractZip(zipData.name, directory);
            instructionManager.executeInstruction(id);
            return true;
        }
    }
    return false;
}

const InfoData Model::getInfo(int id) {
    return data.getInfo(id);
}

const QString Model::getWalkthrough(int id) {
    return data.getWalkthrough(id);
}
