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

#ifndef SRC_CONTROLLER_H_
#define SRC_CONTROLLER_H_
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

    int checkGameDirectory(int id);
    void checkCommonFiles();
    void setup(const QString& level, const QString& game);
    void setupGame(int id);
    void setupLevel(int id);

    void getList(QVector<ListItemData>* list);
    const InfoData getInfo(int id);
    const QString getWalkthrough(int id);
    bool link(int id);
    int getItemState(int id);

 signals:
    void controllerAskGame(int id);
    void controllerGenerateList();
    void controllerTickSignal();
    void controllerDownloadError(int status);

    void checkCommonFilesThreadSignal();
    void setupThreadSignal(const QString& level, const QString& game);
    void setupGameThreadSignal(int id);
    void setupLevelThreadSignal(int id);

 private:
    explicit Controller(QObject *parent = nullptr);
    void initializeThread();
    ~Controller();

    Data& data = Data::getInstance();
    FileManager& fileManager = FileManager::getInstance();
    Model& model = Model::getInstance();
    Downloader& downloader = Downloader::getInstance();
    QScopedPointer<QThread> controllerThread;

    Q_DISABLE_COPY(Controller)
};

#endif  // SRC_CONTROLLER_H_
