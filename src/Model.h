#ifndef MODEL_H
#define MODEL_H

#include <QObject>
#include <QMap>
#include <QDebug>
#include "Data.h"
#include "FileManager.h"
#include "Network.h"

class InstructionManager : public QObject {
    Q_OBJECT
public:
    using Instruction = std::function<void(int id)>;

    void addInstruction(int id, const Instruction& instruction) {
        instructionsMap[id] = instruction;
    }

public slots:
    void executeInstruction(int id) {
        auto it = instructionsMap.find(id);
        if (it != instructionsMap.end()) {
            it.value()(id);
        } else {
            qDebug() << "Invalid instruction ID";
        }
    }

private:
    QMap<int, Instruction> instructionsMap;
};

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
    Model(QObject *parent = nullptr) : QObject(parent) {
        manager.addInstruction(4, [this](int id) {
            qDebug() << "Perform Operation A";
            const QString s = "/"+QString::number(id) + ".TRLE";
            fileManager.makeRelativeLink(s,"/The Rescue.exe","/tomb4.exe");
        });
    };
    ~Model() {};

    InstructionManager manager;
    Q_DISABLE_COPY(Model)
};

#endif // MODEL_H
