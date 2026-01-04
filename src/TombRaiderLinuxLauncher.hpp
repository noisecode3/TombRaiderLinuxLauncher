/* TombRaiderLinuxLauncher
 * Martin Bångens Copyright (C) 2025
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef SRC_TOMBRAIDERLINUXLAUNCHER_HPP_
#define SRC_TOMBRAIDERLINUXLAUNCHER_HPP_

#include <QMainWindow>
#include <QSettings>
#include <QMessageBox>
#include <QScrollArea>
#include <QScrollBar>
#include <QVBoxLayout>
#include <QStringList>
#include <QSet>
#include <QDateTime>
#include <QDebug>
#include <QVector>
#include <QString>

#include "../src/view/Ui.hpp"
#include "../src/CommandLineParser.hpp"

//QT_BEGIN_NAMESPACE
// namespace Ui { class TombRaiderLinuxLauncher; }
//QT_END_NAMESPACE

/** @mainpage TombRaiderLinuxLauncher
 *
 * @section intro
 *
 */

/**
 * @class TombRaiderLinuxLauncher
 * @brief View component in the MVC pattern; Main Window UI class.
 *
 *
 *
 */
class TombRaiderLinuxLauncher : public QMainWindow {
    Q_OBJECT

 public:
    /**
     * Constructs the main window.
     */
    explicit TombRaiderLinuxLauncher(QWidget *parent = nullptr);
    ~TombRaiderLinuxLauncher();

    void setStartupSetting(const StartupSetting settings);

 public slots:
    /**
     * Triggered by the "Run" button.
     */
    // void runClicked();

    /**
     * Triggered by the "Download" or "Remove" button.
     */
    // void downloadOrRemoveClicked();

    /**
     * Opens the Info thru the navigation bar.
     */
    // void infoClicked();

    /**
     * Opens the Walkthrough thru the navigation bar.
     */
    // void walkthroughClicked();

    /**
     * Returns to the first navigation state, the list.
     */
    // void backClicked();

    /**
     * Opens the first-time setup options clicked.
     */
    // void setOptionsClicked();

    /**
     * Updates button states based on selected menu level.
     */
    // void onCurrentItemChanged(
    //         const QModelIndex &current, const QModelIndex &previous);

    /**
     *  Try loading 100 more levels cards by calling for more cover pictures.
     */
    // void loadMoreCovers();

    /**
     * Updates progress by 1% of total work steps.
     */
    // void workTick();

    /**
     * Displays an error dialog for a curl download error.
     */
    // void downloadError(int status);

    /**
     * Switch back to level list or info page after network/python work.
     */
    // void updateLevelDone();

    /**
     * Switch back to list selection state after running the game.
     */
    // void runningLevelDone();

    /**
     * Generates the initial level list after file analysis.
     */
    // void generateList(const QList<int>& availableGames);

    /**
     * Sorts the list by title.
     */
    // void sortByTitle();

    /**
     * Sorts the list by difficulty.
     */
    // void sortByDifficulty();

    /**
     * Sorts the list by duration.
     */
    // void sortByDuration();

    /**
     * Sorts the list by level class.
     */
    // void sortByClass();

    /**
     * Sorts the list by type.
     */
    // void sortByType();

    /**
     * Sorts the list by release date.
     */
    // void sortByReleaseDate();


 private:
    /**
     * 
     */
    // void setList();


    /**
     * 
     */
    // void levelDirSelected(qint64 id);

    /**
     * Configures game and level directories.
     */
    // void setup();

    /**
     * Loads saved settings.
     */
    // void readSavedSettings();

    /**
     * Pars input string to string list.
     */
    // QStringList parsToArg(const QString& str);

    /**
     * Pars input string to list of environment variables.
     */
    // QVector<QPair<QString, QString>> parsToEnv(const QString& str);

    // Controller& controller = Controller::getInstance();
    // QSettings& g_settings = getSettingsInstance();
    StartupSetting m_ss;
    Ui* ui;
};

#endif  // SRC_TOMBRAIDERLINUXLAUNCHER_HPP_
