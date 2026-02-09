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
#include "../src/assert.hpp"
#include <QtGlobal>
#include <qlogging.h>

Model::Model() :
        data(Data::getInstance()),
        fileManager(FileManager::getInstance()),
        downloader(Downloader::getInstance()) {

    connect(&m_runner, &Runner::runningDone,
        this,   [this]() { emit modelRunningDoneSignal(); },
        Qt::QueuedConnection);
}

Model::~Model() {}

void Model::setup() {
    #ifndef TEST
    Path::setProgramFilesPath();
    Path::setResourcePath();
    #endif
    if(data.initializeDatabase()) {
        QList<int> commonFiles;
        checkCommonFiles(&commonFiles);
        // Iterate backward to avoid index shifting
        for (int i = 4; i >= 0; i--) {
            int dirStatus = commonFiles[i];

            Path p(Path::resource);
            p << QString("Original.TR%1").arg(i + 1);
            if (p.isDir()) {
                // game id number
                commonFiles[i] = i + 1;
            } else if (dirStatus == 2) {
                // negative id for not installed
                commonFiles[i] = -(i + 1);
            } else {
                commonFiles.removeAt(i);
            }
        }
        emit generateListSignal(commonFiles);
        QCoreApplication::processEvents();
    } else {
        qCritical() << "Initializing database failed!";
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
            qDebug() << "Could be a broken link/regular file";
        } else {
            games->append(0);
            qDebug() << "File don't exist";
        }
    }
}

QString Model::getPrograFilesDirectory(int id) {
    QString value;
    FolderNames folder;
    if (folder.data.contains(id)) {
        value = folder.data[id][0];
    } else {
        qDebug() << "Id number:" << id << " is not a game.";
    }
    return value;
}

QString Model::getExecutableName(int type) {
    QString value;
    ExecutableNames name;
    if (name.data.contains(type)) {
        value = name.data[type];
    } else {
        qDebug() << "Id number:" << type << " is not a game.";
    }
    return value;
}

int Model::checkGameDirectory(int id) {
    int status = -1;
    Path path(Path::programFiles);
    path << getPrograFilesDirectory(id);
    if (path.get() != "") {
        status = fileManager.checkFileInfo(path);
    }
    return status;
}

void Model::getList(QVector<QSharedPointer<ListItemData>>* list) {
    *list = data.getListItems();
}

void Model::getCoverList(QVector<QSharedPointer<ListItemData>> items) {
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

void Model::run(RunnerOptions options) {
    // Setup the basic
    m_runner.setProgram(options.command);

    Path path = Path(Path::resource);
    fileManager.addLevelDir(path, options.id);

    // Command specific
    if ((options.command == UMU) || (options.command ==  WINE)) {
        // Path the executable directory
        fileManager.getExtraPathToExe(path, data.getType(options.id));

        // Shell arguments
        for (QPair<QString, QString>& env : options.envList) {
            m_runner.addEnvironmentVariable(env);
        }

        // Wine binary path
        if (!options.winePath.isEmpty()|| (options.command ==  WINE )) {
            m_runner.setWinePath(options.winePath);
        }

        // Executable to run
        const quint64 type = getType(options.id);
        m_runner.addArguments(QStringList() << ExecutableNames().data[type]);
        if (options.setup) {
            m_runner.addArguments(QStringList() << "-setup");
        }
    } else if (options.command == LUTRIS) {
        m_runner.addArguments(options.arguments);
    } else if (options.command == STEAM) {
        const quint64 type = getType(options.id);
        const quint64 steamID = SteamAppIds().data[type];
        const QString arument = QString("steam://run/%1").arg(steamID);
        m_runner.addArguments(QStringList() << arument);
    } else if (options.command == BASH) {
        m_runner.addArguments(QStringList() << "start.sh");
        if (options.setup) {
            m_runner.addArguments(QStringList() << "-setup");
        }
    }

    m_runner.setCurrentWorkingDirectory(path.get());
    m_runner.run();
    // m_runner.clear();

    //emit modelRunningDoneSignal();
}

bool Model::setLink(int id) {
    bool status = false;
    Path from(Path::resource);
    Path to(Path::programFiles);

    fileManager.addLevelDir(from, id);
    to << getPrograFilesDirectory(getType(id));

    qDebug() << "Model::setLink :to " <<  to.get();
    qDebug() << "Model::setLink :from " << from.get();

    if (from.isDir()) {
        status = fileManager.linkPaths(from, to);
    } else {
        qDebug() << "Directory: " << from.get() << " seems to bee missing";
    }

    return status;
}

void Model::setupGame(int id) {
    QVector<FileListItem> list = data.getFileList(id);
    const size_t s = list.size();
    Q_ASSERT_WITH_TRACE(s != 0);
    Path from(Path::programFiles);
    Path to(Path::resource);

    from << getPrograFilesDirectory(id);
    to << QString("Original.TR%1").arg(id);

    quint64 lastPrintedPercent = 0;
    for (size_t i = 0; i < s; i++) {
        Path fromFile = from;
        fromFile << list[i].path;
        Path toFile = to;
        toFile << list[i].path;

        const QString calculated = fileManager.calculateMD5(fromFile);

        if (list[i].md5sum == calculated) {
            (void)fileManager.copyFile(fromFile, toFile);

            qint64 percent = (i * 100) / s;
            if (percent != lastPrintedPercent) {
                int delta = percent - lastPrintedPercent;
                for (int j=0; j < delta; j++) {
                    emit this->modelTickSignal();
                    QCoreApplication::processEvents();
                }
                lastPrintedPercent = percent;
            }
        } else {
            qDebug() << "Original file was modified, had" << list[i].md5sum
                     << " got " << calculated << " for file "
                     << list[i].path << Qt::endl;
            (void)fileManager.cleanWorkingDir(to);
            break;
        }
    }

    if (fileManager.backupGameDir(from)) {
        if (!fileManager.linkPaths(to, from)) {
            qDebug() << "Faild to create the link to the new game directory";
        }
    }

    if (lastPrintedPercent < 100) {
        for (int i = lastPrintedPercent; i < 100; i++) {
            emit this->modelTickSignal();
            QCoreApplication::processEvents();
        }
    }
}

bool Model::checkZip(int id) {
    bool status = false;
    ZipData zipData = data.getDownload(id);
    Path path(Path::resource);
    path << zipData.m_fileName;
    if (path.isFile()) {
        status = true;
    }
    return status;
}

bool Model::deleteZip(int id) {
    bool status = false;
    ZipData zipData = data.getDownload(id);
    Path path(Path::resource);
    path << zipData.m_fileName;
    if (path.isFile()) {
        status = fileManager.removeFileOrDirectory(path);
    }
    return status;
}

bool Model::deleteLevel(int id) {
    bool status = false;
    if (id != 0) {
        Path path = Path(Path::resource);
        fileManager.addLevelDir(path, id);
        Path testPath = Path(Path::resource);
        Q_ASSERT_WITH_TRACE(path.get() != testPath.get());

        quint64 error = fileManager.removeFileOrDirectory(path);
        if (error == 0) {
            status = true;
        }

        if (id<0) {
            g_settings.setValue(
                    QString("installed/game%1").arg(-id),
                    "fales");
        } else {
            g_settings.setValue(
                    QString("installed/level%1").arg(id),
                    "false");
        }
    }

    return status;
}

bool Model::backupSaveFiles(int id) {
    bool status = false;
    Path path = Path(Path::resource);
    fileManager.addLevelDir(path, id);
    QStringList list = fileManager.getSaveFiles(path);
    return status;
}

void Model::updateLevel(const int id) {
    qint64 status = m_pyRunner.updateLevel(id);
    emit this->modelLoadingDoneSignal();
}

void Model::syncLevels() {
    qint64 status = m_pyRunner.syncCards();
    emit this->modelLoadingDoneSignal();
}

bool Model::getLevelHaveFile(
        const int id, const QString& md5sum, Path path) {
    bool status = false;
    if (md5sum != "") {
        const QString existingFilesum = fileManager.calculateMD5(path);
        if (existingFilesum != md5sum) {
            downloader.run();
            if (downloader.getStatus() == 0) {
                const QString downloadedSum =
                    fileManager.calculateMD5(path);
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
        const int id, const QString& md5sum, Path path) {
    bool status = false;
    downloader.run();
    if (downloader.getStatus() == 0) {
        const QString downloadedSum = fileManager.calculateMD5(path);
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
        Path path(Path::resource);
        path << zipData.m_fileName;
        downloader.setUrl(zipData.m_URL);
        downloader.setSaveFile(path);
        // this if just slips up execution but has nothing to do with the error


        if (path.isFile()) {
            qWarning() << "File exists:" << path.get();
            status = getLevelHaveFile(id, zipData.m_MD5sum, path);
        } else {
            qDebug() << "File does not exist:" << zipData.m_fileName;
            status = getLevelDontHaveFile(id, zipData.m_MD5sum, path);
        }
        if (status == true) {
            if (!fileManager.extractZip(zipData)) {
                qDebug() << "unpackLevel failed";
            }
        }
        if (g_settings.value("DeleteZip").toBool()) {
            deleteZip(id);
        }
    }
}

const InfoData Model::getInfo(int id) {
    return data.getInfo(id);
}

const quint64 Model::getType(qint64 id) {
    static const QMap<quint64, quint64> coreType = {
            {0, 0},
            {1, 1},
            {2, 2},
            {3, 3},
            {4, 4},
            {5, 5},
            {6, 6},
            {7, 1},
            {8, 2},
            {9, 3},
            {10, 4},
    };
    quint64 result = 0;
    if (id < 0) {
        result = coreType[id];
    } else {
        result = data.getType(id);
    }
    return result;
}

const QString Model::getWalkthrough(int id) {
    return data.getWalkthrough(id);
}

void Model::killRunner() {
    m_runner.stop();
}
