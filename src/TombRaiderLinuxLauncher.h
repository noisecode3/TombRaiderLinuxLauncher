#ifndef TOMBRAIDERPWMODDER_H
#define TOMBRAIDERPWMODDER_H

#include <QMainWindow>
#include <QSettings>

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
    void downloadClicked();
    void setOptionsClicked();

private:
    void setup();
    void readSavedSettings();
    void checkCommonFiles();
    void generateList();

    int checkGameDirectory(int game);
    QString selected;
    QSettings settings;
    Ui::TombRaiderLinuxLauncher *ui;
    enum TRVER {TR1=1,TR2,TR3,TR4,TR5};
};
#endif // TOMBRAIDERPWMODDER_H
