#include "Controller.h"
#include <QDebug>

Controller::Controller(QObject *parent) : QObject(parent), ControllerThread(nullptr)
{
    ControllerThread = new QThread();
    this->moveToThread(ControllerThread);
    connect(ControllerThread, &QThread::finished, ControllerThread, &QThread::deleteLater);
    ControllerThread->start();
    /* this should make emits to signals from the controller and to the controller happen
     * on another thread event loop so TombRaiderLinuxLauncher (view object) should have a slot for
     * getting the results, update the GUI state, if the controller returns something it's just the
     * state of some logic right before it calls the thread signal.
     * Every method here should emit a signal to this object here again and use only quick logic
     * in order to get back out again and never hold up the GUI thread, never. Even if someone have like
     * the slowest hard drives in the world, so the game couldn't even run probably it will not wait for
     * it and keep updating the GUI. This is the most safe and correct way to do this I think.
     */

    // Those start threaded work a signal hub
    connect(this, SIGNAL(setupCampThreadSignal(const QString&, const QString&)),
        this, SLOT(setupCampThread(const QString&, const QString&)));
    connect(this, SIGNAL(setupLevelThreadSignal(int)), this, SLOT(setupLevelThread(int)));


    // Those work like a signal hub for threaded work, like in between callback
    connect(&Model::getInstance(), SIGNAL(modelTickSignal()), this, SLOT(passTickSignal()));
    connect(&FileManager::getInstance(), SIGNAL(fileWorkTickSignal()), this, SLOT(passTickSignal()));
    connect(&Downloader::getInstance(), SIGNAL(networkWorkTickSignal()), this, SLOT(passTickSignal()));

    // The TombRaiderLinuxLauncher object have the connect and slot for last signal emit that displays
    // the result, we pass back what was return before like this
    // connect(&Controller::getInstance(), SIGNAL(setupCampDone(bool)), this, SLOT(checkCommonFiles(bool)));
}

Controller::~Controller()
{
    ControllerThread->quit();
    ControllerThread->wait();
    delete ControllerThread;
    ControllerThread = nullptr;
}

int Controller::checkGameDirectory(int id)
{
    return model.checkGameDirectory(id);
}


void Controller::setupCamp(const QString& level, const QString& game)
{
    emit setupCampThreadSignal(level,game);
}

void Controller::setupCampThread(const QString& level, const QString& game)
{
    bool status = model.setDirectory(level,game);
    emit setupCampDone(status);
}

bool Controller::setupOg(int id)
{
    return model.setUpOg(id);
}

void Controller::setupLevel(int id)
{
    emit setupLevelThreadSignal(id);
}

void Controller::setupLevelThread(int id)
{
    bool staus = model.getGame(id);
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

int Controller::getItemState(int id)
{
    return model.getItemState(id);
}

void Controller::passTickSignal()
{
    emit controllerTickSignal();
}

