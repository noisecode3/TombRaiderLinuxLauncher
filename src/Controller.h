#ifndef CONTROLLER_H
#define CONTROLLER_H
#include <QObject>
#include <QThread>
#include "Model.h"

/**
 * The controller activate UI thread work or light instant work on the model
 * 
 */
class Controller : public QObject
{
    Q_OBJECT
public:
    static Controller& getInstance()
    {
        static Controller instance;
        return instance;
    }
    void startWorkerThread();
    void stopWorkerThread();
    bool setupCamp(const QString& level, const QString& game);
    bool setupOg(int id);
    bool setupLevel(int id);
    void getList(QVector<ListItemData>& list);
    bool link(int id);
    bool link(const QString& levelDir);
    const InfoData getInfo(int id);
    const QString getWalkthrough(int id);
    int getItemState(int id);
    int checkGameDirectory(int id);

signals:
    void workFinished();  // Declare the signal

private slots:
    void doWork();  // Worker functionality
    void onWorkerThreadFinished();

private:
    QThread* workerThread;
    bool workerRunning;

    Model& model = Model::getInstance();

    Controller(QObject *parent = nullptr);
    ~Controller();
    Q_DISABLE_COPY(Controller)
};

#endif // CONTROLLER_H
