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
 */

#ifndef SRC_TOMBRAIDERLINUXLAUNCHER_HPP_
#define SRC_TOMBRAIDERLINUXLAUNCHER_HPP_

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

#include "Controller.hpp"

QT_BEGIN_NAMESPACE
namespace Ui { class TombRaiderLinuxLauncher; }
QT_END_NAMESPACE

/**
 * View component in the MVC pattern; Main UI class for the launcher.
 */
class TombRaiderLinuxLauncher : public QMainWindow {
    Q_OBJECT

 public:
    /**
     * Constructs the main window.
     */
    explicit TombRaiderLinuxLauncher(QWidget *parent = nullptr);
    ~TombRaiderLinuxLauncher();

 public slots:
    /**
     * Triggered by the "Link" button.
     */
    void linkClicked();
    /**
     * Triggered by the "Download" button.
     */
    void downloadClicked();
    /**
     * Opens the Info thru the navigation bar.
     */
    void infoClicked();
    /**
     * Opens the Walkthrough thru the navigation bar.
     */
    void walkthroughClicked();
    /**
     * Returns to the first navigation state, the list.
     */
    void backClicked();
    /**
     * Opens the first-time setup options.
     */
    void setOptionsClicked();
    /**
     * Updates button states based on selected menu level.
     */
    void onListItemSelected();
    /**
     * Updates progress by 1% of total work steps.
     */
    void workTick();
    /**
     * Displays an error dialog for a curl download error.
     */
    void downloadError(int status);
    /**
     * Generates the initial level list after file analysis.
     */
    void generateList(const QList<int>& availableGames);
    /**
     * Sorts the list by author.
     */
    void sortByAuthor();
    /**
     * Sorts the list by title.
     */
    void sortByTitle();
    /**
     * Sorts the list by difficulty.
     */
    void sortByDifficulty();
    /**
     * Sorts the list by duration.
     */
    void sortByDuration();
    /**
     * Sorts the list by level class.
     */
    void sortByClass();
    /**
     * Sorts the list by type.
     */
    void sortByType();
    /**
     * Sorts the list by release date.
     */
    void sortByReleaseDate();

 private:
    /**
     * 
     */
    void originalSelected(QListWidgetItem *selectedItem);
    /**
     * 
     */
    void levelDirSelected(QListWidgetItem *selectedItem);
    /**
     * Configures game and level directories.
     */
    void setup();
    /**
     * Loads saved settings.
     */
    void readSavedSettings();
    /**
     * Executes the sorting algorithm.
     */
    void sortItems(std::function<bool(
        QListWidgetItem*,
        QListWidgetItem*)> compare);

    QSet<QListWidgetItem*> originalGamesSet_m;
    QList<QListWidgetItem*> originalGamesList_m;
    Controller& controller = Controller::getInstance();
    QSettings m_settings;
    Ui::TombRaiderLinuxLauncher *ui;
};

struct OriginalGameData {
    QMap<int, QString> romanNumerals = {
            {0, "null"},
            {1, "I"},
            {2, "II"},
            {3, "III"},
            {4, "IV"},
            {5, "V"},
            {6, "VI"},
            {7, "IUB"},
            {8, "IIGM"},
            {9, "IIILM"},
    };
    const QString getPicture(int id) {
        return QString ("Tomb_Raider_%1.jpg").arg(romanNumerals[id]);
    }
};

#endif  // SRC_TOMBRAIDERLINUXLAUNCHER_HPP_
