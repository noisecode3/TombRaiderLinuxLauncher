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
Model::Model() {
    instructionManager.addInstruction(4, [this](int id) {
        qDebug() << "Perform Operation A";
        const QString s = QString("/%1.TRLE").arg(id);
        fileManager.makeRelativeLink(s, "/The Rescue.exe", "/tomb4.exe");
    });
    instructionManager.addInstruction(5, [this](int id) {
        qDebug() << "Perform Operation B";
        const QString s = QString("/%1.TRLE").arg(id);
        fileManager.makeRelativeLink(s, "/War of the Worlds.exe", "/tomb4.exe");
    });
    instructionManager.addInstruction(11, [this](int id) {
        qDebug() << "Perform Operation C";
        const QString s = QString("%1.TRLE/TRBiohazard").arg(id);
        fileManager.moveFilesToParentDirectory(s, 1);
    });
}

Model::~Model() {}

void Model::setup(const QString& level, const QString& game) {
    if (setDirectory(level, game) == true) {
        QByteArray commonFiles(8, '\0');
        checkCommonFiles(&commonFiles);
        m_availableGames.clear();
        for (int i = 1; i <= 5; i++) {
            qint8 dirStatus = commonFiles[i];
            if (dirStatus == 1) {
                if (fileManager.checkDir(
                    QString("/Original.TR%1").arg(i), false) ==  true) {
                    m_availableGames.append(i);
                }
            } else if (dirStatus == 2) {
                m_availableGames.append(-i);
                // check if there is one of
                // TOMBRAID/tomb.exe
                // Tomb2.exe
                // tomb3.exe
                // tomb4.exe
                // PCTomb5.exe
            } else {
                if (fileManager.checkDir(
                    QString("/Original.TR%1").arg(i), false) ==  true) {
                    m_availableGames.append(i);
                }
            }
        }
        emit generateListSignal(m_availableGames);
        QCoreApplication::processEvents();
    } else {
        // send signal to gui with error about setup fail
        qDebug() << "setDirectory setup failed";
    }
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

void Model::checkCommonFiles(QByteArray* games) {
    for (int i = 1; i <= 5; i++) {
        int dirStatus = checkGameDirectory(i);
        if (dirStatus == 1) {  // symbolic link
            m_availableGames.append(1);
        } else if (dirStatus == 2) {  // directory
            m_availableGames.append(2);
        } else if (dirStatus == 3) {
            m_availableGames.append(3);
            qDebug() << "File could be a broken link or a regular file";
        } else {
            m_availableGames.append(0);
            qDebug() << "File don't exist";
        }
    }
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
        QString dir = QString("%1.TRLE").arg(id);
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
        const QString s = QString("/Original.TR%1").arg(id);
        if (fileManager.checkDir(s, false ))
            status = fileManager.linkGameDir(s, getGameDirectory(id));
    } else if (id > 0) {
        const QString s = QString("/%1.TRLE").arg(id);
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

    const QString levelPath = QString("/Original.TR%1/").arg(id);
    const QString gamePath = QString("%1/").arg(getGameDirectory(id));

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

bool Model::unpackLevel(const int id, const QString& name) {
    bool status = false;
    const QString directory = QString("%1.TRLE").arg(id);
    if (fileManager.extractZip(name, directory) == true) {
        instructionManager.executeInstruction(id);
        status = true;
    }
    return status;
}

bool Model::getLevelHaveFile(
        const int id, const QString& md5sum, const QString& name) {
    bool status = false;
    if (md5sum != "") {
        const QString existingFilesum = fileManager.calculateMD5(name, false);
        if (existingFilesum != md5sum) {
            downloader.run();
            if (downloader.getStatus() == 0) {
                const QString downloadedSum =
                    fileManager.calculateMD5(name, false);
                if (downloadedSum != md5sum) {
                    data.setDownloadMd5(id, downloadedSum);
                }
                status = true;
            }
        } else {
            // send 50% signal for skipped downloading ticks
            for (int i=0; i < 50; i++) {
                emit this->modelTickSignal();
                QCoreApplication::processEvents();
            }
            status = true;
        }
    }
    return status;
}

bool Model::getLevelDontHaveFile(
        const int id, const QString& md5sum, const QString& name) {
    bool status = false;
    downloader.run();
    if (!downloader.getStatus()) {
        const QString downloadedSum = fileManager.calculateMD5(name, false);
        if (downloadedSum != md5sum) {
            data.setDownloadMd5(id, downloadedSum);
        }
        status = true;
    }
    return status;
}

void Model::getLevel(int id) {
    assert(id > 0);
    if (id > 0) {
        bool status = false;
        ZipData zipData = data.getDownload(id);
        downloader.setUrl(zipData.url);
        downloader.setSaveFile(zipData.name);
        // this if just slips up execution but has nothing to do with the error
        if (fileManager.checkFile(zipData.name, false)) {
            qWarning() << "File exists:" << zipData.name;
            status = getLevelHaveFile(id, zipData.md5sum, zipData.name);
        } else {
            qDebug() << "File does not exist:" << zipData.name;
            status = getLevelDontHaveFile(id, zipData.md5sum, zipData.name);
        }
        if (status == true) {
            if (!unpackLevel(id, zipData.name)) {
                qDebug() << "unpackLevel failed";
            }
        }
    }
}

const InfoData Model::getInfo(int id) {
    return data.getInfo(id);
}

const QString Model::getWalkthrough(int id) {
    return data.getWalkthrough(id);
}
