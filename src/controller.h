#ifndef CONTROLLER_H
#define CONTROLLER_H
#include <QObject>
#include "worker.h"
class Controller : public QObject
{
    Q_OBJECT
public:
    explicit Controller(QObject *parent = nullptr);
    void startWorkerThread();
    void stopWorkerThread();

private slots:
    void onWorkerThreadFinished();

private:
    WorkerThread *workerThread;
};
#endif