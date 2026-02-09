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

#include "../src/globalTypes.hpp"
#include "../src/Path.hpp"
#include "../src/Data.hpp"
#include "../src/FileManager.hpp"
#include "../src/Network.hpp"
#include "../src/Runner.hpp"
#include "../src/PyRunner.hpp"
#include "../src/settings.hpp"

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
    bool checkZip(int id);
    bool deleteZip(int id);
    bool deleteLevel(int id);
    bool backupSaveFiles(int id);
    void getList(QVector<QSharedPointer<ListItemData>>* list);
    void getCoverList(QVector<QSharedPointer<ListItemData>> tiems);
    int getItemState(int id);
    void run(RunnerOptions options);
    bool setLink(int id);
    QString getPrograFilesDirectory(int id);
    QString getExecutableName(int type);
    void setupGame(int id);
    void getLevel(int id);
    const InfoData getInfo(int id);
    const quint64 getType(qint64 id);
    const QString getWalkthrough(int id);
    void killRunner();
    void setup();
    void updateLevel(const int id);
    void syncLevels();

 signals:
    void generateListSignal(QList<int> availableGames);
    void modelTickSignal();
    void modelReloadLevelListSignal();
    void modelLoadingDoneSignal();
    void modelRunningDoneSignal();

 private:
    bool getLevelHaveFile(
        const int id, const QString& md5sum, Path path);
    bool getLevelDontHaveFile(
        const int id, const QString& md5sum, Path path);

    Runner m_runner;
    PyRunner m_pyRunner;

    Data& data;
    FileManager& fileManager;
    Downloader& downloader;

    QSettings& g_settings = getSettingsInstance();

    Model();
    ~Model();

    Q_DISABLE_COPY(Model)
};




#endif  // SRC_MODEL_HPP_
