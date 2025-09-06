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
// #include "../src/GameFileTree.hpp"
// #include "../src/gameFileTreeData.hpp"
#include "../src/Path.hpp"
#include "../src/PyRunner.hpp"
#include "../src/Model.hpp"

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
    PyRunner pyRunner = PyRunner();
};


class GameFileTreeTest : public QObject {
    Q_OBJECT

 public:
    GameFileTreeTest() {
    }
    ~GameFileTreeTest() {
    }

 private slots:
    void ListTest() {
        Path::setTestProgramFilesPath();
        Path::setTestResourcePath();
        Path level(Path::resource);
        Path game(Path::programFiles);
        QVERIFY(model.setupDirectories(level.get(), game.get()));
        Path testLevel(Path::resource);
        quint64 id = 3713;
        testLevel << QString("%1.TRLE").arg(id);
        if (testLevel.isDir()) {
            qDebug() << "Exists: " << testLevel.get();
        } else {
            model.getLevel(id);
            qDebug() << "Does not exist: " << testLevel.get();
        }
        Path testLevelBefore = testLevel;
        testLevel = fileManager.getExtraPathToExe(testLevel);
        QVERIFY(testLevelBefore.get() ==  testLevel.get());
    }

    void test2() {
        QVERIFY(1 + 1 == 2);
    }

 private:
    Model& model = Model::getInstance();
    FileManager& fileManager = FileManager::getInstance();
};
#endif  // TEST_TEST_HPP_
