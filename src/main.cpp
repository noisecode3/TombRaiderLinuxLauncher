#include "TombRaiderLinuxLauncher.h"

#include <QApplication>


/**
 * 
 */
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QApplication::setOrganizationName("TombRaiderLinuxLauncher");
    QApplication::setApplicationName("TombRaiderLinuxLauncher");

    // Construct the QSettings object
    TombRaiderLinuxLauncher w;

    QStringList arguments = a.arguments();
    if (arguments.contains("--fullscreen")) {
        w.showFullScreen();
    } else {
        w.show();
    }

    return a.exec();
}
