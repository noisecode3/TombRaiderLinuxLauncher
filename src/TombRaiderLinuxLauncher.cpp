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

    connect(ui->pushButtonLink, SIGNAL (clicked()), this, SLOT (linkClicked()));
    connect(ui->pushButtonDownload, SIGNAL (clicked()), this, SLOT (downloadClicked()));
    connect(ui->pushButtonInfo, SIGNAL (clicked()), this, SLOT (infoClicked()));
    connect(ui->pushButtonWalkthrough, SIGNAL (clicked()), this, SLOT (walkthroughClicked()));
    connect(ui->infoBackButton, SIGNAL (clicked()), this, SLOT (backClicked()));
    connect(ui->walkthroughBackButton, SIGNAL (clicked()), this, SLOT (backClicked()));
    connect(ui->setOptions, SIGNAL (clicked()), this, SLOT (setOptionsClicked()));
    connect(ui->listWidgetModds, SIGNAL(itemSelectionChanged()), this, SLOT(onListItemSelected()));

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


int TombRaiderLinuxLauncher::testallGames(int id){
    // this should be in model if it works
    int dirStaus = controller.checkGameDirectory(id);
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
            msgBox.setText("TombRaider "+QString::number(id)+" found, you want to proceed?");
            msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
            msgBox.setDefaultButton(QMessageBox::No);
            int result = msgBox.exec();
            if (result == QMessageBox::Yes) {
                qDebug() << "User clicked Yes.";
                controller.setupOg(id);
            }
            else
            {
                qDebug() << "User clicked No or closed the dialog.";
            }
        }
        else if(dirStaus == 3)// The path is not a directory.
        {
            //it has been replaced by a file from somewhere else or the user
            // or is missing
        }
    }
    return -1;
}

void TombRaiderLinuxLauncher::checkCommonFiles()
{
    testallGames(3);
    testallGames(4);
    generateList();
}

void TombRaiderLinuxLauncher::generateList()
{
    ui->listWidgetModds->setIconSize(QSize(320, 240));
    QFont defaultFont = QApplication::font();
    int newSize = 22;
    defaultFont.setPointSize(newSize);
    ui->listWidgetModds->setFont(defaultFont);
    const QString& directoryPath = settings.value("levelPath").toString();
    const QString& pictures = ":/pictures/pictures/";

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
                wi->setData(Qt::UserRole, QVariant(-3));
                ui->listWidgetModds->addItem(wi);
            }
            else
            {
                qDebug() << "No link or id implementation";
                //read some json file 
                //QListWidgetItem *wi = new QListWidgetItem(QIcon(pictures+"Tomb_Raider_III_unkown.jpg"),file.fileName());
                //ui->listWidgetModds->addItem(wi);
            }
        }
        if (Ending == ".TR4")
        {
            if(file.fileName() == "Original.TR4")
            {
                QListWidgetItem *wi = new QListWidgetItem(QIcon(pictures+"Tomb_Raider_IIII.jpg"),"Tomb Raider IV Original");
                wi->setData(Qt::UserRole, QVariant(-4));
                ui->listWidgetModds->addItem(wi);
            }
            else
            {
                qDebug() << "No link or id implementation";
                //QListWidgetItem *wi = new QListWidgetItem(QIcon(pictures+"Tomb_Raider_III_unkown.jpg"),file.fileName());
                //ui->listWidgetModds->addItem(wi);
            }
        }
    }

    QVector<ListItemData> list;
    controller.getList(list);
    const size_t s = list.size();
    for (int i = 0; i<s;i++)
    {
        QString tag = list[i].title +" by "+ list[i].author;
        QListWidgetItem *wi = new QListWidgetItem(list[i].picture,tag);
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
    if (controller.setupCamp(levelPathValue, gamePathValue))
        checkCommonFiles();
}

void TombRaiderLinuxLauncher::setup()
{
    ui->Tabs->setCurrentIndex(ui->Tabs->indexOf(ui->Tabs->findChild<QWidget*>("Setup")));
    ui->setupStackedWidget->setCurrentWidget(ui->setupStackedWidget->findChild<QWidget*>("firstTime"));
    ui->Tabs->setTabEnabled(ui->Tabs->indexOf(ui->Tabs->findChild<QWidget*>("Levels")), false);
    ui->Tabs->setTabEnabled(ui->Tabs->indexOf(ui->Tabs->findChild<QWidget*>("Modding")), false);

    qDebug() << "Entering setup" << Qt::endl;
    const QString& homeDir = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
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
    QListWidgetItem *selectedItem = ui->listWidgetModds->currentItem();
    int id = selectedItem->data(Qt::UserRole).toInt();
    QString s = selectedItem->text();
    if (id)
    {
        if(!controller.link(id))
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
        controller.setupLevel(id);
        if (ui->listWidgetModds->currentItem() == selectedItem)
        {
            ui->pushButtonLink->setEnabled(true);
            ui->pushButtonDownload->setEnabled(false);
            ui->pushButtonInfo->setEnabled(true);
        }
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
        ui->infoListWidget->clear();
        for (const QIcon &icon : info.imageList)
        {
            QListWidgetItem *item = new QListWidgetItem(icon, "");
            ui->infoListWidget->addItem(item);
        }
        ui->infoWebEngineView->show();
        ui->stackedWidget->setCurrentWidget(ui->stackedWidget->findChild<QWidget*>("info"));
        if(controller.getWalkthrough(id) != "")
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
        ui->stackedWidget->setCurrentWidget(ui->stackedWidget->findChild<QWidget*>("walkthrough"));
    }
}

void TombRaiderLinuxLauncher::backClicked()
{
    if(ui->stackedWidget->currentWidget() == ui->stackedWidget->findChild<QWidget*>("info"))
    {
        ui->stackedWidget->setCurrentWidget(ui->stackedWidget->findChild<QWidget*>("select"));
    }
    else if (ui->stackedWidget->currentWidget() == ui->stackedWidget->findChild<QWidget*>("walkthrough"))
    {
        ui->stackedWidget->setCurrentWidget(ui->stackedWidget->findChild<QWidget*>("select"));
    }
}
TombRaiderLinuxLauncher::~TombRaiderLinuxLauncher()
{
    delete ui;
}
