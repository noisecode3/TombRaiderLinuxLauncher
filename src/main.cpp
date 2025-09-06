/* TombRaiderLinuxLauncher
 * Martin Bångens Copyright (C) 2025
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <QtGlobal>
#include "../src/CommandLineParser.hpp"
#ifdef TEST
#include <QCoreApplication>
#include <QTest>
#include "../test/test.hpp"
#else
#include <QApplication>
#include "../src/TombRaiderLinuxLauncher.hpp"
#endif

#ifdef TEST
/**
 * The main function used for console tests
 */
int main(int argc, char *argv[]) {
    int status = 0;

    QCoreApplication app(argc, argv);

    CommandLineParser clp("APP");
    const StartupSetting ss = clp.process(app.arguments());

    status = clp.getProcessStatus();

    if (status == 0) {
        // Run the tests
        GameFileTreeTest test;
        status = QTest::qExec(&test, app.arguments());
    }

    return status;  // Exit after handling the custom flag
}
#else
/**
 * The main function used for the regular qt app.
 * Takes care of command line arguments and create the window.
 */
int main(int argc, char *argv[]) {
    int status = 1;
    if (argc > 100) {
        qDebug() << "Error: To many arguments, over 100";
    } else {
        QApplication app(argc, argv);
        QApplication::setOrganizationName("TombRaiderLinuxLauncher");
        QApplication::setApplicationName("TombRaiderLinuxLauncher");

        CommandLineParser clp("APP");
        const StartupSetting ss = clp.process(app.arguments());

        status = clp.getProcessStatus();
        if (status == 0) {
            // Construct the main window object
            TombRaiderLinuxLauncher w;

            if (w.setStartupSetting(ss)) {
                if (ss.fullscreen == true) {
                    w.showFullScreen();
                } else {
                    w.show();
                }

                status = app.exec();

            } else {
                qDebug() << "Error: Could not set startup arguments internally";
            }
        }
    }

    return status;
}
#endif
