#include "tombraiderpwmodder.h"
#include "ui_tombraiderpwmodder.h"

TombRaiderPWModder::TombRaiderPWModder(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::TombRaiderPWModder)
{
    ui->setupUi(this);
}

TombRaiderPWModder::~TombRaiderPWModder()
{
    delete ui;
}

