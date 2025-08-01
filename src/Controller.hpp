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

#ifndef SRC_CONTROLLER_HPP_
#define SRC_CONTROLLER_HPP_
#include <QObject>
#include <QThread>
#include "../src/Model.hpp"

/**
 * The controller activate UI thread work or light instant work on the model
 * 
 */
class Controller : public QObject {
    Q_OBJECT

 public:
    static Controller& getInstance() {
        // cppcheck-suppress threadsafety-threadsafety
        static Controller instance;
        return instance;
    }

    void setup(const QString& level, const QString& game);
    void setupGame(int id);
    void setupLevel(int id);
    void updateLevel(int id);
    void syncLevels();
    void getCoverList(QVector<ListItemData*>* items);

    int checkGameDirectory(int id);
    void getList(QVector<ListItemData>* list);
    const InfoData getInfo(int id);
    const QString getWalkthrough(int id);
    bool link(int id);
    int getItemState(int id);

 signals:
    void controllerGenerateList(const QList<int>& availableGames);
    void controllerTickSignal();
    void controllerDownloadError(int status);
    void controllerReloadLevelList();
    void controllerLoadingDone();

 private:
    Controller();
    ~Controller();
    void initThreads();

    void runOnThreadA(std::function<void()> func);
    void runOnThreadB(std::function<void()> func);

    QScopedPointer<QThread> threadA;
    QScopedPointer<QThread> threadB;
    QScopedPointer<QObject> workerA;
    QScopedPointer<QObject> workerB;

    Data& data = Data::getInstance();
    FileManager& fileManager = FileManager::getInstance();
    Model& model = Model::getInstance();
    Downloader& downloader = Downloader::getInstance();

    Q_DISABLE_COPY(Controller)
};

#endif  // SRC_CONTROLLER_HPP_
