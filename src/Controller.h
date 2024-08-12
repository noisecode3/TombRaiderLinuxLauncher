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
    void setupCamp(const QString& level, const QString& game);
    bool setupOg(int id);
    void setupLevel(int id);
    void getList(QVector<ListItemData>& list);
    bool link(int id);
    const InfoData getInfo(int id);
    const QString getWalkthrough(int id);
    int getItemState(int id);
    int checkGameDirectory(int id);

private:
    Model& model = Model::getInstance();

    Controller(QObject *parent = nullptr);
    ~Controller();
    QThread* ControllerThread;
    Q_DISABLE_COPY(Controller)

public slots:
    void passTickSignal();
    void setupCampThread(const QString& level, const QString& game);
    void setupLevelThread(int id);
signals:
    void controllerTickSignal();
    void setupCampThreadSignal(const QString& level, const QString& game);
    void setupCampDone(bool status);
    void setupLevelThreadSignal(int id);
    //void setupLevelDone(bool status);
};

#endif // CONTROLLER_H
