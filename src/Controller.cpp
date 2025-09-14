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
    threadA.reset(new QThread());
    threadB.reset(new QThread());
    workerA.reset(new QObject());
    workerB.reset(new QObject());

    workerA->moveToThread(threadA.data());
    workerB->moveToThread(threadB.data());

    threadA->start();
    threadB->start();

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
}

Controller::~Controller() {
    threadA->quit();
    threadB->quit();
    threadA->wait();
    threadB->wait();
}

void Controller::runOnThreadA(std::function<void()> func) {
    QMetaObject::invokeMethod(
            workerA.data(), [func]() { func(); }, Qt::QueuedConnection);
}

void Controller::runOnThreadB(std::function<void()> func) {
    QMetaObject::invokeMethod(
            workerB.data(), [func]() { func(); }, Qt::QueuedConnection);
}

// Threaded work
void Controller::setup() {
    runOnThreadA([=]() { model.setup(); });
}

void Controller::setupGame(int id) {
    runOnThreadA([=]() { model.setupGame(id); });
}

void Controller::setupLevel(int id) {
    runOnThreadA([=]() { model.getLevel(id); });
}

void Controller::updateLevel(int id) {
    runOnThreadA([=]() { model.updateLevel(id); });
}

void Controller::syncLevels() {
    runOnThreadA([=]() { model.syncLevels(); });
}

void Controller::getCoverList(QVector<QSharedPointer<ListItemData>> items) {
    runOnThreadB([=]() { model.getCoverList(items); });
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
