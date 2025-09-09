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

#include <random>
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

/*  Extra path to executable directory: Part 1 - Prologue - Lara's Home/Part 2 - Treasure Island/Part 3 - Into The Depths/Part 4 - Penglai Shan/Part 5 - The Shard/Part 6 - Mexico
 *  FAIL!  : GameFileTreeTest::ListTest() 'testLevelExtraToExe.exists()' returned FALSE. ()
 *  In Mexico for 2922 XD haha
 */

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
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> distr(1, 3718);

        for (int i = 0; i < 10; ++i) {
            //  id numbers that have faild...
            //  we need at better way to do this, put them in a QList?
            //  quint64 id = 2217;
            //  quint64 id = 3347;
            quint64 id = distr(gen);
            Path testLevel(Path::resource);
            testLevel << QString("%1.TRLE").arg(id);
            model.getLevel(id);
            if (testLevel.isDir()) {
                qDebug() << "Exists: " << testLevel.get();
                Path testLevelExtraToExe = testLevel;
                quint64 type = model.getType(id);
                fileManager.getExtraPathToExe(testLevel, type);
                testLevelExtraToExe << model.getExecutableName(type);
                QVERIFY(testLevelExtraToExe.exists());
                model.deleteZip(id);
                fileManager.removeFileOrDirectory(testLevel);
            } else {
                qDebug() << "Does not exist: " << testLevel.get();
            }
        }
    }

 private:
    Model& model = Model::getInstance();
    FileManager& fileManager = FileManager::getInstance();
};
#endif  // TEST_TEST_HPP_
