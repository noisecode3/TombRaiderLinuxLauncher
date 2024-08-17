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

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef SRC_MODEL_H_
#define SRC_MODEL_H_

#include <QObject>
#include <QMap>
#include <QDebug>
#include "Data.h"
#include "FileManager.h"
#include "Network.h"

class InstructionManager : public QObject
{
    Q_OBJECT
 public:
    using Instruction = std::function<void(int id)>;

    void addInstruction(int id, const Instruction& instruction)
    {
        instructionsMap[id] = instruction;
    }

 public slots:
    void executeInstruction(int id)
    {
        auto it = instructionsMap.find(id);
        if (it != instructionsMap.end())
        {
            it.value()(id);
        }
        else
        {
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

 signals:
    void modelTickSignal();

 private:
    Data& data = Data::getInstance();
    FileManager& fileManager = FileManager::getInstance();
    Downloader& downloader = Downloader::getInstance();
    InstructionManager instructionManager;

    explicit Model(QObject *parent = nullptr);
    ~Model();

    Q_DISABLE_COPY(Model)
};

#endif  // SRC_MODEL_H_
