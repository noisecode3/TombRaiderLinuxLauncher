#include "Controller.h"
#include <QDebug>

Controller::Controller(QObject *parent) : QObject(parent), workerRunning(false)
{
    /* 
    Im not sure I need the thread for something else but found that
    it was working by just using signals/slots to update the gui
    workerThread = new QThread();

    // Move the Controller to a separate thread
    this->moveToThread(workerThread);

    // Connect signals and slots for thread management
    connect(workerThread, SIGNAL(started()), this, SLOT(doSetupOgWork()));
    connect(workerThread, SIGNAL(started()), this, SLOT(doSetupLevelWork()));
    connect(workerThread, &QThread::finished, workerThread, &QThread::deleteLater);
    */
}

Controller::~Controller()
{
    // Cleanup: Ensure the worker thread is stopped and cleaned up
   // stopWorkerThread();
}
/*
void Controller::startWorkerThread()
{
    if (!workerRunning)
    {
        workerThread->start();
        workerRunning = true;
    }
}

void Controller::stopWorkerThread()
{
    if (workerRunning)
    {
        workerThread->quit();
        workerThread->wait();
        workerRunning = false;
    }
}
*/
int Controller::checkGameDirectory(int id)
{
    return model.checkGameDirectory(id);
}

bool Controller::setupCamp(const QString& level, const QString& game)
{
    return model.setDirectory(level,game);
}

bool Controller::setupOg(int id)
{
    return model.setUpOg(id);
}

bool Controller::setupLevel(int id)
{
    id_m = id;
    //startWorkerThread();
    doSetupLevelWork();
    //stopWorkerThread();
    return true;
}

void Controller::getList(QVector<ListItemData>& list)
{
    return model.getList(list);
}

const InfoData Controller::getInfo(int id)
{
    return model.getInfo(id);
}

const QString Controller::getWalkthrough(int id)
{
    return model.getWalkthrough(id);
}

bool Controller::link(int id)
{
    return model.setLink(id);
}

bool Controller::link(const QString& levelDir)
{
    return false;
}

int Controller::getItemState(int id)
{
    return model.getItemState(id);
}

void Controller::doSetupOgWork()
{
    model.getGame(id_m);
}

void Controller::doSetupLevelWork()
{
    // Your worker functionality goes here
    qDebug() << "Worker is doing some work...";
    model.getGame(id_m);

    // Emit a signal to indicate the work is finished
    //emit workFinished();
}

void Controller::WorkerThreadFinished()
{
    qDebug() << "Worker thread finished.";
    // Handle worker thread finished event
}
