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

#include <algorithm>
#include "TombRaiderLinuxLauncher.hpp"
#include "ui_TombRaiderLinuxLauncher.h"
#include "staticData.hpp"
// #include "debug.hpp"

TombRaiderLinuxLauncher::TombRaiderLinuxLauncher(QWidget *parent)
    : QMainWindow(parent) {
    ui = new Ui::TombRaiderLinuxLauncher;
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
    connect(&Controller::getInstance(),
        SIGNAL(controllerGenerateList(const QList<int>&)),
        this, SLOT(generateList(const QList<int>&)));

    // Error signal connections
    connect(&Controller::getInstance(), SIGNAL(controllerDownloadError(int)),
        this, SLOT(downloadError(int)));

    // Set init state
    ui->pushButtonLink->setEnabled(false);
    ui->pushButtonInfo->setEnabled(false);
    ui->pushButtonDownload->setEnabled(false);
    QWidget* filter_p = ui->filter;
    QPushButton* filterButton_p = ui->filterButton;
    filter_p->setVisible(false);

    connect(ui->filterButton, &QPushButton::clicked,
        [filter_p, filterButton_p]() -> void {
            bool isVisible = !filter_p->isVisible();
            filter_p->setVisible(isVisible);

            QIcon arrowDownIcon(":/icons/down-arrow.svg");
            QIcon arrowUpIcon(":/icons/up-arrow.svg");

            if (isVisible) {
                filterButton_p->setIcon(arrowUpIcon);
            } else {
                filterButton_p->setIcon(arrowDownIcon);
            }
            filterButton_p->setIconSize(QSize(16, 16));
        });

    // Get the system palette
    QPalette systemPalette = ui->filterButton->palette();

    // Get system colors for different roles
    QColor normalColor = systemPalette.color(QPalette::Button);
    QColor hoverColor = systemPalette.color(QPalette::Highlight);
    QColor textColor = systemPalette.color(QPalette::ButtonText);
    QColor borderColor = systemPalette.color(QPalette::Mid);

    QIcon arrowDownIcon(":/icons/down-arrow.svg");
    QIcon arrowUpIcon(":/icons/up-arrow.svg");

    filterButton_p->setIcon(arrowDownIcon);
    filterButton_p->setIconSize(QSize(16, 16));

    connect(ui->radioButtonLevelName, &QRadioButton::clicked,
            this, &TombRaiderLinuxLauncher::sortByTitle);
    connect(ui->radioButtonAuthor, &QRadioButton::clicked,
            this, &TombRaiderLinuxLauncher::sortByAuthor);
    connect(ui->radioButtonDifficulty, &QRadioButton::clicked,
            this, &TombRaiderLinuxLauncher::sortByDifficulty);
    connect(ui->radioButtonDuration, &QRadioButton::clicked,
            this, &TombRaiderLinuxLauncher::sortByDuration);
    connect(ui->radioButtonClass, &QRadioButton::clicked,
            this, &TombRaiderLinuxLauncher::sortByClass);
    connect(ui->radioButtonType, &QRadioButton::clicked,
            this, &TombRaiderLinuxLauncher::sortByType);
    connect(ui->radioButtonReleaseDate, &QRadioButton::clicked,
            this, &TombRaiderLinuxLauncher::sortByReleaseDate);

    // Read settings
    QString value = m_settings.value("setup").toString();
    if (value != "yes") {
        setup();
    } else {
        readSavedSettings();
    }
}

void TombRaiderLinuxLauncher::generateList(const QList<int>& availableGames) {
    const QString pictures = ":/pictures/";
    OriginalGameData pictueData;

    foreach(const int &id, availableGames) {
        // debugStop(QString("%1").arg(id));
        int IdPositive;
        bool linkedGameDir;
        if (id < 0) {
            linkedGameDir = false;
            IdPositive = id*(-1);
        } else {
            linkedGameDir = true;
            IdPositive = id;
        }
        // debugStop();
        // Picture and title
        QString iconPath = pictures + pictueData.getPicture(IdPositive);
        QString itemName =
            QString("Tomb Raider %1 Original")
                .arg(pictueData.romanNumerals[IdPositive]);

        qDebug() << "iconPath :" << iconPath;
        qDebug() << "itemName :" << itemName;
        QListWidgetItem *wi = new QListWidgetItem(QIcon(iconPath), itemName);
        wi->setData(Qt::UserRole, QVariant(IdPositive*(-1)));
        wi->setData(Qt::UserRole + 1, QVariant(linkedGameDir));
        ui->listWidgetModds->addItem(wi);
        originalGamesSet_m.insert(wi);
        originalGamesList_m.append(wi);
    }

    StaticData staticData;
    auto mapType = staticData.getType();
    auto mapClass = staticData.getClass();
    auto mapDifficulty = staticData.getDifficulty();
    auto mapDuration = staticData.getDuration();

    QVector<ListItemData> list;
    controller.getList(&list);
    const qint64 s = list.size();
    for (qint64 i = 0; i < s; i++) {
        QString tag = QString("%1 by %2\n")
                          .arg(list[i].m_title)
                          .arg(list[i].m_author);

        tag += QString(
                "Type: %1\nClass: %2\nDifficulty: %3\nDuration: %4\nDate:%5")
                   .arg(mapType.at(list[i].m_type))
                   .arg(mapClass.at(list[i].m_class))
                   .arg(mapDifficulty.at(list[i].m_difficulty))
                   .arg(mapDuration.at(list[i].m_duration))
                   .arg(list[i].m_releaseDate);

        QListWidgetItem *wi =
            new QListWidgetItem(list[i].m_picture, tag);

        wi->setData(Qt::UserRole, QVariant(i + 1));
        QVariantMap itemData;
        itemData["title"] = list[i].m_title;
        itemData["author"] = list[i].m_author;
        itemData["type"] = list[i].m_type;
        itemData["class_"] = list[i].m_class;
        itemData["releaseDate"] = list[i].m_releaseDate;
        itemData["difficulty"] = list[i].m_difficulty;
        itemData["duration"] = list[i].m_duration;
        // qDebug() << itemData << Qt::endl;
        wi->setData(Qt::UserRole + 1, itemData);
        ui->listWidgetModds->addItem(wi);
    }
    sortByTitle();
}

void TombRaiderLinuxLauncher::sortItems(
    std::function<bool(QListWidgetItem*, QListWidgetItem*)> compare) {
    QList<QListWidgetItem*> items;

    // Step 1: Extract items from the QListWidget
    if (ui->checkBoxOriginalFirst->isChecked() == true) {
        for (int i = 0; i < ui->listWidgetModds->count(); ++i) {
            QListWidgetItem* item = ui->listWidgetModds->item(i);
            // Append the item only if it's not in originalGamesSet_m
            if (!originalGamesSet_m.contains(item)) {
                items.append(item);
            }
        }
    } else {
        for (int i = 0; i < ui->listWidgetModds->count(); ++i) {
            items.append(ui->listWidgetModds->item(i));
        }
    }

    // Step 2: Sort the items using the provided comparison lambda
    std::sort(items.begin(), items.end(), compare);

    // Step 3: Adjust the position of each item without clearing the list
    qint64 originalGamesSize =
        ui->checkBoxOriginalFirst->isChecked() ? originalGamesList_m.size() : 0;

    qint64 size = originalGamesSize + items.size();
    for (qint64 i = 0; i < size; ++i) {
        QListWidgetItem* item;
        if (i < originalGamesSize) {
            // Handle originalGamesList_m items
            item = ui->listWidgetModds->takeItem(
                    ui->listWidgetModds->row(originalGamesList_m[i]));
            ui->listWidgetModds->insertItem(i, item);
        } else {
            // Handle items list
            item = ui->listWidgetModds->takeItem(
                    ui->listWidgetModds->row(items[i - originalGamesSize]));
            ui->listWidgetModds->insertItem(i, item);
        }
    }
}

bool compareTitles(QListWidgetItem* a, QListWidgetItem* b) {
    const QVariantMap aData = a->data(Qt::UserRole + 1).toMap();
    const QVariantMap bData = b->data(Qt::UserRole + 1).toMap();

    return aData.value("title").toString().toLower()
        < bData.value("title").toString().toLower();
}

bool compareAuthors(QListWidgetItem* a, QListWidgetItem* b) {
    const QVariantMap aData = a->data(Qt::UserRole + 1).toMap();
    const QVariantMap bData = b->data(Qt::UserRole + 1).toMap();
    return aData.value("author").toString().toLower()
        < bData.value("author").toString().toLower();
}

bool compareDifficulties(QListWidgetItem* a, QListWidgetItem* b) {
    const QVariantMap aData = a->data(Qt::UserRole + 1).toMap();
    const QVariantMap bData = b->data(Qt::UserRole + 1).toMap();
    return aData.value("difficulty").toInt()
        > bData.value("difficulty").toInt();
}

bool compareDurations(QListWidgetItem* a, QListWidgetItem* b) {
    const QVariantMap aData = a->data(Qt::UserRole + 1).toMap();
    const QVariantMap bData = b->data(Qt::UserRole + 1).toMap();
    return aData.value("duration").toInt()
        > bData.value("duration").toInt();
}

bool compareClasses(QListWidgetItem* a, QListWidgetItem* b) {
    const QVariantMap aData = a->data(Qt::UserRole + 1).toMap();
    const QVariantMap bData = b->data(Qt::UserRole + 1).toMap();
    return aData.value("class_").toInt()
        > bData.value("class_").toInt();
}

bool compareTypes(QListWidgetItem* a, QListWidgetItem* b) {
    const QVariantMap aData = a->data(Qt::UserRole + 1).toMap();
    const QVariantMap bData = b->data(Qt::UserRole + 1).toMap();
    return aData.value("type").toInt()
        > bData.value("type").toInt();
}

bool compareReleaseDates(QListWidgetItem* a, QListWidgetItem* b) {
    const QVariantMap aData = a->data(Qt::UserRole + 1).toMap();
    const QVariantMap bData = b->data(Qt::UserRole + 1).toMap();

    const QDate dateA = QDate::fromString(
        aData.value("releaseDate").toString(), "dd-MMM-yyyy");
    const QDate dateB = QDate::fromString(
        bData.value("releaseDate").toString(), "dd-MMM-yyyy");

    return dateA > dateB;  // descending order
}


void TombRaiderLinuxLauncher::sortByTitle() {
    sortItems(compareTitles);
}

void TombRaiderLinuxLauncher::sortByAuthor() {
    sortItems(compareAuthors);
}

void TombRaiderLinuxLauncher::sortByDifficulty() {
    sortItems(compareDifficulties);
}

void TombRaiderLinuxLauncher::sortByDuration() {
    sortItems(compareDurations);
}

void TombRaiderLinuxLauncher::sortByClass() {
    sortItems(compareClasses);
}

void TombRaiderLinuxLauncher::sortByType() {
    sortItems(compareTypes);
}

void TombRaiderLinuxLauncher::sortByReleaseDate() {
    sortItems(compareReleaseDates);
}

void TombRaiderLinuxLauncher::readSavedSettings() {
    const QString gamePathValue = m_settings.value("gamePath").toString();
    qDebug() << "Read game path value:" << gamePathValue;
    const QString levelPathValue = m_settings.value("levelPath").toString();
    qDebug() << "Read level path value:" << levelPathValue;
    controller.setup(levelPathValue, gamePathValue);
}

void TombRaiderLinuxLauncher::setup() {
    ui->Tabs->setCurrentIndex(
        ui->Tabs->indexOf(ui->Tabs->findChild<QWidget*>("Setup")));
    ui->setupStackedWidget->setCurrentWidget(
        ui->setupStackedWidget->findChild<QWidget*>("firstTime"));
    ui->Tabs->setTabEnabled(
        ui->Tabs->indexOf(ui->Tabs->findChild<QWidget*>("Levels")), false);
    ui->Tabs->setTabEnabled(
        ui->Tabs->indexOf(ui->Tabs->findChild<QWidget*>("Modding")), false);

    qDebug() << "Entering setup" << Qt::endl;
    const QString homeDir =
        QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
    qDebug() << "Home Directory:" << homeDir;
    const QString s = "/.steam/root/steamapps/common/";
    const QString l = "/.local/share/TombRaiderLinuxLauncher";
    ui->gamePathEdit->setText(homeDir + s);
    ui->levelPathEdit->setText(homeDir + l);
}

void TombRaiderLinuxLauncher::originalSelected(QListWidgetItem *selectedItem) {
    if (selectedItem != nullptr) {
        int id = selectedItem->data(Qt::UserRole).toInt();
        bool linkedGameDir = selectedItem->data(Qt::UserRole + 1).toBool();
        // the game directory was a symbolic link and it has a level directory
        if ((linkedGameDir == true) && (controller.getItemState(id) == 1)) {
            ui->pushButtonLink->setEnabled(true);
            ui->pushButtonDownload->setEnabled(false);
        } else {
            ui->pushButtonLink->setEnabled(false);
            ui->pushButtonDownload->setEnabled(true);
        }
        ui->pushButtonInfo->setEnabled(false);
    }
}

void TombRaiderLinuxLauncher::levelDirSelected(QListWidgetItem *selectedItem) {
    if (selectedItem != nullptr) {
        int id = selectedItem->data(Qt::UserRole).toInt();
        int state = controller.getItemState(id);
        // Activate or deactivate pushbuttons based on the selected item
        qDebug() << id << Qt::endl;
        // if state == 1 then only activate link button
        // if state == 2 then only activate link and info button
        // if state == 0 then only activate download button
        // if state == -1 then de-activate all buttons

        if (state == 1) {
            ui->pushButtonLink->setEnabled(true);
            ui->pushButtonInfo->setEnabled(false);
            ui->pushButtonDownload->setEnabled(false);
        } else if (state == 2) {
            ui->pushButtonLink->setEnabled(true);
            ui->pushButtonInfo->setEnabled(true);
            ui->pushButtonDownload->setEnabled(false);
        } else if (state == 0) {
            ui->pushButtonLink->setEnabled(false);
            ui->pushButtonInfo->setEnabled(true);
            ui->pushButtonDownload->setEnabled(true);
        } else if (state == -1) {
            ui->pushButtonLink->setEnabled(false);
            ui->pushButtonInfo->setEnabled(false);
            ui->pushButtonDownload->setEnabled(false);
        }
    }
}

void TombRaiderLinuxLauncher::onListItemSelected() {
    QListWidgetItem *selectedItem = ui->listWidgetModds->currentItem();
    if (selectedItem != nullptr) {
        int id = selectedItem->data(Qt::UserRole).toInt();
        if (id < 0) {  // its the original game
            originalSelected(selectedItem);
        } else if (id > 0) {  // do not know what to do with 0
            levelDirSelected(selectedItem);
        }
    }
}

void TombRaiderLinuxLauncher::setOptionsClicked() {
    QString gamePath = ui->gamePathEdit->text();
    QString levelPath = ui->levelPathEdit->text();
    m_settings.setValue("gamePath" , gamePath);
    m_settings.setValue("levelPath" , levelPath);
    m_settings.setValue("setup" , "yes");

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

void TombRaiderLinuxLauncher::linkClicked() {
    bool status = false;
    QListWidgetItem *selectedItem = ui->listWidgetModds->currentItem();
    int id = selectedItem->data(Qt::UserRole).toInt();
    if (id != 0) {
        status = controller.link(id);
    } else {
        qDebug() << "id error";
    }
    if (status == true) {
        QApplication::quit();
    } else {
        qDebug() << "link error";
    }
}

void TombRaiderLinuxLauncher::downloadClicked() {
    QListWidgetItem *selectedItem = ui->listWidgetModds->currentItem();
    int id = selectedItem->data(Qt::UserRole).toInt();
    if (id < 0) {
        ui->listWidgetModds->setEnabled(false);
        ui->progressBar->setValue(0);
        ui->stackedWidgetBar->setCurrentWidget(
            ui->stackedWidgetBar->findChild<QWidget*>("progress"));
        // debugStop(QString("%1").arg(id*(-1)));
        controller.setupGame(id*(-1));
    } else if (id > 0) {
        ui->listWidgetModds->setEnabled(false);
        ui->progressBar->setValue(0);
        ui->stackedWidgetBar->setCurrentWidget(
            ui->stackedWidgetBar->findChild<QWidget*>("progress"));
        controller.setupLevel(id);
    }
}

void TombRaiderLinuxLauncher::infoClicked() {
    QListWidgetItem *selectedItem = ui->listWidgetModds->currentItem();
    int id = selectedItem->data(Qt::UserRole).toInt();
    if (id != 0) {
        InfoData info = controller.getInfo(id);
        ui->infoWebEngineView->setHtml(info.m_body);
        ui->infoListWidget->setViewMode(QListView::IconMode);
        ui->infoListWidget->setIconSize(QSize(502, 377));
        ui->infoListWidget->setDragEnabled(false);
        ui->infoListWidget->setAcceptDrops(false);
        ui->infoListWidget->setDragDropMode(QAbstractItemView::NoDragDrop);
        ui->infoListWidget->setDefaultDropAction(Qt::IgnoreAction);
        ui->infoListWidget->setSelectionMode(QAbstractItemView::NoSelection);
        ui->infoListWidget->clear();
        for (int i = 0; i < info.m_imageList.size(); ++i) {
            const QIcon &icon = info.m_imageList.at(i);
            QListWidgetItem *item = new QListWidgetItem(icon, "");
            ui->infoListWidget->addItem(item);
        }
        ui->infoWebEngineView->show();
        ui->stackedWidget->setCurrentWidget(
                ui->stackedWidget->findChild<QWidget*>("info"));
        if (controller.getWalkthrough(id) != "") {
            ui->pushButtonWalkthrough->setEnabled(true);
        } else {
            ui->pushButtonWalkthrough->setEnabled(false);
        }
    }
}

void TombRaiderLinuxLauncher::walkthroughClicked() {
    QListWidgetItem *selectedItem = ui->listWidgetModds->currentItem();
    int id = selectedItem->data(Qt::UserRole).toInt();
    if (id != 0) {
        ui->walkthroughWebEngineView->setHtml(controller.getWalkthrough(id));
        ui->walkthroughWebEngineView->show();
        ui->stackedWidget->setCurrentWidget(
                ui->stackedWidget->findChild<QWidget*>("walkthrough"));
    }
}

void TombRaiderLinuxLauncher::backClicked() {
    ui->infoWebEngineView->setHtml("");
    ui->walkthroughWebEngineView->setHtml("");
    if (ui->stackedWidget->currentWidget() ==
        ui->stackedWidget->findChild<QWidget*>("info")) {
        ui->stackedWidget->setCurrentWidget(
            ui->stackedWidget->findChild<QWidget*>("select"));
    } else if (ui->stackedWidget->currentWidget() ==
        ui->stackedWidget->findChild<QWidget*>("walkthrough")) {
        ui->stackedWidget->setCurrentWidget(
            ui->stackedWidget->findChild<QWidget*>("select"));
    }
}

void TombRaiderLinuxLauncher::workTick() {
    int value = ui->progressBar->value();
    ui->progressBar->setValue(value + 1);
    qDebug() << ui->progressBar->value() << "%";
    if (ui->progressBar->value() >= 100) {
        QListWidgetItem *selectedItem = ui->listWidgetModds->currentItem();
        int id = selectedItem->data(Qt::UserRole).toInt();
        if (id < 0) {  // its the original game
            ui->pushButtonLink->setEnabled(true);
            ui->pushButtonInfo->setEnabled(false);
            ui->pushButtonDownload->setEnabled(false);
            selectedItem->setData(Qt::UserRole + 1, QVariant(true));
        } else if (id > 0) {  // do not know what to do with 0
            ui->pushButtonLink->setEnabled(true);
            ui->pushButtonInfo->setEnabled(true);
            ui->pushButtonDownload->setEnabled(false);
        }
        ui->stackedWidgetBar->setCurrentWidget(
            ui->stackedWidgetBar->findChild<QWidget*>("navigate"));
        ui->listWidgetModds->setEnabled(true);
    }
}

void TombRaiderLinuxLauncher::downloadError(int status) {
    ui->progressBar->setValue(0);
    ui->pushButtonLink->setEnabled(true);
    ui->pushButtonInfo->setEnabled(true);
    ui->pushButtonDownload->setEnabled(true);
    ui->stackedWidgetBar->setCurrentWidget(
        ui->stackedWidgetBar->findChild<QWidget*>("navigate"));
    QMessageBox msgBox;
    msgBox.setWindowTitle("Error");
    if (status == 1) {
        msgBox.setText("No internet");
    } else if (status == 2) {
        msgBox.setText("You seem to be missing ssl keys");
    } else {
        msgBox.setText("Could not connect");
    }
    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.setDefaultButton(QMessageBox::Ok);
    msgBox.exec();
}

TombRaiderLinuxLauncher::~TombRaiderLinuxLauncher() {
    delete ui;
}
