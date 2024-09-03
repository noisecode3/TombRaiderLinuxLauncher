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

#ifndef SRC_TOMBRAIDERLINUXLAUNCHER_H_
#define SRC_TOMBRAIDERLINUXLAUNCHER_H_

#include <QMainWindow>
#include <QSettings>
#include <QMessageBox>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QStringList>
#include <QListWidgetItem>
#include <QSet>
#include <QDebug>
#include <QVector>
#include <QString>
#include <algorithm>

#include "Controller.h"

QT_BEGIN_NAMESPACE
namespace Ui { class TombRaiderLinuxLauncher; }
QT_END_NAMESPACE

/**
 * 
 */
class TombRaiderLinuxLauncher : public QMainWindow
{
    Q_OBJECT
 public:
    /**
     * Create Main Window
     */
    explicit TombRaiderLinuxLauncher(QWidget *parent = nullptr);
    ~TombRaiderLinuxLauncher();

 public slots:
    /**
     * Called by Link function button
     */
    void linkClicked();
    /**
     * Called by Download function button
     */
    void downloadClicked();
    /**
     * Called by Info navigation button
     */
    void infoClicked();
    /**
     *  Called by Walkthrough navigation button
     */
    void walkthroughClicked();
    /**
     *  Called by Back navigation button
     */
    void backClicked();
    /**
     * Called by first time setup button
     */
    void setOptionsClicked();
    /**
     * Called when selecting menu level, for disable and enable buttons
     */
    void onListItemSelected();
    /**
     * Called when 1% work done calculated from total work steps
     */
    void workTick();
    /**
     *
     */
    void downloadError(int status);
    /**
     *
     */
    void askGame(int id);
    /**
     *
     */
    void generateList();
    void sortByAuthor();
    void sortByTitle();
    void sortByDifficulty();
    void sortByDuration();
    void sortByClass();
    void sortByType();
    void sortByReleaseDate();

 private:
    /**
     * 
     */
    void setup();
    /**
     * 
     */
    void readSavedSettings();
    /**
     * 
     */
    void sortItems(std::function<bool(QListWidgetItem*, QListWidgetItem*)> compare);

    
    QSet<QListWidgetItem*> originalGamesSet_m;
    QList<QListWidgetItem*> originalGamesList_m;
    Controller& controller = Controller::getInstance();
    QSettings settings;
    Ui::TombRaiderLinuxLauncher *ui;
};
#endif  // SRC_TOMBRAIDERLINUXLAUNCHER_H_
