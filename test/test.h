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

#ifndef TEST_TEST_H_
#define TEST_TEST_H_

// This is some test for another application that can work as an example
// it more nice to test individual parts of the application without the GUI

#include <QtCore>
#include <QtTest/QtTest>

#include "SimpleZipper.h"

/**
 * @class   TestSimpleZipper
 *
 * @brief   Unit tests for SimpleZipper class.
 *
 * @details The TestSimpleZipper class implements unit tests for the static
 *          methods in the SimpleZipper class.
 *          It creates three text files and a subdirectory with two text files,
 *          and zips and unzips these by file and folder,
 *          and checks the file contents survive the zip-unzip.
 */
class TestSimpleZipper : public QObject {
    Q_OBJECT

 private:
    QDir mTempDir;
    QFile mFile1;
    QFile mFile2;
    QFile mFile3;
    QDir mSubDir;
    QFile mSubFile1;
    QFile mSubFile2;

 private slots:
    /**
     * @brief Creates a temporary directory with three text files and a subdirectory with two text files.
     */
    void initTestCase() {
        // Create a temporary directory
        mTempDir = QDir(QDir::tempPath() + "/TestSimpleZipper");
        QVERIFY(mTempDir.mkpath("."));

        // Create three text files
        mFile1.setFileName(mTempDir.filePath("mFile1.txt"));
        QVERIFY(mFile1.open(QIODevice::WriteOnly));
        mFile1.write("Hello World!");
        mFile1.close();

        mFile2.setFileName(mTempDir.filePath("mFile2.txt"));
        QVERIFY(mFile2.open(QIODevice::WriteOnly));
        mFile2.write("Goodbye World!");
        mFile2.close();

        mFile3.setFileName(mTempDir.filePath("mFile3.txt"));
        QVERIFY(mFile3.open(QIODevice::WriteOnly));
        mFile3.write("Lorem Ipsum.");
        mFile3.close();

        // Create a subdirectory with two text files
        mSubDir = QDir(mTempDir.filePath("subdir"));
        QVERIFY(mSubDir.mkpath("."));

        mSubFile1.setFileName(mSubDir.filePath("mSubFile1.txt"));
        QVERIFY(mSubFile1.open(QIODevice::WriteOnly));
        mSubFile1.write("Sub File 1");
        mSubFile1.close();

        mSubFile2.setFileName(mSubDir.filePath("mSubFile2.txt"));
        QVERIFY(mSubFile2.open(QIODevice::WriteOnly));
        mSubFile2.write("Sub File 2");
        mSubFile2.close();
    }

    /**
     * @brief Zips and unzips the temporary directory and checks the files are the same.
     */
    void testZipDirectory()
    {
        // Name for zip file
        QDir parentDir = mTempDir;
        parentDir.cdUp();
        QString zipPath = parentDir.filePath("testZipper.zip");

        // Zip the temporary directory
        QVERIFY(SimpleZipper::zipFolder(mTempDir.absolutePath(), zipPath));

        // Check the file exists
        QVERIFY(QFile::exists(zipPath));

        // Directory for unzipping
        QString tempUnzipDir = mTempDir.filePath("testZipperUnzip");

        // Unzip the zip file to the temporary directory
        QVERIFY(SimpleZipper::unzipFile(zipPath, tempUnzipDir));

        // Check the contents of the unzipped files are the same
        // as the original files
        QFile unzippedFile1(tempUnzipDir + "/mFile1.txt");
        QFile unzippedFile2(tempUnzipDir + "/mFile2.txt");
        QFile unzippedFile3(tempUnzipDir + "/mFile3.txt");
        QFile unzippedSubFile1(tempUnzipDir + "/subdir/mSubFile1.txt");
        QFile unzippedSubFile2(tempUnzipDir + "/subdir/mSubFile2.txt");

        QVERIFY(unzippedFile1.open(QIODevice::ReadOnly));
        QVERIFY(unzippedFile2.open(QIODevice::ReadOnly));
        QVERIFY(unzippedFile3.open(QIODevice::ReadOnly));
        QVERIFY(unzippedSubFile1.open(QIODevice::ReadOnly));
        QVERIFY(unzippedSubFile2.open(QIODevice::ReadOnly));

        QVERIFY(mFile1.open(QIODevice::ReadOnly));
        QVERIFY(mFile2.open(QIODevice::ReadOnly));
        QVERIFY(mFile3.open(QIODevice::ReadOnly));
        QVERIFY(mSubFile1.open(QIODevice::ReadOnly));
        QVERIFY(mSubFile2.open(QIODevice::ReadOnly));

        QCOMPARE(unzippedFile1.readAll(), mFile1.readAll());
        QCOMPARE(unzippedFile2.readAll(), mFile2.readAll());
        QCOMPARE(unzippedFile3.readAll(), mFile3.readAll());
        QCOMPARE(unzippedSubFile1.readAll(), mSubFile1.readAll());
        QCOMPARE(unzippedSubFile2.readAll(), mSubFile2.readAll());

        // Close original files
        mFile1.close();
        mFile2.close();
        mFile3.close();
        mSubFile1.close();
        mSubFile2.close();
    }

    /**
     * @brief Zips and unzips a single text file and check the output is the same.
     */
    void testZipFiles()
    {
        QFileInfo fileInfo(mFile1.fileName());
        QString zipFileName = fileInfo.absoluteDir().path() + "/" +
            fileInfo.baseName() + ".zip";

        QString unzipFileName = fileInfo.absoluteDir().path() + "/" +
            fileInfo.baseName() + "/" + fileInfo.fileName();

        // Zip the file
        QVERIFY(SimpleZipper::zipFile(mFile1.fileName()));

        // Check the file exists
        QVERIFY(QFile::exists(zipFileName));

        // Unzip the file
        QVERIFY(SimpleZipper::unzipFile(
            zipFileName,
            fileInfo.absoluteDir().path() + "/" + fileInfo.baseName()));

        // Check the unzipped file exists
        QVERIFY(QFile::exists(unzipFileName));

        // Compare the unzipped file to the original file
        QFile unzippedFile(unzipFileName);
        QVERIFY(unzippedFile.open(QIODevice::ReadOnly));
        QVERIFY(mFile1.open(QIODevice::ReadOnly));
        QCOMPARE(unzippedFile.readAll(), mFile1.readAll());

        // Close original file
        mFile1.close();
    }

    /**
     * @brief Deletes the temporary directory and all files created in it.
     */
    void cleanupTestCase() {
        QVERIFY(mTempDir.removeRecursively());
    }
};

QTEST_MAIN(TestSimpleZipper)

#endif  // TEST_TEST_H_
