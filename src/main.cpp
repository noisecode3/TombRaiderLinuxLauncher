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

#ifdef TEST
#include <QCommandLineParser>
#include <QCoreApplication>
#include <QTest>
#include "../src/binary.hpp"
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
    QCoreApplication app(argc, argv);
    qint64 status = 0;

    // Access the existing QTest command-line parser
    QCommandLineParser parser;
    parser.setApplicationDescription("Tomb Raider Linux Launcher Test Suite");
    parser.addHelpOption();

    // Add custom -w option for widescreen
    parser.addOption(QCommandLineOption(
        QStringList {"w", "widescreen"},
        "Set widescreen bit on original games, probably not useful for TRLE",
        "PATH"));

    // Add custom -b option for binary detection
    parser.addOption(QCommandLineOption(
        QStringList {"b", "binary"},
        "Print PE Header Information, to record Tomb Raider and TRLE binaries",
        "PATH"));

    // Process arguments
    parser.process(app);

    // Handle custom -w flag
    if (parser.isSet("widescreen")  == true) {
        status = widescreen_set(parser.value("widescreen"));
    } else if (parser.isSet("binary")  == true) {
        readPEHeader(parser.value("binary"));
        readExportTable(parser.value("binary"));
        analyzeImportTable(parser.value("binary").toStdString());
    } else {
        // Pass remaining arguments to QTest
        PyRunnerTest test;
        QStringList testArgs = app.arguments();
        status =  QTest::qExec(&test, testArgs);
    }
    return status;  // Exit after handling the custom flag
}
#else
/**
 * The main function used for the regular qt app.
 * Takes care of command line arguments and create the window.
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
