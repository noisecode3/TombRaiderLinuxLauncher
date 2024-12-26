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
Model::Model(QObject *parent) : QObject(parent) {
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
}

Model::~Model() {}

bool Model::setup(const QString& level, const QString& game) {
    bool status = false;
    if (setDirectory(level, game) == true) {
        status = true;
        emit generateListSignal(checkCommonFiles());
        QCoreApplication::processEvents();
    }
    return status;
}

bool Model::setDirectory(const QString& level, const QString& game) {
    bool status = false;
    if (fileManager.setUpCamp(level, game) &&
            downloader.setUpCamp(level) &&
            data.initializeDatabase(level)) {
        status = true;
    }
    return status;
}

const QList<int>& Model::checkCommonFiles() {
    m_availableGames.clear();
    for (int i = 1; i <= 5; i++) {
        int dirStatus = checkGameDirectory(i);
        if (dirStatus == 1) {  // symbolic link
            m_availableGames.append(i);
        } else if (dirStatus == 2) {  // directory
            m_availableGames.append(-i);
        }
    }
    return m_availableGames;
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
    case 10:
        return folder.TEN;
    default:
        qDebug() << "Id number:"  << id << " is not a game.";
        return "";
    }
}

int Model::checkGameDirectory(int id) {
    int status = -1;
    const QString s = getGameDirectory(id);
    qDebug() << s;
    if (s != "") {
        status = fileManager.checkFileInfo(s, true);
    }
    return status;
}

void Model::getList(QVector<ListItemData>* list) {
    *list = data.getListItems();
}

int Model::getItemState(int id) {
    int status = 0;
    if (id < 0) {
        status = 1;
    } else if (id > 0) {
        QString dir(QString::number(id) + ".TRLE");
        if (fileManager.checkDir(dir, false)) {
            status = 2;
        } else {
            status = 0;
        }
    } else {
        status = -1;
    }
    return status;
}

bool Model::setLink(int id) {
    bool status = false;
    if (id < 0) {  // we use original game id as negative number
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
            qDebug() << "Faild to create the link to the new game directory";
        }
    }
    for (int i=0; i < 100; i++) {
        emit this->modelTickSignal();
        QCoreApplication::processEvents();
    }
}

bool Model::getLevel(int id) {
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
