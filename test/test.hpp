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

class TestTombRaiderLinuxLauncher : public QObject {
    Q_OBJECT

 private slots:
    void test1() {
        QVERIFY(true);
    }

    void test2() {
        QVERIFY(1 + 1 == 2);
    }
};

#endif  // TEST_TEST_HPP_
