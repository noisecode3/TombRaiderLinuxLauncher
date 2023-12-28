#include "controller.h"
Controller::Controller(QObject *parent) : QObject(parent)
{
    workerThread = new WorkerThread(3, true, "folderPath", "directoryPath");
    connect(workerThread, &WorkerThread::finished, this, &Controller::onWorkerThreadFinished);
}
void Controller::startWorkerThread()
{
    workerThread->start();
}

void Controller::stopWorkerThread()
{
    workerThread->quit();
}

void Controller::onWorkerThreadFinished()
{
    // Handle worker thread finished event
}
