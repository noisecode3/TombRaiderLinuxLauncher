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
    void setupCamp(const QString& level, const QString& game);
    bool setupOg(int id);
    void setupLevel(int id);
    void getList(QVector<ListItemData>& list);
    bool link(int id);
    const InfoData getInfo(int id);
    const QString getWalkthrough(int id);
    int getItemState(int id);
    int checkGameDirectory(int id);

 public slots:
    void passTickSignal();
    void passDownloadError(int status);
    void setupCampThread(const QString& level, const QString& game);
    void setupLevelThread(int id);

 signals:
    void controllerTickSignal();
    void controllerDownloadError(int status);
    void setupCampThreadSignal(const QString& level, const QString& game);
    void setupCampDone(bool status);
    void setupLevelThreadSignal(int id);

 private:
    Model& model = Model::getInstance();

    explicit Controller(QObject *parent = nullptr);
    ~Controller();
    QThread* ControllerThread;
    Q_DISABLE_COPY(Controller)
};

#endif  // SRC_CONTROLLER_H_
