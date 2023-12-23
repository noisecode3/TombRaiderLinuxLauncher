#include "tombraiderpwmodder.h"

#include <QApplication>


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QApplication::setOrganizationName("TombRaiderLinuxLauncher");
    QApplication::setApplicationName("TombRaiderLinuxLauncher");

    // Construct the QSettings object
    TombRaiderLinuxLauncher w;
    w.show();
    return a.exec();
}
