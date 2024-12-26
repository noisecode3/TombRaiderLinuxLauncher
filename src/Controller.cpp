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

#include "Controller.hpp"
#include <QDebug>

Controller::Controller(QObject *parent)
    : QObject(parent), controllerThread(new QThread(this)) {
    initializeThread();
}

Controller::~Controller() {
    controllerThread->quit();
    controllerThread->wait();
}

void Controller::initializeThread() {
    this->moveToThread(controllerThread.data());
    connect(controllerThread.data(), &QThread::finished,
            controllerThread.data(), &QThread::deleteLater);
    controllerThread->start();

    //  Using the controller thread to start model work
    /*
    connect(this, &Controller::checkCommonFilesThreadSignal,
            this, [this]() {
        model.checkCommonFiles();
    });
    */

    connect(this, &Controller::setupThreadSignal,
            this, [this](const QString& level, const QString& game) {
        model.setup(level, game);
    });

    connect(this, &Controller::setupLevelThreadSignal,
            this, [this](int id) {
        model.getLevel(id);
    });

    connect(this, &Controller::setupGameThreadSignal,
            this, [this](int id) {
        model.setupGame(id);
    });

    //  Comming back from model or other model objects
    auto tickSignal = [this](){ emit controllerTickSignal(); };
    connect(&model, &Model::modelTickSignal,
            this, tickSignal, Qt::QueuedConnection);

    connect(&fileManager, &FileManager::fileWorkTickSignal,
            this, tickSignal, Qt::QueuedConnection);

    connect(&downloader, &Downloader::networkWorkTickSignal,
            this, tickSignal, Qt::QueuedConnection);

    connect(&downloader, &Downloader::networkWorkErrorSignal,
            this, [this](int status) {
        emit controllerDownloadError(status);
    }, Qt::QueuedConnection);

    connect(&model, &Model::generateListSignal,
            this, [this](const QList<int>& availableGames) {
        emit controllerGenerateList(availableGames);
    }, Qt::QueuedConnection);
}

void Controller::checkCommonFiles() {
    emit checkCommonFilesThreadSignal();
}

void Controller::setup(const QString& level, const QString& game) {
    emit setupThreadSignal(level, game);
}

void Controller::setupGame(int id) {
    emit setupGameThreadSignal(id);
}

void Controller::setupLevel(int id) {
    emit setupLevelThreadSignal(id);
}

// Using the GUI Threads
int Controller::checkGameDirectory(int id) {
    return model.checkGameDirectory(id);
}

void Controller::getList(QVector<ListItemData>* list) {
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

