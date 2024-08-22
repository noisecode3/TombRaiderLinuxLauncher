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

#include <QMessageBox>
#include <QScrollArea>
#include <QVBoxLayout>
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
    ui(new Ui::TombRaiderLinuxLauncher)
{
    ui->setupUi(this);

    // Button signal connections
    connect(ui->pushButtonLink, SIGNAL(clicked()), this, SLOT(linkClicked()));
    connect(ui->pushButtonDownload, SIGNAL(clicked()),
        this, SLOT(downloadClicked()));
    connect(ui->pushButtonInfo, SIGNAL(clicked()), this, SLOT(infoClicked()));
    connect(ui->pushButtonWalkthrough, SIGNAL(clicked()),
        this, SLOT(walkthroughClicked()));
    connect(ui->infoBackButton, SIGNAL(clicked()), this, SLOT(backClicked()));
    connect(ui->walkthroughBackButton, SIGNAL(clicked()),
        this, SLOT(backClicked()));
    connect(ui->setOptions, SIGNAL(clicked()), this, SLOT(setOptionsClicked()));
    connect(ui->listWidgetModds, SIGNAL(itemSelectionChanged()),
        this, SLOT(onListItemSelected()));

    // Progress bar signal connection
    connect(&Controller::getInstance(), SIGNAL(controllerTickSignal()),
        this, SLOT(workTick()));

    // Thread work done signal connections
    connect(&Controller::getInstance(), SIGNAL(controllerGenerateList()),
        this, SLOT(generateList()));

    // Error signal connections
    connect(&Controller::getInstance(), SIGNAL(controllerDownloadError(int)),
        this, SLOT(downloadError(int)));

    // Ask if a game should be moved and get a symbolic link
    connect(&Controller::getInstance(), SIGNAL(controllerAskGame(int)),
        this, SLOT(askGame(int)));

    // Set init state
    ui->pushButtonLink->setEnabled(false);
    ui->pushButtonInfo->setEnabled(false);
    ui->pushButtonDownload->setEnabled(false);

    // Read settings
    QString value = settings.value("setup").toString();
    if (value != "yes")
        setup();
    else
        readSavedSettings();
}

void TombRaiderLinuxLauncher::askGame(int id)
{
    QMessageBox msgBox;
    msgBox.setWindowTitle("Confirmation");
    msgBox.setText(
        "TombRaider " +
        QString::number(id) +
        " found, you want to proceed?");
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::No);
    int result = msgBox.exec();
    if (result == QMessageBox::Yes)
    {
        qDebug() << "User clicked Yes.";
        controller.setupGame(id);
    }
    else
    {
        qDebug() << "User clicked No or closed the dialog.";
        controller.checkCommonFiles();
    }
}

void TombRaiderLinuxLauncher::generateList()
{
    const QString& directoryPath = settings.value("levelPath").toString();
    const QString& pictures = ":/pictures/pictures/";

    QDir info(directoryPath);
    QFileInfoList entryInfoList = info.entryInfoList();

    QMap<QString, QPair<QString, int>> fileMap = {
        {".TR1", {"Tomb_Raider_I.jpg", -1}},
        {".TR2", {"Tomb_Raider_II.jpg", -2}},
        {".TR3", {"Tomb_Raider_III.jpg", -3}},
        {".TR4", {"Tomb_Raider_IIII.jpg", -4}},
        {".TR5", {"Tomb_Raider_IIIII.jpg", -5}}
    };

    foreach(const QFileInfo &file, entryInfoList)
    {
        QString extension = file.suffix().prepend(".");
        if (fileMap.contains(extension))
        {
            if (file.fileName() == "Original" + extension)
            {
                QString iconPath = pictures + fileMap[extension].first;
                QString itemName =
                    QString("Tomb Raider %1 Original").arg(extension.mid(3));
                int userRole = fileMap[extension].second;

                QListWidgetItem *wi =
                    new QListWidgetItem(QIcon(iconPath), itemName);
                wi->setData(Qt::UserRole, QVariant(userRole));
                ui->listWidgetModds->addItem(wi);
            }
            else
            {
                qDebug() << "No link or id implementation";
                // Implement logic to handle other files
            }
        }
    }

    QVector<ListItemData> list;
    controller.getList(list);
    const size_t s = list.size();
    for (int i = 0; i < s; i++)
    {
        QString tag = list[i].title +" by "+ list[i].author;
        QListWidgetItem *wi =
            new QListWidgetItem(list[i].picture, tag);
        wi->setData(Qt::UserRole, QVariant(i+1));
        ui->listWidgetModds->addItem(wi);
    }
}

void TombRaiderLinuxLauncher::readSavedSettings()
{
    const QString& gamePathValue = settings.value("gamePath").toString();
    qDebug() << "Read game path value:" << gamePathValue;
    const QString& levelPathValue = settings.value("levelPath").toString();
    qDebug() << "Read level path value:" << levelPathValue;
    controller.setup(levelPathValue, gamePathValue);
}

void TombRaiderLinuxLauncher::setup()
{
    ui->Tabs->setCurrentIndex(
        ui->Tabs->indexOf(ui->Tabs->findChild<QWidget*>("Setup")));
    ui->setupStackedWidget->setCurrentWidget(
        ui->setupStackedWidget->findChild<QWidget*>("firstTime"));
    ui->Tabs->setTabEnabled(
        ui->Tabs->indexOf(ui->Tabs->findChild<QWidget*>("Levels")), false);
    ui->Tabs->setTabEnabled(
        ui->Tabs->indexOf(ui->Tabs->findChild<QWidget*>("Modding")), false);

    qDebug() << "Entering setup" << Qt::endl;
    const QString& homeDir =
        QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
    qDebug() << "Home Directory:" << homeDir;
    const QString& s = "/.steam/root/steamapps/common/";
    const QString& l = "/.local/share/TombRaiderLinuxLauncher";
    ui->gamePathEdit->setText(homeDir + s);
    ui->levelPathEdit->setText(homeDir + l);
}

void TombRaiderLinuxLauncher::onListItemSelected()
{
    QListWidgetItem *selectedItem = ui->listWidgetModds->currentItem();
    if (selectedItem)
    {
        int id = selectedItem->data(Qt::UserRole).toInt();
        int state = controller.getItemState(id);
        // Activate or deactivate pushbuttons based on the selected item
        qDebug() << id << Qt::endl;
        // if state == 1 then only activate link button
        // if state == 2 then only activate link and info button
        // if state == 0 then only activate download button
        // if state == -1 then de-activate all buttons

        if (state == 1)
        {
            ui->pushButtonLink->setEnabled(true);
            ui->pushButtonInfo->setEnabled(false);
            ui->pushButtonDownload->setEnabled(false);
        }
        else if (state == 2)
        {
            ui->pushButtonLink->setEnabled(true);
            ui->pushButtonInfo->setEnabled(true);
            ui->pushButtonDownload->setEnabled(false);
        }
        else if (state == 0)
        {
            ui->pushButtonLink->setEnabled(false);
            ui->pushButtonInfo->setEnabled(true);
            ui->pushButtonDownload->setEnabled(true);
        }
        else if (state == -1)
        {
            ui->pushButtonLink->setEnabled(false);
            ui->pushButtonInfo->setEnabled(false);
            ui->pushButtonDownload->setEnabled(false);
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

    ui->tableWidgetSetup->item(0, 0)->setText(gamePath);
    ui->tableWidgetSetup->item(1, 0)->setText(levelPath);
    ui->Tabs->setTabEnabled(
        ui->Tabs->indexOf(ui->Tabs->findChild<QWidget*>("Levels")), true);
    ui->Tabs->setTabEnabled(
        ui->Tabs->indexOf(ui->Tabs->findChild<QWidget*>("Modding")), true);
    ui->Tabs->show();
    ui->Tabs->setCurrentIndex(
        ui->Tabs->indexOf(ui->Tabs->findChild<QWidget*>("Levels")));
    ui->setupStackedWidget->setCurrentWidget(
        ui->setupStackedWidget->findChild<QWidget*>("settings"));

    readSavedSettings();
}

void TombRaiderLinuxLauncher::linkClicked()
{
    QListWidgetItem *selectedItem = ui->listWidgetModds->currentItem();
    int id = selectedItem->data(Qt::UserRole).toInt();
    if (id)
    {
        if (!controller.link(id))
        {
            qDebug() << "Sumo linko el la compleeteo";
        }
        else
        {
            qDebug() << "Problemo de link";
        }
    }
    else
    {
        qDebug() << "Say no more";
    }
    QApplication::quit();
}

void TombRaiderLinuxLauncher::downloadClicked()
{
    QListWidgetItem *selectedItem = ui->listWidgetModds->currentItem();
    int id = selectedItem->data(Qt::UserRole).toInt();
    if (id)
    {
        ui->progressBar->setValue(0);
        ui->stackedWidgetBar->setCurrentWidget(
            ui->stackedWidgetBar->findChild<QWidget*>("progress"));
        controller.setupLevel(id);
    }
}

void TombRaiderLinuxLauncher::infoClicked()
{
    QListWidgetItem *selectedItem = ui->listWidgetModds->currentItem();
    int id = selectedItem->data(Qt::UserRole).toInt();
    if (id)
    {
        InfoData info = controller.getInfo(id);
        ui->infoWebEngineView->setHtml(info.body);
        ui->infoListWidget->setViewMode(QListView::IconMode);
        ui->infoListWidget->setIconSize(QSize(502, 377));
        ui->infoListWidget->setDragEnabled(false);
        ui->infoListWidget->setAcceptDrops(false);
        ui->infoListWidget->setDragDropMode(QAbstractItemView::NoDragDrop);
        ui->infoListWidget->setDefaultDropAction(Qt::IgnoreAction);
        ui->infoListWidget->setSelectionMode(QAbstractItemView::NoSelection);
        ui->infoListWidget->clear();
        for (int i = 0; i < info.imageList.size(); ++i)
        {
            const QIcon &icon = info.imageList.at(i);
            QListWidgetItem *item = new QListWidgetItem(icon, "");
            ui->infoListWidget->addItem(item);
        }
        ui->infoWebEngineView->show();
        ui->stackedWidget->setCurrentWidget(
                ui->stackedWidget->findChild<QWidget*>("info"));
        if (controller.getWalkthrough(id) != "")
        {
            ui->pushButtonWalkthrough->setEnabled(true);
        }
        else
        {
            ui->pushButtonWalkthrough->setEnabled(false);
        }
    }
}

void TombRaiderLinuxLauncher::walkthroughClicked()
{
    QListWidgetItem *selectedItem = ui->listWidgetModds->currentItem();
    int id = selectedItem->data(Qt::UserRole).toInt();
    if (id)
    {
        ui->walkthroughWebEngineView->setHtml(controller.getWalkthrough(id));
        ui->walkthroughWebEngineView->show();
        ui->stackedWidget->setCurrentWidget(
                ui->stackedWidget->findChild<QWidget*>("walkthrough"));
    }
}

void TombRaiderLinuxLauncher::backClicked()
{
    if (ui->stackedWidget->currentWidget() ==
        ui->stackedWidget->findChild<QWidget*>("info"))
    {
        ui->stackedWidget->setCurrentWidget(
            ui->stackedWidget->findChild<QWidget*>("select"));
    }
    else if (ui->stackedWidget->currentWidget() ==
        ui->stackedWidget->findChild<QWidget*>("walkthrough"))
    {
        ui->stackedWidget->setCurrentWidget(
            ui->stackedWidget->findChild<QWidget*>("select"));
    }
}

void TombRaiderLinuxLauncher::workTick()
{
    int value = ui->progressBar->value();
    ui->progressBar->setValue(value + 1);
    qDebug() << ui->progressBar->value() << "%";
    if (ui->progressBar->value() >= 100)
    {
        ui->pushButtonLink->setEnabled(true);
        ui->pushButtonInfo->setEnabled(true);
        ui->pushButtonDownload->setEnabled(false);
        ui->stackedWidgetBar->setCurrentWidget(
            ui->stackedWidgetBar->findChild<QWidget*>("navigate"));
    }
}

void TombRaiderLinuxLauncher::downloadError(int status)
{
    ui->progressBar->setValue(0);
    ui->pushButtonLink->setEnabled(true);
    ui->pushButtonInfo->setEnabled(true);
    ui->pushButtonDownload->setEnabled(true);
    ui->stackedWidgetBar->setCurrentWidget(
        ui->stackedWidgetBar->findChild<QWidget*>("navigate"));
    QMessageBox msgBox;
    msgBox.setWindowTitle("Error");
    if (status == 1)
    {
        msgBox.setText("No internet");
    }
    else if (status == 2)
    {
        msgBox.setText("You seem to be missing ssl keys");
    }
    else
    {
        msgBox.setText("Could not connect");
    }
    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.setDefaultButton(QMessageBox::Ok);
    msgBox.exec();
}

TombRaiderLinuxLauncher::~TombRaiderLinuxLauncher()
{
    delete ui;
}
