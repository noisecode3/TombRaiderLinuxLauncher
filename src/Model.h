#ifndef MODEL_H
#define MODEL_H

#include <QObject>
#include "Data.h"
#include "FileManager.h"
#include "Network.h"

class Model : public QObject
{
    Q_OBJECT
public:
    static Model& getInstance()
    {
        static Model instance;
        return instance;
    };
    bool setDirectory(const QString& level, const QString& game);
    int checkGameDirectory(int id);
    int checkLevelDirectory(int id);
    void getList(QVector<ListItemData>& list);
    int getItemState(int id);
    bool setLink(int id);
    QString getGameDirectory(int id);
    bool setUpOg(int id);
    bool getGame(int id);
    const InfoData getInfo(int id);
    const QString getWalkthrough(int id);
private:
    Data& data = Data::getInstance();
    FileManager& fileManager = FileManager::getInstance();
    Downloader& downloader = Downloader::getInstance();
    Model(QObject *parent = nullptr) : QObject(parent) {};
    ~Model() {};

    Q_DISABLE_COPY(Model)
};

#endif // MODEL_H
