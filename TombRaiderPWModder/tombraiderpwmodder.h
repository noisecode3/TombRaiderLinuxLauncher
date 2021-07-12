#ifndef TOMBRAIDERPWMODDER_H
#define TOMBRAIDERPWMODDER_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class TombRaiderPWModder; }
QT_END_NAMESPACE

class TombRaiderPWModder : public QMainWindow
{
    Q_OBJECT

public:
    TombRaiderPWModder(QWidget *parent = nullptr);
    ~TombRaiderPWModder();

private:
    Ui::TombRaiderPWModder *ui;
};
#endif // TOMBRAIDERPWMODDER_H
