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

#include <QObject>
#include <QMap>
#include <QDebug>
#include <QtCore>
#include <cassert>
#include "Data.hpp"
#include "FileManager.hpp"
#include "Network.hpp"

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
    const QList<int>& checkCommonFiles();
    int checkGameDirectory(int id);
    int checkLevelDirectory(int id);
    void getList(QVector<ListItemData>* list);
    int getItemState(int id);
    bool setLink(int id);
    QString getGameDirectory(int id);
    void setupGame(int id);
    bool getLevel(int id);
    const InfoData getInfo(int id);
    const QString getWalkthrough(int id);
    bool setDirectory(const QString& level, const QString& game);
    bool setup(const QString& level, const QString& game);

 signals:
    void generateListSignal(QList<int> availableGames);
    void modelTickSignal();

 private:
    bool getLevelHaveFile(
        const int id, const QString& md5sum, const QString& name);
    bool getLevelDontHaveFile(
        const int id, const QString& md5sum, const QString& name);
    bool unpackLevel(const int id, const QString& name);

    QList<int> m_availableGames;
    Data& data = Data::getInstance();
    FileManager& fileManager = FileManager::getInstance();
    Downloader& downloader = Downloader::getInstance();
    InstructionManager instructionManager;

    Model();
    ~Model();

    Q_DISABLE_COPY(Model)
};

#endif  // SRC_MODEL_HPP_
