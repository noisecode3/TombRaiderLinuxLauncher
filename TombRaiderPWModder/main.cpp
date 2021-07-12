#include "tombraiderpwmodder.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    TombRaiderPWModder w;
    w.show();
    return a.exec();
}
