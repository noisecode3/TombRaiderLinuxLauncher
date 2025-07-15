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
    void getList(QVector<ListItemData>* list);
    void getCoverList(QVector<ListItemData*>* tiems);
    int getItemState(int id);
    bool runWine(const int id);
    bool setLink(int id);
    QString getGameDirectory(int id);
    QString getExecutableName(int id);
    void setupGame(int id);
    void getLevel(int id);
    const InfoData getInfo(int id);
    const QString getWalkthrough(int id);
    bool setupDirectories(const QString& level, const QString& game);
    void setup(const QString& level, const QString& game);
    bool updateLevel(const int id);

 signals:
    void generateListSignal(QList<int> availableGames);
    void modelTickSignal();
    void modelReloadLevelListSignal();

 private:
    bool getLevelHaveFile(
        const int id, const QString& md5sum, const QString& name);
    bool getLevelDontHaveFile(
        const int id, const QString& md5sum, const QString& name);
    bool unpackLevel(const int id, const QString& name, const QString& exe);

    Runner m_wineRunner = Runner("/usr/bin/wine");
    PyRunner m_pyRunner;
    Data& data = Data::getInstance();
    FileManager& fileManager = FileManager::getInstance();
    Downloader& downloader = Downloader::getInstance();

    Model();
    ~Model();

    Q_DISABLE_COPY(Model)
};




#endif  // SRC_MODEL_HPP_
