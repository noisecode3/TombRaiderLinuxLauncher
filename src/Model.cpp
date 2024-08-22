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

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <QDebug>
#include <assert.h>
#include "Model.h"

// Those lambda should be in another header file
Model::Model(QObject *parent) : QObject(parent), checkCommonFilesIndex_m(1)
{
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

Model::~Model()
{
}

void Model::setup(const QString& level, const QString& game)
{
    setDirectory(level, game);
    checkCommonFiles();
}

bool Model::setDirectory(const QString& level, const QString& game)
{
    if (fileManager.setUpCamp(level, game) &&
        downloader.setUpCamp(level) &&
        data.initializeDatabase(level))
        return true;
    else
        return false;
}

void Model::checkCommonFiles()
{
    int index = checkCommonFilesIndex_m;
    assert(index >= 1 && index <= 5);
    for (int i = index; i <= 5; i++)
    {
        if (checkGameDirectory(i) == 2)
        {
            checkCommonFilesIndex_m = i+1;
            emit askGameSignal(i);
            return;
        }
    }
    emit generateListSignal();
}

QString Model::getGameDirectory(int id)
{
    struct FolderNames folder;
    switch (id)
    {
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

int Model::checkGameDirectory(int id)
{
    const QString s = getGameDirectory(id);
    if (s != "")
        return fileManager.checkFileInfo(s, true);
    return -1;
}

void Model::getList(QVector<ListItemData>& list)
{
    list = data.getListItems();
}

int Model::getItemState(int id)
{
    if (id < 0)
        return 1;
    else if (id > 0)
    {
        QString map(QString::number(id) + ".TRLE");
        if (fileManager.checkDir(map, false))
            return 2;
        else
            return 0;
    }
    return -1;
}

bool Model::setLink(int id)
{
    if (id < 0)
    {
        id = -id;
        const QString s = "/Original.TR" + QString::number(id);
        if (fileManager.checkDir(s, false ))
            return fileManager.linkGameDir(s, getGameDirectory(id));
    }
    else if (id > 0)
    {
        const QString s = "/"+QString::number(id) + ".TRLE";
        const int t = data.getType(id);

        if (fileManager.checkDir(s, false ))
            return fileManager.linkGameDir(s, getGameDirectory(t));
    }
    return false;
}

void Model::setupGame(int id)
{
    std::array<QVector<QString>, 2> list = data.getFileList(id, false);
    const size_t s = list[0].size();
    const size_t sm = list[1].size();
    if (s != sm)
    {
        qDebug()
            << "Corrupt list, there seems to bee"
            << " more or less checksums for the files\n";
        assert(false);
    }
    const QString& sd = "/Original.TR" + QString::number(id) +"/";
    const QString& sg = getGameDirectory(id) + "/";
    for (size_t i = 0; i < s; i++)
    {
        const QString& fFile = list[0][i];
        const QString& fMd5sum = list[1][i];
        const QString&  calculated = fileManager.calculateMD5(sg+fFile, true);
        if (fMd5sum == calculated)
        {
            fileManager.copyFile(sg+fFile, sd+fFile,  true);
        }
        else
        {
            qDebug() << "Original file was modified, had" << fMd5sum
                     << " got " << calculated << " for file "
                     << fFile << Qt::endl;
            fileManager.cleanWorkingDir(sd + fFile);
            break;
        }
    }
    if (fileManager.backupGameDir(sg))
    {
        const QString&  src = sd.chopped(1);
        const QString&  des = sg.chopped(1);
        if (!fileManager.linkGameDir(src, des))
        {
            checkCommonFiles();
            return;
        }
    }
    checkCommonFiles();
}

bool Model::getGame(int id)
{
    assert(id > 0);
    if (id)
    {
        int status = 0;
        ZipData zipData = data.getDownload(id);
        downloader.setUrl(zipData.url);
        downloader.setSaveFile(zipData.name);

        if (fileManager.checkFile(zipData.name, false))
        {
            qDebug() << "File exists:" << zipData.name;
            const QString& sum = fileManager.calculateMD5(zipData.name, false);
            if (sum != zipData.md5sum)
            {
                downloader.run();
                status = downloader.getStatus();
            }
            else
            {
                // send 50% signal here
                for (int i=0; i < 50; i++)
                {
                    emit this->modelTickSignal();
                }
            }
        }
        else
        {
            qWarning() << "File does not exist:" << zipData.name;
            downloader.run();
            status = downloader.getStatus();
        }
        if (status == 0)
        {
            fileManager.extractZip(zipData.name, QString::number(id)+".TRLE");
            instructionManager.executeInstruction(id);
            return true;
        }
    }
    return false;
}

const InfoData Model::getInfo(int id)
{
    return data.getInfo(id);
}

const QString Model::getWalkthrough(int id)
{
    return data.getWalkthrough(id);
}
