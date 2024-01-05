#ifndef CONTROLLER_H
#define CONTROLLER_H
#include <QObject>
#include "worker.h"
/**
 * @brief The controller is the way for the UI to with blocking thread work
 * 
 */
class Controller : public QObject
{
    Q_OBJECT
public:
    /**
     * 
     */
    explicit Controller(QObject *parent = nullptr);
    /**
     * 
     */
    void startWorkerThread();
    /**
     * 
     */
    void stopWorkerThread();

private slots:
    /**
     * 
     */
    void onWorkerThreadFinished();

private:
    WorkerThread *workerThread;
};
#endif