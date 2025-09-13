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

class GameFileTreeTest : public QObject {
    Q_OBJECT

 public:
    GameFileTreeTest() : faildLevels({3713, 2218, 3347, 2922})
    {}
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

        for (int i = 0; i < 30; ++i) {
            quint64 id = distr(gen);
            //  quint64 id = faildLevels.at(i);
            Path testLevel(Path::resource);
            testLevel << QString("%1.TRLE").arg(id);
            model.getLevel(id);
            if (testLevel.isDir()) {
                qDebug() << "Exists: " << testLevel.get();
                Path testLevelExtraToExe = testLevel;
                quint64 type = model.getType(id);
                fileManager.getExtraPathToExe(testLevelExtraToExe, type);
                testLevelExtraToExe << model.getExecutableName(type);
                qDebug() << "testLevelExtraToExe: " << testLevelExtraToExe.get();
                QVERIFY(testLevelExtraToExe.exists());
                model.deleteZip(id);
                fileManager.removeFileOrDirectory(testLevel);
            } else {
                qDebug() << "Does not exist: " << testLevel.get();
            }
        }
    }

 private:
    const QList<quint64> faildLevels;
    Model& model = Model::getInstance();
    FileManager& fileManager = FileManager::getInstance();
};
#endif  // TEST_TEST_HPP_
