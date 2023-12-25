#include <QMessageBox>
#include <QStandardPaths>
#include <QDirIterator>
#include <QDir>
#include <QFileInfo>
#include <QFile>
#include <QCryptographicHash>
#include <QStringList>
#include <QDebug>
#include "TombRaiderLinuxLauncher.h"
#include "ui_TombRaiderLinuxLauncher.h"
#include "originalFileList.h"

#include <QDir>
#include "quazip/quazip.h"
#include "quazip/quazipfile.h"
#include <iostream>

#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cstdio>
#include <filesystem>
#include <cstring>
#include <openssl/md5.h>
#include <openssl/evp.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <curl/curl.h> //depend
#include <zip.h> //depend
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <curl/curl.h>

/*
 *
 *
 *
 *
 */
TombRaiderLinuxLauncher::TombRaiderLinuxLauncher(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::TombRaiderLinuxLauncher)
{
    ui->setupUi(this);
    connect(ui->pushButtonLink, SIGNAL (clicked()), this, SLOT (linkClicked()));
    connect(ui->setOptions, SIGNAL (clicked()), this, SLOT (setOptionsClicked()));

    pool games;
    games.setName("Jonson-TheInfadaCult.TR3");
    games.setFileList(TRLE3573FileList);
    games.setData(TRLE3573);

    // Read settings
    QString value = settings.value("setup").toString();
    //settings.setValue("setup" , "no");
    if (value != "yes")
        setup();
    else
        readSavedSettings();

}
/*
 * 0 Game dose not exist.
 * 1 The path is a symbolic link.
 * 2 The path is not a symbolic link.
 * 3 value 3:The path is not a directory.
 */
int TombRaiderLinuxLauncher::checkGameDirectory(int game)
{
    struct folderNames folder;
    QString name;
    switch (game)
    {
    case TR1:
        name = folder.TR1;
        break;
    case TR2:
        name = folder.TR2;
        break;
    case TR3:
        name = folder.TR3;
        break;
    case TR4:
        name = folder.TR4;
        break;
    case TR5:
        name = folder.TR5;
        break;
    default:
        qDebug() << game << " is not a game.";
        return 0;
        break;
    }
    QString filePath = settings.value("gamePath").toString() + name;
    QFileInfo fileInfo(filePath);
    if (fileInfo.isDir())
    {
        qDebug() << "The path is a directory.";
        if (fileInfo.isSymLink())
        {
            qDebug() << "return value 1:The path is a symbolic link.";
            return 1;
        }
        else
        {
            qDebug() << "return value 2:The path is not a symbolic link.";
            return 2;
        }
    }
    else
    {
        qDebug() << "value 3:The path is not a directory.";
        qDebug() << "filePath " << filePath;
        return 3;
    }
}
/*
 *
 *
 *
 *
 */
void TombRaiderLinuxLauncher::packOriginalGame(int game)
{
    TR3FileListWork();
    struct folderNames folder;
    const QString folderPath = settings.value("gamePath").toString() + folder.TR3;
    const size_t s = TR3FileList.getSize();

    QString directoryPath = settings.value("levelPath").toString() + "/Original.TR3";

    // Create the directory if it doesn't exist
    if (!QDir(directoryPath).exists()) {
        if (QDir().mkpath(directoryPath)) {
            qDebug() << "Directory created successfully.";
        } else {
            qDebug() << "Error creating directory.";
            return;
        }
    } else {
        qDebug() << "Directory already exists.";
    }

    for (size_t i = 0; i < s; i++)
    {
        file f(TR3FileList.getFile(i));
        const QString sourcePath = folderPath + "/" + f.getPath();
        const QString destinationPath = directoryPath + "/" + f.getPath();

        if(f.getMd5sum() == calculateMD5(sourcePath))
        {
            // Ensure the destination directory exists
            const QFileInfo destinationFileInfo(destinationPath);
            QDir destinationDir(destinationFileInfo.absolutePath());
            if (!destinationDir.exists()) {
                if (!QDir().mkpath(destinationDir.absolutePath())) {
                    qDebug() << "Error creating destination directory.";
                    return;
                }
            }

            // Copy the file
            if (QFile::copy(sourcePath, destinationPath))
            {
                qDebug() << "File copy successfully.";
            }
            else
            {
                if(QFile::exists(destinationPath))
                {
                    if(f.getMd5sum() == calculateMD5(destinationPath))
                        qDebug() << "File exist";
                }
                else
                {
                    qDebug() << "Failed to copy file " << f.getPath() << Qt::endl;
                    return;
                }
            }
        }
        else {
            qDebug() << "Original file was modified" << Qt::endl;
            return;
        }
    }
    QDir directory(folderPath);

    if (directory.exists()) {
        // Remove the directory and its contents recursively
        if (directory.removeRecursively()) {
            qDebug() << "Directory removed successfully.";
        } else {
            qDebug() << "Error removing directory.";
        }
    } else {
        qDebug() << "Directory does not exist.";
    }

    // Create a symbolic link
    if (QFile::link(directoryPath, folderPath))
    {
        qDebug() << "Symbolic link created successfully.";
    }
    else
    {
        qDebug() << "Failed to create symbolic link.";
    }

}
void TombRaiderLinuxLauncher::checkCommonFiles()
{
    int dirStaus = checkGameDirectory(TR3); // we only setup tr3 for now
    if (dirStaus)
    {
        if(dirStaus == 1)// The path is a symbolic link.
        {
            //its in use
            // pass for now
        }
        else if(dirStaus == 2)// The path is not a symbolic link.
        {
            //this means we backup the game to levelPath IF that is whats inside
            QMessageBox msgBox;
            msgBox.setWindowTitle("Confirmation");
            msgBox.setText("TombRaider III found, you want to proceed?\nThis will erase saved games and is only for testing");
            msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
            msgBox.setDefaultButton(QMessageBox::No);
            int result = msgBox.exec();
            if (result == QMessageBox::Yes) {
                qDebug() << "User clicked Yes.";
                packOriginalGame(3);
            } else {
                qDebug() << "User clicked No or closed the dialog.";
            }
        }
        else if(dirStaus == 3)// The path is not a directory.
        {
            //it has been replaced by a file from somewhere else or the user
        }
        generateList();
    }
}
/*
 *
 *
 *
 *
 */


void TombRaiderLinuxLauncher::generateList()
{
    ui->listWidgetModds->setIconSize(QSize(320, 240));
    QFont defaultFont = QApplication::font();
    int newSize = 22;
    defaultFont.setPointSize(newSize);
    ui->listWidgetModds->setFont(defaultFont);
    QString directoryPath = settings.value("levelPath").toString();
    QString pictures = directoryPath + "/pictures/";
    QDir info(directoryPath);
    QFileInfoList enteryInfoList = info.entryInfoList();

    foreach (const QFileInfo &file, enteryInfoList)
    {
        QString ending = file.fileName().right(3);
        if (ending == "TR3")
        {
            qDebug() << "The last three characters are 'TR3'.";
                if(file.fileName() == "Original.TR3")
                {
                    QListWidgetItem *wi = new QListWidgetItem(QIcon(pictures+"Tomb_Raider_III.jpg"),"Tomb Raider III Original");
                    ui->listWidgetModds->addItem(wi);
                }
                else if(file.fileName() == "Jonson-TheInfadaCult.TR3")
                {
                    QListWidgetItem *wi = new QListWidgetItem(QIcon(pictures+"trle.net/3573.jpg"),"The Infada Cult\nby Jonson");
                    ui->listWidgetModds->addItem(wi);
                }
                else
                {
                    QListWidgetItem *wi = new QListWidgetItem(file.fileName());
                    ui->listWidgetModds->addItem(wi);
                }
        }
        else
        {
            qDebug() << "The last three characters are not 'TR3'.";
        }

    }
}
/*
 *
 *
 *
 *
 */
void TombRaiderLinuxLauncher::readSavedSettings()
{
    // Read a setting
    QString gamePathValue = settings.value("gamePath").toString();
    qDebug() << "Read game path value:" << gamePathValue;
    QString levelPathValue = settings.value("levelPath").toString();
    qDebug() << "Read level path value:" << levelPathValue;
    checkCommonFiles();
}
/*
 *
 *
 *
 *
 */
void TombRaiderLinuxLauncher::setup()
{
    ui->Tabs->setCurrentIndex(ui->Tabs->indexOf(ui->Tabs->findChild<QWidget*>("Setup")));
    ui->setupStackedWidget->setCurrentWidget(ui->setupStackedWidget->findChild<QWidget*>("firstTime"));
    ui->Tabs->setTabEnabled(ui->Tabs->indexOf(ui->Tabs->findChild<QWidget*>("Levels")), false);
    ui->Tabs->setTabEnabled(ui->Tabs->indexOf(ui->Tabs->findChild<QWidget*>("Modding")), false);

    qDebug() << "Entering setup" << Qt::endl;
    QString homeDir = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
    qDebug() << "Home Directory:" << homeDir;
    QString s = "/.steam/root/steamapps/common/";
    QString l = "/.local/share/TombRaiderLinuxLauncher";
    ui->gamePathEdit->setText(homeDir + s);
    ui->levelPathEdit->setText(homeDir + l);
}
/*
 *
 *
 *
 *
 */
void TombRaiderLinuxLauncher::setOptionsClicked()
{
    QString gamePath = ui->gamePathEdit->text();
    QString levelPath = ui->levelPathEdit->text();
    settings.setValue("gamePath" , gamePath);
    settings.setValue("levelPath" , levelPath);
    settings.setValue("setup" , "yes");

    ui->tableWidgetSetup->item(0,0)->setText(gamePath);
    ui->tableWidgetSetup->item(1,0)->setText(levelPath);
    ui->Tabs->setTabEnabled(ui->Tabs->indexOf(ui->Tabs->findChild<QWidget*>("Levels")), true);
    ui->Tabs->setTabEnabled(ui->Tabs->indexOf(ui->Tabs->findChild<QWidget*>("Modding")), true);
    ui->Tabs->show();
    ui->Tabs->setCurrentIndex(ui->Tabs->indexOf(ui->Tabs->findChild<QWidget*>("Levels")));
    ui->setupStackedWidget->setCurrentWidget(ui->setupStackedWidget->findChild<QWidget*>("settings"));

    readSavedSettings();
}
/*
 *
 *
 *
 *
 */
const QString TombRaiderLinuxLauncher::calculateMD5(const QString& filename)
{
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly))
    {
        qDebug() << "Error opening file for reading: " << file.errorString();
        return "";
    }

    QCryptographicHash md5(QCryptographicHash::Md5);

    char buffer[1024];
    qint64 bytesRead;

    while ((bytesRead = file.read(buffer, sizeof(buffer))) > 0)
    {
        md5.addData(buffer, static_cast<int>(bytesRead));
    }

    file.close();

    return QString(md5.result().toHex());
}

void TombRaiderLinuxLauncher::linkClicked()
{
    struct folderNames folder;
    const QString gamePath = settings.value("gamePath").toString() + folder.TR3;
    qDebug() << "Read game path value:" << gamePath;
    const QString levelPath = settings.value("levelPath").toString();
    qDebug() << "Read level path value:" << levelPath;
    QList<QListWidgetItem *> list = ui->listWidgetModds->selectedItems();
    if (!list.isEmpty())
    {
        // Create a symbolic link
        QString s = list.first()->text();
        if (s == "Tomb Raider III Original")
            s = "Original.TR3";
        else if (s == "The Infada Cult\nby Jonson")
            s = "Jonson-TheInfadaCult.TR3";
        const QString p = levelPath + "/" + s;
        if (QFile::link(p, gamePath))
        {
            qDebug() << "Symbolic link created successfully.";
        }
        else
        {
            if (QFile::exists(gamePath))
            {
                QFileInfo i(gamePath);
                if (i.isSymLink())
                {
                    QFile::remove(gamePath);
                    if (QFile::link(p, gamePath))
                        qDebug() << "Symbolic link created successfully.";
                    else
                        qDebug() << "Failed to create symbolic link.";
                }
                else
                    qDebug() << "Failed to create symbolic link.";
            }
            else
                qDebug() << "Failed to create symbolic link.";
        }
    QApplication::quit();
    }
}


size_t TombRaiderLinuxLauncher::write_data(void *ptr, size_t size, size_t nmemb, void *stream)
{
    size_t written = fwrite(ptr, size, nmemb, (FILE *)stream);
    return written;
}

size_t TombRaiderLinuxLauncher::downloadFile(const char* url, const char* filename)
{
    CURL *curl_handle;
    static const char *pagefilename = filename;
    FILE *pagefile;
    if (filename[0] == '/')
    {
        curl_global_init(CURL_GLOBAL_ALL);

        /* init the curl session */
        curl_handle = curl_easy_init();

        /* set URL to get here */
        curl_easy_setopt(curl_handle, CURLOPT_URL, url);

        /* Switch on full protocol/debug output while testing */
        curl_easy_setopt(curl_handle, CURLOPT_VERBOSE, 1L);

        /* disable progress meter, set to 0L to enable it */
        curl_easy_setopt(curl_handle, CURLOPT_NOPROGRESS, 1L);

        /* send all data to this function  */
        curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, write_data);

        /* open the file */
        pagefile = fopen(pagefilename, "wb");
        if(pagefile) {

            /* write the page body to this file handle */
            curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, pagefile);

            /* get it! */
            curl_easy_perform(curl_handle);

            /* close the header file */
            fclose(pagefile);
        }
        /* cleanup curl stuff */
        curl_easy_cleanup(curl_handle);
        curl_global_cleanup();
    }
    return 0;
}


void TombRaiderLinuxLauncher::extractZip(const QString& zipFilename, const QString& extractPath)
{
    QuaZip zip(zipFilename);
    if (!zip.open(QuaZip::mdUnzip))
    {
        std::cerr << "Error opening ZIP file" << std::endl;
        return;
    }

    QuaZipFileInfo info;
    QuaZipFile file(&zip);

    for (bool more = zip.goToFirstFile(); more; more = zip.goToNextFile())
    {
        if (zip.getCurrentFileInfo(&info) && file.open(QIODevice::ReadOnly))
        {
            QByteArray data = file.readAll();
            file.close();

            // Create directories if they don't exist
            QString filePath = extractPath + QDir::separator() + info.name;
            QString directory = filePath.left(filePath.lastIndexOf(QDir::separator()));

            QDir().mkpath(directory);

            // Create a new file on disk and write the data
            QFile outFile(filePath);
            if (outFile.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Unbuffered))
            {
                outFile.write(data);
                outFile.close();
            }
            else
            {
                std::cerr << "Error opening file for writing: " << filePath.toStdString() << std::endl;
            }
        }
        else
        {
            std::cerr << "Error reading file info from ZIP archive" << std::endl;
        }
    }

    zip.close();
}



TombRaiderLinuxLauncher::~TombRaiderLinuxLauncher()
{
    delete ui;
}
