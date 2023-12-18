#ifndef TOMBRAIDERPWMODDER_H
#define TOMBRAIDERPWMODDER_H

#include <QMainWindow>
#include <unistd.h>
#include "leveldata.h"

QT_BEGIN_NAMESPACE
namespace Ui { class TombRaiderPWModder; }
QT_END_NAMESPACE

class TombRaiderPWModder : public QMainWindow
{
    Q_OBJECT
public:
    TombRaiderPWModder(QWidget *parent = nullptr);
    ~TombRaiderPWModder();

public slots:
    void linkClicked();

private:
    void setup();
    void readNextCstring(int &index, char* cString, char* path);
    Ui::TombRaiderPWModder *ui;
};
#endif // TOMBRAIDERPWMODDER_H
