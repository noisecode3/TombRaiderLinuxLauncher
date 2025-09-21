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

#include "../src/Controller.hpp"
#include <QMetaObject>

Controller::Controller() {
    threadDatabase.reset(new QThread());
    threadFile.reset(new QThread());
    threadScrape.reset(new QThread());
    threadRun.reset(new QThread());

    workerDatabase.reset(new QObject());
    workerFile.reset(new QObject());
    workerScrape.reset(new QObject());
    workerRun.reset(new QObject());

    workerDatabase->moveToThread(threadDatabase.data());
    workerFile->moveToThread(threadFile.data());
    workerScrape->moveToThread(threadScrape.data());
    workerRun->moveToThread(threadRun.data());

    threadDatabase->start();
    threadFile->start();
    threadScrape->start();
    threadRun->start();

    connect(&model, &Model::modelTickSignal,
            this,   &Controller::controllerTickSignal,
        Qt::QueuedConnection);

    connect(&fileManager, &FileManager::fileWorkTickSignal,
            this,         &Controller::controllerTickSignal,
        Qt::QueuedConnection);

    connect(&downloader, &Downloader::networkWorkTickSignal,
            this,        &Controller::controllerTickSignal,
        Qt::QueuedConnection);

    connect(&downloader, &Downloader::networkWorkErrorSignal,
            this,        &Controller::controllerDownloadError,
        Qt::QueuedConnection);

    connect(&model, &Model::generateListSignal,
            this,   &Controller::controllerGenerateList,
        Qt::QueuedConnection);

    connect(&model, &Model::modelReloadLevelListSignal,
            this,   &Controller::controllerReloadLevelList,
        Qt::QueuedConnection);

    connect(&model, &Model::modelLoadingDoneSignal,
            this,   &Controller::controllerLoadingDone,
        Qt::QueuedConnection);

    connect(&model, &Model::modelRunningDoneSignal,
            this,   &Controller::controllerRunningDone,
        Qt::QueuedConnection);
}

Controller::~Controller() {
    threadDatabase->quit();
    threadFile->quit();
    threadScrape->quit();
    threadRun->quit();

    threadDatabase->wait();
    threadFile->wait();
    threadScrape->wait();
    threadRun->wait();
}

void Controller::runOnThreadDatabase(std::function<void()> func) {
    QMetaObject::invokeMethod(workerDatabase.data(),
            [func]() { func(); }, Qt::QueuedConnection);
}

void Controller::runOnThreadFile(std::function<void()> func) {
    QMetaObject::invokeMethod(workerFile.data(),
            [func]() { func(); }, Qt::QueuedConnection);
}

void Controller::runOnThreadScrape(std::function<void()> func) {
    QMetaObject::invokeMethod(workerScrape.data(),
            [func]() { func(); }, Qt::QueuedConnection);
}

void Controller::runOnThreadRun(std::function<void()> func) {
    QMetaObject::invokeMethod(workerRun.data(),
            [func]() { func(); }, Qt::QueuedConnection);
}

// Threaded work
void Controller::setup() {
    runOnThreadFile([=]() { model.setup(); });
}

void Controller::setupGame(int id) {
    runOnThreadFile([=]() { model.setupGame(id); });
}

void Controller::setupLevel(int id) {
    runOnThreadFile([=]() { model.getLevel(id); });
}

void Controller::updateLevel(int id) {
    runOnThreadScrape([=]() { model.updateLevel(id); });
}

void Controller::syncLevels() {
    runOnThreadScrape([=]() { model.syncLevels(); });
}

void Controller::getCoverList(QVector<QSharedPointer<ListItemData>> items) {
    runOnThreadDatabase([=]() { model.getCoverList(items); });
}

void Controller::run(RunnerOptions opptions) {
    runOnThreadRun([=]() { model.run(opptions); });
}

// UI/main thread work
int Controller::checkGameDirectory(int id) {
    return model.checkGameDirectory(id);
}

void Controller::getList(QVector<QSharedPointer<ListItemData>>* list) {
    model.getList(list);
}

const InfoData Controller::getInfo(int id) {
    return model.getInfo(id);
}

const QString Controller::getWalkthrough(int id) {
    return model.getWalkthrough(id);
}

bool Controller::link(int id) {
    return model.setLink(id);
}

int Controller::getItemState(int id) {
    return model.getItemState(id);
}
