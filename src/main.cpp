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
 */

#ifdef TEST
#include <QCommandLineParser>
#include <QCoreApplication>
#include <QTest>
#include "binary.hpp"
#include "test.hpp"
#else
#include "TombRaiderLinuxLauncher.hpp"
#include <QApplication>
#endif

#ifdef TEST
/**
 * 
 */
int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);

    // Access the existing QTest command-line parser
    QCommandLineParser parser;
    parser.setApplicationDescription("Tomb Raider Linux Launcher Test Suite");
    parser.addHelpOption();

    // Add custom -w option for widescreen
    parser.addOption(QCommandLineOption(
        QStringList {"w", "widescreen"},
        "Set widescreen bit on original games, probably not useful for TRLE",
        "PATH"));

    // Process arguments
    parser.process(app);

    // Handle custom -w flag
    if (parser.isSet("widescreen")  == true) {
        widescreen_set(parser.value("widescreen"));
        return 0;  // Exit after handling the custom flag
    }

    // Pass remaining arguments to QTest
    TestTombRaiderLinuxLauncher test;
    QStringList testArgs = app.arguments();
    return QTest::qExec(&test, testArgs);
}
#else
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
    if (arguments.contains("--fullscreen") == true) {
        w.showFullScreen();
    } else {
        w.show();
    }
    return a.exec();
}
#endif
