#ifndef TOMBRAIDERPWMODDER_H
#define TOMBRAIDERPWMODDER_H

#include <QMainWindow>
#include <QSettings>
#include "controller.h"
#include "worker.h"

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
     * 
     */
    TombRaiderLinuxLauncher(QWidget *parent = nullptr);
    /**
     * 
     */
    ~TombRaiderLinuxLauncher();

public slots:
    /**
     * 
     */
    void linkClicked();
    /**
     * 
     */
    void downloadClicked();
    /**
     * 
     */
    void setOptionsClicked();
    /**
     * 
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
     * 0 Game dose not exist.
     * 1 The path is a symbolic link.
     * 2 The path is not a symbolic link.
     * 3 value 3:The path is not a directory.
     */
    int checkGameDirectory(int game);
    Pool poolData;
    QString selected;
    QSettings settings;
    Ui::TombRaiderLinuxLauncher *ui;
    enum TRVER {TR1=1,TR2,TR3,TR4,TR5};
};
#endif // TOMBRAIDERPWMODDER_H
