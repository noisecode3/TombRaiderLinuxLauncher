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

#ifndef TEST_TEST_HPP_
#define TEST_TEST_HPP_

#include <QtCore>
#include <QtTest/QtTest>
#include "../src/PyRunner.hpp"

class PyRunnerTest : public QObject {
    Q_OBJECT

 private slots:
    void ListTest() {
        // QStringList arguments;
        // arguments << "tombll_manage_data.py" << "-l";
        // pyrunner.run(arguments);
        // QVERIFY(pyrunner.getStatus() == 0);
        // qDebug() << "\npyrunner.getStatus(): " << pyrunner.getStatus();
    }

    void test2() {
        QVERIFY(1 + 1 == 2);
    }
 private:
    PyRunner pyrunner =
        PyRunner("tester_root/usr/share/TombRaiderLinuxLauncher");
};

#endif  // TEST_TEST_HPP_
