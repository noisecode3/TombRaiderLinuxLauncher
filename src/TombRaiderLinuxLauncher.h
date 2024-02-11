#ifndef TOMBRAIDERPWMODDER_H
#define TOMBRAIDERPWMODDER_H

#include <QMainWindow>
#include <QSettings>
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
    TombRaiderLinuxLauncher(QWidget *parent = nullptr);
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
    void checkCommonFiles();
    /**
     * 
     */
    void generateList();
    /**
     *
     */
    int testallGames(int id);

    Controller& controller = Controller::getInstance();
    QSettings settings;
    Ui::TombRaiderLinuxLauncher *ui;
};
#endif // TOMBRAIDERPWMODDER_H
