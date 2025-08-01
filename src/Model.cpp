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

#include "../src/Model.hpp"
#include "../src/Data.hpp"
#include "../src/Path.hpp"

Model::Model() {}
Model::~Model() {}

bool Model::setupDirectories(const QString& level, const QString& game) {
    bool status = false;

    if (fileManager.setUpCamp(level, game) &&
            downloader.setUpCamp(level) &&
            data.initializeDatabase(level) &&
            m_pyRunner.setUpCamp(level)) {
        Path::setProgramFilesPath();
        Path::setResourcePath();
        status = true;
    }
    return status;
}

void Model::setup(const QString& level, const QString& game) {
    if (setupDirectories(level, game) == true) {
        QList<int> commonFiles;
        checkCommonFiles(&commonFiles);
        // Iterate backward to avoid index shifting
        for (int i = 4; i >= 0; i--) {
            int dirStatus = commonFiles[i];

            Path p(Path::resource);
            p << QString("Original.TR%1").arg(i + 1);
            if (p.isDir()) {
                commonFiles[i] = i + 1;
            } else if (dirStatus == 2) {
                commonFiles[i] = -(i + 1);
            } else {
                commonFiles.removeAt(i);
            }
        }
        emit generateListSignal(commonFiles);
        QCoreApplication::processEvents();
    } else {
        // send signal to gui with error about setup fail
        qDebug() << "setupDirectories setup failed";
    }
}

void Model::checkCommonFiles(QList<int>* games) {
    for (int i = 1; i <= 5; i++) {
        int dirStatus = checkGameDirectory(i);
        if (dirStatus == 1) {  // symbolic link
            games->append(1);
        } else if (dirStatus == 2) {  // directory
            games->append(2);
        } else if (dirStatus == 3) {
            games->append(3);
            qDebug() << "File could be a broken link or a regular file";
        } else {
            games->append(0);
            qDebug() << "File don't exist";
        }
    }
}

QString Model::getGameDirectory(int id) {
    QString value;
    FolderNames folder;
    if (folder.data.contains(id)) {
        value = folder.data[id][0];
    } else {
        qDebug() << "Id number:" << id << " is not a game.";
    }
    return value;
}

QString Model::getExecutableName(int id) {
    QString value;
    ExecutableNames name;
    if (name.data.contains(id)) {
        value = name.data[id];
    } else {
        qDebug() << "Id number:" << id << " is not a game.";
    }
    return value;
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

void Model::getCoverList(QVector<ListItemData*>* items) {
    data.getCoverPictures(items);
    emit modelReloadLevelListSignal();
}

int Model::getItemState(int id) {
    int status = 0;

    Path path(Path::resource);
    if (id < 0) {
        path << QString("Original.TR%1").arg(id);
        if (path.isDir()) {
            status = 2;
        } else {
            status = 1;
        }
    } else if (id > 0) {
        path << QString("%1.TRLE").arg(id);
        qDebug() << "getItemState: path is " << path.get();
        if (path.isDir()) {
            status = 2;
        } else {
            status = 0;
        }
    } else {
        status = -1;
    }
    return status;
}

bool Model::runBash(const int id) {
    bool status = true;
    QString s;
    if (id < 0) {  // we use original game id as negative number
        int orgId = (-1)*id;
        s = QString("Original.TR%1").arg(orgId);
    } else {
        s = QString("%1.TRLE").arg(id);
    }
    const QString a = fileManager.getFullPath(s, false);
    const QString b =  ExecutableNames().data[data.getType(id)];

    m_bashRunner.setWorkingDirectory(a);
    m_bashRunner.insertArguments(QStringList()
            << QString("%1/%2/start.sh").arg(a, b));
    m_bashRunner.run();

    return status;
}

bool Model::runUmu(const int id) {
    bool status = true;

    QString a;
    if (id < 0) {  // we use original game id as negative number
        int orgId = (-1)*id;
        QString s = QString("Original.TR%1").arg(orgId);
        a = fileManager.getFullPath(s, false);
    } else {
        QString s = QString("%1.TRLE").arg(id);
        a = fileManager.getFullPath(s, false);
    }

    const QString b = fileManager.getExtraPathToExe(a);
    const QString c =  ExecutableNames().data[data.getType(id)];

    m_umuRunner.setWorkingDirectory(b);
    m_umuRunner.insertArguments(QStringList() << QString("%1/%2").arg(b, c));
    m_umuRunner.run();

    return status;
}


void Model::setUmuEnv(const QVector<QPair<QString, QString>>& environment) {
    for (const QPair<QString, QString>& e : environment) {
        m_umuRunner.insertProcessEnvironment(e);
    }
}

void Model::setUmuSetup() {
    m_umuRunner.toggleSetupFlag();
}

bool Model::runSteam(const int id) {
    bool status = true;
    QStringList arg;
    const qint64 appid =  SteamAppIds().data[data.getType(id)];
    arg << QString("steam://run/%1").arg(appid);
    m_steamRunner.insertArguments(arg);
    m_steamRunner.run();
    return status;
}

bool Model::runLutris(const QStringList& arg) {
    bool status = true;
    m_lutrisRunner.insertArguments(arg);
    m_lutrisRunner.run();
    return status;
}

bool Model::runWine(const qint64 id) {
    bool status = true;

    QString a;
    if (id < 0) {  // we use original game id as negative number
        int orgId = (-1)*id;
        QString s = QString("Original.TR%1").arg(orgId);
        a = fileManager.getFullPath(s, false);
    } else {
        QString s = QString("%1.TRLE").arg(id);
        a = fileManager.getFullPath(s, false);
    }

    const QString b = fileManager.getExtraPathToExe(a);
    const QString c =  ExecutableNames().data[data.getType(id)];

    m_wineRunner.setWorkingDirectory(b);
    m_wineRunner.insertArguments(QStringList() << QString("%1/%2").arg(b, c));
    m_wineRunner.run();
    return status;
}

void Model::setWineEnv(const QVector<QPair<QString, QString>>& environment) {
    for (const QPair<QString, QString>& e : environment) {
        m_wineRunner.insertProcessEnvironment(e);
    }
}

void Model::setWineSetup() {
    m_umuRunner.toggleSetupFlag();
}

bool Model::setLink(int id) {
    bool status = false;
    Path path(Path::resource);
    if (id < 0) {  // we use original game id as negative number
        int orgId = (-1)*id;
        path << QString("Original.TR%1").arg(orgId);
        const QString s = path.get();
        if (path.isDir()) {
            status = fileManager.linkGameDir(s, getGameDirectory(orgId));
        } else {
            qDebug() << "Dirr: " << s << " seems to bee missing";
        }
    } else {
        path << QString("%1.TRLE").arg(id);
        const QString s = path.get();
        const int t = data.getType(id);

        if (path.isDir()) {
            status = fileManager.linkGameDir(s, getGameDirectory(t));
        } else {
            qDebug() << "Dirr: " << s << " seems to bee missing";
        }
    }
    return status;
}

void Model::setupGame(int id) {
    QVector<FileListItem> list = data.getFileList(id);
    const size_t s = list.size();
    assert(s != (unsigned int)0);

    const QString levelPath = QString("/Original.TR%1/").arg(id);
    const QString gamePath = QString("/%1/").arg(getGameDirectory(id));

    for (size_t i = 0; i < s; i++) {
        const QString levelFile = QString("%1%2").arg(levelPath, list[i].path);
        const QString gameFile = QString("%1%2").arg(gamePath, list[i].path);
        const QString calculated = fileManager.calculateMD5(gameFile, true);

        if (list[i].md5sum == calculated) {
            (void)fileManager.copyFile(gameFile, levelFile,  true);
        } else {
            qDebug() << "Original file was modified, had" << list[i].md5sum
                     << " got " << calculated << " for file "
                     << list[i].path << Qt::endl;
            (void)fileManager.cleanWorkingDir(levelPath);
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

void Model::updateLevel(const int id) {
    qint64 status = m_pyRunner.updateLevel(id);
    emit this->modelLoadingDoneSignal();
}

void Model::syncLevels() {
    qint64 status = m_pyRunner.syncCards();
    emit this->modelLoadingDoneSignal();
}

bool Model::unpackLevel(const int id, const QString& name, const QString& exe) {
    bool status = false;
    const QString directory = QString("%1.TRLE").arg(id);
    if (fileManager.extractZip(name, directory, exe) == true) {
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
        downloader.setUrl(zipData.m_URL);
        downloader.setSaveFile(zipData.m_fileName);
        // this if just slips up execution but has nothing to do with the error

        Path path(Path::resource);
        path << zipData.m_fileName;

        if (path.isFile()) {
            qWarning() << "File exists:" << zipData.m_fileName;
            status = getLevelHaveFile(id, zipData.m_MD5sum, zipData.m_fileName);
        } else {
            qDebug() << "File does not exist:" << zipData.m_fileName;
            status = getLevelDontHaveFile(
                    id, zipData.m_MD5sum, zipData.m_fileName);
        }
        if (status == true) {
            if (!unpackLevel(
                id,
                zipData.m_fileName,
                getExecutableName(zipData.m_type))) {
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
