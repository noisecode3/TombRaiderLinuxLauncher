#include <QDebug>
#include "Model.h"
bool Model::setDirectory(const QString& level, const QString& game)
{
    if (fileManager.setUpCamp(level,game) &&
        downloader.setUpCamp(level) &&
        data.initializeDatabase(level))
        return true;
    else
        return false;
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
        return fileManager.checkFileInfo(s,true);
    return -1;
}

void Model::getList(QVector<ListItemData>& list)
{
    list = data.getListItems();
}


int Model::getItemState(int id)
{
    if(id < 0)
        return 1;
    else if (id > 0)
    {
        QString map(QString::number(id) + ".TRLE");
        if(fileManager.checkDir(map, false))
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

bool Model::setUpOg(int id)
{
    std::array<QVector<QString>,2> list = data.getFileList(id,false);
    const size_t s = list[0].size();
    const size_t sm = list[1].size();
    if (s!=sm)
    {
        qDebug() << "Corrupt List";
        return false;
    }
    const QString& sd = "/Original.TR" + QString::number(id) +"/";
    const QString& sg = getGameDirectory(id) + "/";
    for (size_t i = 0; i < s; i++)
    {
        const QString& fFile = list[0][i];
        const QString& fMd5sum = list[1][i];
        const QString&  calculated = fileManager.calculateMD5(sg+fFile, true);
        if(fMd5sum == calculated)
        {
            fileManager.copyFile(sg+fFile, sd+fFile,  true);
        }
        else
        {
            qDebug() << "Original file was modified, had" << fMd5sum
                     << " got " << calculated << " for file " << fFile << Qt::endl;
            fileManager.cleanWorkingDir(sd + fFile);
            break;
        }
    }
    if (fileManager.backupGameDir(sg))
    {
        const QString&  src = sd.chopped(1);
        const QString&  des = sg.chopped(1);
        if(!fileManager.linkGameDir(src,des))
        {
            return true;
        }
    }
    qWarning() << "Failed to rename directory:" << sg;
    return false;
}

bool Model::getGame(int id)
{
    if (id<0)
        return setUpOg(-id);
    if (id)
    {
        ZipData zipData = data.getDownload(id);
        downloader.setUrl(zipData.url);
        downloader.setSaveFile(zipData.name);

        if (fileManager.checkFileInfo(zipData.name,false))
        {
            qDebug() << "File exists:" << zipData.name;
            if(fileManager.calculateMD5(zipData.name,false) != zipData.md5sum)
            {
                downloader.run();
            }
        } else {
            qWarning() << "File does not exist:" << zipData.name;
            downloader.run();
        }
        //fileManager.createDirectory(QString::number(id)+".TRLE", false);
        fileManager.extractZip(zipData.name, QString::number(id)+".TRLE");
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

#include "Model.moc"
