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
#include "worker.h"
#include "ui_TombRaiderLinuxLauncher.h"

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
    connect(ui->pushButtonDownload, SIGNAL (clicked()), this, SLOT (downloadClicked()));
    connect(ui->setOptions, SIGNAL (clicked()), this, SLOT (setOptionsClicked()));

    //pool games;
    //games.setName("Jonson-TheInfadaCult.TR3");
    //games.setFileList(TRLE3573FileList);
    //games.setData(TRLE3573);

    // Read settings
    QString value = settings.value("setup").toString();
    settings.setValue("setup" , "yes");
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
 * this should be in the worker class 
 *
 */
void TombRaiderLinuxLauncher::checkCommonFiles()
{
    int dirStaus = checkGameDirectory(TR3); // we only setup tr3 for now
    folderNames folder;
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
                WorkerThread packOriginalGame(3, true, settings.value("gamePath").toString() + folder.TR3, settings.value("levelPath").toString() + "/Original.TR3");


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

void TombRaiderLinuxLauncher::downloadClicked()
{
}

TombRaiderLinuxLauncher::~TombRaiderLinuxLauncher()
{
    delete ui;
}
