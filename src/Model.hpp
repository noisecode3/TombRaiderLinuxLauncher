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

#ifndef SRC_MODEL_HPP_
#define SRC_MODEL_HPP_

#include <QAbstractListModel>
#include <QObject>
#include <QMap>
#include <QBitArray>
#include <QDebug>
#include <QtCore>

#include <cassert>

#include "../src/Path.hpp"
#include "../src/Data.hpp"
#include "../src/FileManager.hpp"
#include "../src/Network.hpp"
#include "../src/Runner.hpp"
#include "../src/PyRunner.hpp"

class InstructionManager : public QObject {
    Q_OBJECT
 public:
    using Instruction = std::function<void(int id)>;

    void addInstruction(int id, const Instruction& instruction) {
        instructionsMap[id] = instruction;
    }

 public slots:
    void executeInstruction(int id) {
        auto it = instructionsMap.find(id);
        if (it != instructionsMap.end()) {
            it.value()(id);
        } else {
            qDebug() << "Invalid instruction ID";
        }
    }

 private:
    QMap<int, Instruction> instructionsMap;
};

class Model : public QObject {
    Q_OBJECT

 public:
    static Model& getInstance() {
        // cppcheck-suppress threadsafety-threadsafety
        static Model instance;
        return instance;
    }
    void checkCommonFiles(QList<int>* games);
    int checkGameDirectory(int id);
    int checkLevelDirectory(int id);
    bool deleteZip(int id);
    bool backupSaveFiles(int id);
    void getList(QVector<QSharedPointer<ListItemData>>* list);
    void getCoverList(QVector<QSharedPointer<ListItemData>> tiems);
    int getItemState(int id);
    bool runUmu(const int id);
    void setUmuEnv(const QVector<QPair<QString, QString>>& environment);
    void setUmuSetup(bool setup);
    bool runWine(const qint64 id);
    void setWineEnv(const QVector<QPair<QString, QString>>& environment);
    void setWineSetup(bool setup);
    bool runLutris(const QStringList& arg);
    bool runSteam(const int id);
    bool runBash(const int id);
    bool setLink(int id);
    QString getGameDirectory(int id);
    QString getExecutableName(int type);
    void setupGame(int id);
    void getLevel(int id);
    const InfoData getInfo(int id);
    const int getType(int id);
    const QString getWalkthrough(int id);
    bool setupDirectories(const QString& level, const QString& game);
    void setup(const QString& level, const QString& game);
    void updateLevel(const int id);
    void syncLevels();

 signals:
    void generateListSignal(QList<int> availableGames);
    void modelTickSignal();
    void modelReloadLevelListSignal();
    void modelLoadingDoneSignal();

 private:
    bool getLevelHaveFile(
        const int id, const QString& md5sum, Path path);
    bool getLevelDontHaveFile(
        const int id, const QString& md5sum, Path path);

    Runner m_wineRunner   = Runner("wine");
    Runner m_lutrisRunner = Runner("lutris");
    Runner m_steamRunner  = Runner("steam");
    Runner m_umuRunner    = Runner("umu-run");
    Runner m_bashRunner   = Runner("bash");

    PyRunner m_pyRunner;
    Data& data = Data::getInstance();
    FileManager& fileManager = FileManager::getInstance();
    Downloader& downloader = Downloader::getInstance();

    Model();
    ~Model();

    Q_DISABLE_COPY(Model)
};




#endif  // SRC_MODEL_HPP_
