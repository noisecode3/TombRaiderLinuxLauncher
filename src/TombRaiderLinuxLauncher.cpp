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


TombRaiderLinuxLauncher::TombRaiderLinuxLauncher(QWidget *parent)
    :QMainWindow(parent),
    poolData(QStandardPaths::writableLocation(QStandardPaths::HomeLocation)
        + QString("/.local/share/TombRaiderLinuxLauncher")),
    ui(new Ui::TombRaiderLinuxLauncher)
{
    ui->setupUi(this);
    connect(ui->pushButtonLink, SIGNAL (clicked()), this, SLOT (linkClicked()));
    connect(ui->pushButtonDownload, SIGNAL (clicked()), this, SLOT (downloadClicked()));
    connect(ui->setOptions, SIGNAL (clicked()), this, SLOT (setOptionsClicked()));
    connect(ui->listWidgetModds, SIGNAL(itemSelectionChanged()), this, SLOT(onListItemSelected()));
    ui->pushButtonLink->setEnabled(false);
    ui->pushButtonInfo->setEnabled(false);
    ui->pushButtonDownload->setEnabled(false);

    // Read settings
    QString value = settings.value("setup").toString();
    settings.setValue("setup" , "yes");
    if (value != "yes")
        setup();
    else
        readSavedSettings();
}

int TombRaiderLinuxLauncher::checkGameDirectory(int game)
{
    struct FolderNames folder;
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

void TombRaiderLinuxLauncher::checkCommonFiles()
{
    int dirStaus = checkGameDirectory(TR3); // we only setup tr3 for now
    FolderNames folder;
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
                packOriginalGame.run();


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
        QString Ending = file.fileName().right(4);
        if (Ending == ".TR3")
        {
            if(file.fileName() == "Original.TR3")
            {
                QListWidgetItem *wi = new QListWidgetItem(QIcon(pictures+"Tomb_Raider_III.jpg"),"Tomb Raider III Original");
                ui->listWidgetModds->addItem(wi);
            }
            else
            {
                QListWidgetItem *wi = new QListWidgetItem(file.fileName());
                ui->listWidgetModds->addItem(wi);
            }
        }
    }
    const int s = poolData.getSize();
    for (int i = 0; i<s;i++)
    {
        LevelData level = poolData.getData(i);
        QString tag = level.info.title +" by "+ level.info.author;
        QListWidgetItem *wi = new QListWidgetItem(level.picture.data,tag);
        wi->setData(Qt::UserRole, QVariant(i+1));
        ui->listWidgetModds->addItem(wi);
    }
}

void TombRaiderLinuxLauncher::readSavedSettings()
{
    QString gamePathValue = settings.value("gamePath").toString();
    qDebug() << "Read game path value:" << gamePathValue;
    QString levelPathValue = settings.value("levelPath").toString();
    qDebug() << "Read level path value:" << levelPathValue;
    checkCommonFiles();
}

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

void TombRaiderLinuxLauncher::onListItemSelected()
{
    QString directoryPath = settings.value("levelPath").toString();
    QListWidgetItem *selectedItem = ui->listWidgetModds->currentItem();
    if (selectedItem)
    {
        int retrievedIdentifier = selectedItem->data(Qt::UserRole).toInt();
        // Activate or deactivate pushbuttons based on the selected item
        qDebug() << retrievedIdentifier << Qt::endl;
        if (retrievedIdentifier)
        {
            QString id = selectedItem->data(Qt::UserRole).toString();
            QString finalPath = directoryPath + "/" + id + ".TRLE";
            QDir directory(finalPath);
            if (directory.exists())
            {
                qDebug() << "Directory exists.";
                ui->pushButtonLink->setEnabled(true);
                ui->pushButtonDownload->setEnabled(false);
                ui->pushButtonInfo->setEnabled(false);
            }
            else
            {
                qDebug() << "Directory does not exist.";
                ui->pushButtonLink->setEnabled(false);
                ui->pushButtonDownload->setEnabled(true);
                ui->pushButtonInfo->setEnabled(false);
            }
        }
        else
        {
            QString tag = selectedItem->text();
            if (tag == "Tomb Raider III Original")
                tag="Original.TR3";

            QString finalPath = directoryPath + "/" + tag;
            QDir directory(finalPath);
            if (directory.exists()) {
                qDebug() << "Directory exists.";
                ui->pushButtonLink->setEnabled(true);
                ui->pushButtonDownload->setEnabled(false);
                ui->pushButtonInfo->setEnabled(false);
            } else {
                qDebug() << "Directory does not exist.";
                ui->pushButtonLink->setEnabled(false);
                ui->pushButtonDownload->setEnabled(false);
                ui->pushButtonInfo->setEnabled(false);
            }
        }
    }
}

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

void TombRaiderLinuxLauncher::linkClicked()
{
    struct FolderNames folder;
    const QString gamePath = settings.value("gamePath").toString() + "TombRaider (III)";
    const QString levelPath = settings.value("levelPath").toString();
    QListWidgetItem *selectedItem = ui->listWidgetModds->currentItem();
    int retrievedIdentifier = selectedItem->data(Qt::UserRole).toInt();
    QString s = selectedItem->text();
    if (!retrievedIdentifier)
    {
        if (s == "Tomb Raider III Original")
        {
            s = "Original.TR3";
        }
    }
    else
    {
        QString id = selectedItem->data(Qt::UserRole).toString();
        s = id + ".TRLE";
    }
    // Create a symbolic link
    const QString p = levelPath + "/" + s;
    if (QFile::link(p, gamePath))
    {
        qDebug() << "Symbolic link created successfully.";
    }
    else
    {
        QFileInfo i(gamePath);
        if (i.isSymLink())
        {
            QFile::remove(gamePath);
            if (QFile::link(p, gamePath))
            {
                qDebug() << "Symbolic link created successfully.";
            }
            else
            {
                qDebug() << "Failed to create symbolic link.";
            }
        }
        else
        {
            qDebug() << "Failed to create symbolic link.";
        }
    }
    QApplication::quit();
}
//TODO/////////////////////////////////////////////////////////////////////////////////////////////////
void TombRaiderLinuxLauncher::downloadClicked()
{
    struct FolderNames folder;
    QString directoryPath = settings.value("levelPath").toString();
    QListWidgetItem *selectedItem = ui->listWidgetModds->currentItem();

    int retrievedIdentifier = selectedItem->data(Qt::UserRole).toInt();
    if (retrievedIdentifier)
    {
        LevelData level =  poolData.getData(retrievedIdentifier-1);
        QUrl url(level.zip.url);
        QString path =  directoryPath+"/"+level.zip.name;
        QFile zip(path);
        QString levelDir = settings.value("levelPath").toString() + "/"+
                           QString::number(retrievedIdentifier)+".TRLE";
        if (zip.exists()) {
            qDebug() << "File exists:" << path;
            FileManager f;
            if(f.calculateMD5(path) != level.zip.md5sum)
            {
                Downloader d;
                d.setUrl(url);
                d.setSavePath(path);
                d.run();
            }
        } else {
            qWarning() << "File does not exist:" << path;
            Downloader d;
            d.setUrl(url);
            d.setSavePath(path);
            d.run();
        }

        WorkerThread unpackLevel(retrievedIdentifier, false, path, levelDir);
        unpackLevel.run();

        if (ui->listWidgetModds->currentItem() == selectedItem)
        {
            ui->pushButtonLink->setEnabled(true);
            ui->pushButtonDownload->setEnabled(false);
        }
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////
TombRaiderLinuxLauncher::~TombRaiderLinuxLauncher()
{
    delete ui;
}
