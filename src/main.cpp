/* TombRaiderLinuxLauncher
 * Martin Bångens Copyright (C) 2024
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifdef TEST
#include <QtTest/QtTest>
#include "binary.h"
#include "test.h"

/**
 * 
 */
int main(int argc, char *argv[]) {
    if (argc == 3 && strcmp(argv[1], "-w") == 0) {
        widescreen_set(argv[2]);
    } else {
        TestTombRaiderLinuxLauncher test;
        return QTest::qExec(&test, argc, argv);
    }
}
#else
#include "TombRaiderLinuxLauncher.h"
#include <QApplication>

/**
 * 
 */
int main(int argc, char *argv[]) {
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
#endif
