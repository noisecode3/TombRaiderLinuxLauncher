#ifndef TOMBRAIDERPWMODDER_H
#define TOMBRAIDERPWMODDER_H

#include <QMainWindow>
#include <QSettings>
#include "leveldata.h"

QT_BEGIN_NAMESPACE
namespace Ui { class TombRaiderLinuxLauncher; }
QT_END_NAMESPACE

class TombRaiderLinuxLauncher : public QMainWindow
{
    Q_OBJECT
public:
    TombRaiderLinuxLauncher(QWidget *parent = nullptr);
    ~TombRaiderLinuxLauncher();

public slots:
    void linkClicked();
    void setOptionsClicked();

private:
    void setup();
    void readSavedSettings();
    void checkCommonFiles();
    void generateList();

    const QString calculateMD5(const QString &filename);
    int checkGameDirectory(int game);
    void packOriginalGame(int game);
    QString selected;
    QSettings settings;
    Ui::TombRaiderLinuxLauncher *ui;
    enum TRVER {TR1=1,TR2,TR3,TR4,TR5};
    struct folderNames
    {
        QString TR1 = "/Tomb Raider (I)";
        QString TR2 = "/Tomb Raider (II)";
        QString TR3 = "/TombRaider (III)";
        QString TR4 = "/Tomb Raider (IV) The Last Revelation";
        QString TR5 = "/Tomb Raider (V) Chronicles";
    };
};
#endif // TOMBRAIDERPWMODDER_H
