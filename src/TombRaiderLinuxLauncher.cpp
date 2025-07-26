/* TombRaiderLinuxLauncher
 * Martin Bångens Copyright (C) 2025
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include "../src/TombRaiderLinuxLauncher.hpp"
#include "ui_TombRaiderLinuxLauncher.h"
#include <qlineedit.h>


TombRaiderLinuxLauncher::TombRaiderLinuxLauncher(QWidget *parent)
    : QMainWindow(parent) {
    ui = new Ui::TombRaiderLinuxLauncher;
    ui->setupUi(this);

    // Button signal connections
    // List tab
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

    // Settings tab
    connect(ui->commandLinkButtonGSSave, SIGNAL(clicked()),
        this, SLOT(GlobalSaveClicked()));
    connect(ui->commandLinkButtonGSReset, SIGNAL(clicked()),
        this, SLOT(GlobalResetClicked()));
    connect(ui->commandLinkButtonLSSave, SIGNAL(clicked()),
        this, SLOT(LevelSaveClicked()));
    connect(ui->commandLinkButtonLSReset, SIGNAL(clicked()),
        this, SLOT(LevelResetClicked()));

    // Progress bar signal connection
    connect(&Controller::getInstance(), SIGNAL(controllerTickSignal()),
        this, SLOT(workTick()));

    // Arrive with next batch of level icons
    connect(&Controller::getInstance(), SIGNAL(controllerReloadLevelList()),
        this, SLOT(loadMoreCovers()));

    // Thread work done signal connections
    connect(&Controller::getInstance(),
        SIGNAL(controllerGenerateList(const QList<int>&)),
        this, SLOT(generateList(const QList<int>&)));

    // Error signal connections
    connect(&Controller::getInstance(), SIGNAL(controllerDownloadError(int)),
        this, SLOT(downloadError(int)));

    // Loading done signal connections
    connect(&Controller::getInstance(), SIGNAL(controllerLoadingDone()),
        this, SLOT(UpdateLevelDone()));

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

    QIcon arrowDownIcon(":/icons/down-arrow.svg");
    QIcon arrowUpIcon(":/icons/up-arrow.svg");

    filterButton_p->setIcon(arrowDownIcon);
    filterButton_p->setIconSize(QSize(16, 16));

    connect(ui->radioButtonLevelName, &QRadioButton::clicked,
            this, &TombRaiderLinuxLauncher::sortByTitle);
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
    connect(ui->checkBoxOriginal, &QRadioButton::clicked,
            this, &TombRaiderLinuxLauncher::showtOriginal);

    connect(ui->comboBoxClass, &QComboBox::currentTextChanged,
            this, &TombRaiderLinuxLauncher::filterByClass);
    connect(ui->comboBoxType, &QComboBox::currentTextChanged,
            this, &TombRaiderLinuxLauncher::filterByType);
    connect(ui->comboBoxDifficulty, &QComboBox::currentTextChanged,
            this, &TombRaiderLinuxLauncher::filterByDifficulty);
    connect(ui->comboBoxDuration, &QComboBox::currentTextChanged,
            this, &TombRaiderLinuxLauncher::filterByDuration);

    levelListModel = new LevelListModel(this);
    ui->listViewLevels->setModel(levelListModel);
    CardItemDelegate* delegate = new CardItemDelegate(ui->listViewLevels);
    ui->listViewLevels->setItemDelegate(delegate);
    ui->listViewLevels->setSpacing(8);
    ui->listViewLevels->setMouseTracking(true);
    connect(
        ui->listViewLevels->selectionModel(),
        &QItemSelectionModel::currentChanged,
        this,
        &TombRaiderLinuxLauncher::onCurrentItemChanged);


    connect(ui->commandLinkButton, &QCommandLinkButton::clicked, this, [=]() {
        QString searchText = ui->lineEditSearch->text();
        qDebug() << "You searched for:" << searchText;
        levelListModel->filterSearch(searchText);
    });

    connect(ui->checkBoxInstalled, &QCheckBox::clicked, this, [=]() {
        levelListModel->filterInstalled();
    });

    m_loadingIndicatorWidget = new LoadingIndicator(ui->loading);
    m_loadingIndicatorWidget->setFixedSize(64, 64);
    ui->verticalLayout_15->
            addWidget(m_loadingIndicatorWidget, 0, Qt::AlignCenter);
    m_loadingIndicatorWidget->show();
    ui->stackedWidget->setCurrentWidget(
        ui->stackedWidget->findChild<QWidget*>("loading"));
    m_loadingDoneGoTo = "select";

    ui->Tabs->setTabEnabled(
        ui->Tabs->indexOf(ui->Tabs->findChild<QWidget*>("Modding")), false);

    // Read settings
    QString value = settings.value("setup").toString();
    if (value != "yes") {
        setup();
    } else {
        readSavedSettings();
    }
}

void TombRaiderLinuxLauncher::generateList(const QList<int>& availableGames) {
    // Update list only when 24 hours past
    QDateTime now = QDateTime::currentDateTime();
    QString lastUpdatedStr = settings.value("lastUpdated").toString();
    QDateTime lastUpdated = QDateTime::fromString(lastUpdatedStr, Qt::ISODate);

    // Define today's noon
    QDateTime todayNoon = QDateTime(QDate::currentDate(), QTime(12, 0));
    // Define the next valid time after noon
    QDateTime targetTime;

    if (now < todayNoon) {
        // It's before today's noon -> check against yesterday's noon
        targetTime = todayNoon.addDays(-1);
    } else {
        // It's after today's noon -> check against today's noon
        targetTime = todayNoon;
    }

    if (!lastUpdated.isValid() || lastUpdated < targetTime) {
        controller.syncLevels();
        settings.setValue("lastUpdated", now.toString(Qt::ISODate));
        m_availableGames = availableGames;
    } else {
        levelListModel->setLevels(availableGames);
        setInstalled();
        ui->stackedWidget->setCurrentWidget(
            ui->stackedWidget->findChild<QWidget*>("select"));
    }
}

void TombRaiderLinuxLauncher::setInstalled() {
    QStringList keys = settings.allKeys();
    QHash<int, bool> installedLevel;
    QHash<int, bool> installedGame;
    for (const QString &key : keys) {
        if (key.startsWith("installed/game")) {
            qint64 id = key.mid(QString("installed/game").length()).toInt();
            installedGame.insert(id, settings.value(key).toBool());
        } else if (key.startsWith("installed/level")) {
            qint64 id = key.mid(QString("installed/level").length()).toInt();
            installedLevel.insert(id, settings.value(key).toBool());
        }
    }
    levelListModel->setInstalledListOriginal(installedGame);
    levelListModel->setInstalledList(installedLevel);
}

void TombRaiderLinuxLauncher::sortByTitle() {
    levelListModel->sortItems(levelListModel->compareTitles);
}

void TombRaiderLinuxLauncher::sortByDifficulty() {
    levelListModel->sortItems(levelListModel->compareDifficulties);
}

void TombRaiderLinuxLauncher::sortByDuration() {
    levelListModel->sortItems(levelListModel->compareDurations);
}

void TombRaiderLinuxLauncher::sortByClass() {
    levelListModel->sortItems(levelListModel->compareClasses);
}

void TombRaiderLinuxLauncher::sortByType() {
    levelListModel->sortItems(levelListModel->compareTypes);
}

void TombRaiderLinuxLauncher::sortByReleaseDate() {
    levelListModel->sortItems(levelListModel->compareReleaseDates);
}

void TombRaiderLinuxLauncher::showtOriginal() {
    levelListModel->showOriginal();
}

void TombRaiderLinuxLauncher::filterByClass(const QString& class_) {
    levelListModel->filterClass(class_);
}

void TombRaiderLinuxLauncher::filterByType(const QString& type) {
    levelListModel->filterType(type);
}

void TombRaiderLinuxLauncher::filterByDifficulty(const QString& difficulty) {
    levelListModel->filterDifficulty(difficulty);
}

void TombRaiderLinuxLauncher::filterByDuration(const QString& duration) {
    levelListModel->filterDuration(duration);
}

void TombRaiderLinuxLauncher::readSavedSettings() {
    const QString gamePathValue = settings.value("gamePath").toString();
    ui->tableWidgetSetup->item(0, 0)->setText(gamePathValue);
    qDebug() << "Read game path value:" << gamePathValue;
    const QString levelPathValue = settings.value("levelPath").toString();
    ui->tableWidgetSetup->item(1, 0)->setText(levelPathValue);
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
    const QString s = "/.steam/steam/steamapps/common/";
    const QString l = "/.local/share/TombRaiderLinuxLauncher";
    ui->gamePathEdit->setText(homeDir + s);
    ui->levelPathEdit->setText(homeDir + l);
}

void TombRaiderLinuxLauncher::levelDirSelected(qint64 id) {
    if (id != 0) {
        int state = controller.getItemState(id);
        // Activate or deactivate pushbuttons based on the selected item
        qDebug() << "Selected: " << id << Qt::endl;
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
        } else {
            ui->pushButtonLink->setEnabled(false);
            ui->pushButtonInfo->setEnabled(false);
            ui->pushButtonDownload->setEnabled(false);
        }
    }
}

void TombRaiderLinuxLauncher::onCurrentItemChanged(
        const QModelIndex &current, const QModelIndex &previous) {
    if (current.isValid()) {
        qint64 id = levelListModel->getLid(current);
        if (levelListModel->getListType()) {  // its the original game
            if (levelListModel->getInstalled(current)) {
                ui->pushButtonLink->setEnabled(true);
                ui->pushButtonDownload->setEnabled(false);
            } else {
                ui->pushButtonLink->setEnabled(false);
                ui->pushButtonDownload->setEnabled(true);
            }
            ui->pushButtonInfo->setEnabled(false);
        } else {
            levelDirSelected(id);
        }
        // ui->lineEditCustomCommand->setEnabled(true);
        // ui->lineEditCustomCommand->setText(
        //     m_settings.value(QString("level%1/CustomCommand")
        //         .arg(id)).toString());
        ui->lineEditEnvironmentVariables->setEnabled(true);
        ui->lineEditEnvironmentVariables->setText(
            settings.value(QString("level%1/EnvironmentVariables")
                .arg(id)).toString());
        ui->comboBoxRunnerType->setEnabled(true);
        ui->comboBoxRunnerType->setCurrentIndex(
            settings.value(QString("level%1/RunnerType")
                .arg(id)).toInt());
        ui->commandLinkButtonLSSave->setEnabled(true);
        ui->commandLinkButtonLSReset->setEnabled(true);
    }
}

void TombRaiderLinuxLauncher::setOptionsClicked() {
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
        ui->Tabs->indexOf(ui->Tabs->findChild<QWidget*>("Modding")), false);
    ui->Tabs->show();
    ui->Tabs->setCurrentIndex(
        ui->Tabs->indexOf(ui->Tabs->findChild<QWidget*>("Levels")));
    ui->setupStackedWidget->setCurrentWidget(
        ui->setupStackedWidget->findChild<QWidget*>("settings"));

    readSavedSettings();
}

QStringList TombRaiderLinuxLauncher::parsToArg(const QString& str) {
    return str.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
}

QVector<QPair<QString, QString>>
    TombRaiderLinuxLauncher::parsToEnv(const QString& str) {
    QRegularExpression re("(\\w+)\\s*=\\s*\"([^\"]*)\"");
    QRegularExpressionMatchIterator it = re.globalMatch(str);

    QVector<QPair<QString, QString>> envList;
    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        // VARIABLE="some text here"
        QPair<QString, QString> env;
        env.first = match.captured(1);  // VARIABLE
        env.second = match.captured(2);  // "some text here"
        envList.append(env);
    }
    return envList;
}

void TombRaiderLinuxLauncher::linkClicked() {
    qDebug() << "linkClicked()";
    QModelIndex current = ui->listViewLevels->currentIndex();
    if (current.isValid()) {
        qint64 id = levelListModel->getLid(current);
        if (id != 0) {
            qint64 type = settings.value(
                    QString("level%1/RunnerType").arg(id)).toInt();
            QLineEdit* input = ui->lineEditEnvironmentVariables;
            qDebug() << "Type was: " << type;

            if (type == 0) {
                QVector<QPair<QString, QString>> envList =
                        parsToEnv(input->text());
                if (ui->checkBoxSetup->isChecked()) {
                    Model::getInstance().setUmuSetup();
                }
                Model::getInstance().setUmuEnv(envList);
                Model::getInstance().runUmu(id);

            } else if (type == 1) {
                QVector<QPair<QString, QString>> envList =
                        parsToEnv(input->text());
                if (ui->checkBoxSetup->isChecked()) {
                    Model::getInstance().setUmuSetup();
                }
                Model::getInstance().setWineEnv(envList);
                Model::getInstance().runWine(id);

            } else if (type == 2) {
                QStringList argList = parsToArg(input->text());
                Model::getInstance().runLutris(argList);

            } else if (type == 3) {
                if (!controller.link(id)) {
                    qDebug() << "link error";
                }
                QStringList argList = parsToArg(input->text());
                Model::getInstance().runLutris(argList);

            } else if (type == 4) {
                Model::getInstance().runSteam(id);

            } else if (type == 5) {
                if (levelListModel->getListType()) {
                    id = (-1)*id;
                }
                if (!controller.link(id)) {
                    qDebug() << "link error";
                }
                QApplication::quit();

            } else if (type == 6) {
                if (levelListModel->getListType()) {
                    id = (-1)*id;
                }
                if (!controller.link(id)) {
                    qDebug() << "link error";
                }

            } else if (type == 7) {
                Model::getInstance().runBash(id);
            }
        }
    }
}

void TombRaiderLinuxLauncher::downloadClicked() {
    QModelIndex current = ui->listViewLevels->currentIndex();
    if (current.isValid()) {
        qint64 id = levelListModel->getLid(current);
        if (levelListModel->getListType()) {
            ui->listViewLevels->setEnabled(false);
            ui->progressBar->setValue(0);
            ui->stackedWidgetBar->setCurrentWidget(
            ui->stackedWidgetBar->findChild<QWidget*>("progress"));
            controller.setupGame(id);
        } else {
            ui->listViewLevels->setEnabled(false);
            ui->progressBar->setValue(0);
            ui->stackedWidgetBar->setCurrentWidget(
            ui->stackedWidgetBar->findChild<QWidget*>("progress"));
            controller.setupLevel(id);
        }
    }
}

void TombRaiderLinuxLauncher::infoClicked() {
    QModelIndex current = ui->listViewLevels->currentIndex();
    if (current.isValid()) {
        qint64 id = levelListModel->getLid(current);
        if (id != 0) {
            InfoData info = controller.getInfo(id);
            if (info.m_body == "" && info.m_imageList.size() == 0) {
                m_loadingIndicatorWidget->show();
                ui->stackedWidget->setCurrentWidget(
                    ui->stackedWidget->findChild<QWidget*>("loading"));
                controller.updateLevel(id);
                m_loadingDoneGoTo = "info";
                return;
            }

            ui->infoWebEngineView->setHtml(info.m_body);

            // Get the vertical scrollbar size
            // to center the images for all themes
            int scrollbarWidth = ui->infoListWidget
                ->style()->pixelMetric(QStyle::PM_ScrollBarExtent);
            QMargins margins = ui->infoListWidget->contentsMargins();
            int left = margins.left();
            int right = margins.right();

            QListWidget *w = ui->infoListWidget;
            w->setMinimumWidth(left+502+scrollbarWidth+right);
            w->setMaximumWidth(left+502+scrollbarWidth+right);

            w->setViewMode(QListView::IconMode);
            w->setIconSize(QSize(502, 377));
            w->setDragEnabled(false);
            w->setAcceptDrops(false);
            w->setDragDropMode(QAbstractItemView::NoDragDrop);
            w->setDefaultDropAction(Qt::IgnoreAction);
            w->setSelectionMode(QAbstractItemView::NoSelection);
            w->clear();
            for (int i = 0; i < info.m_imageList.size(); ++i) {
                const QIcon &icon = info.m_imageList.at(i);
                QListWidgetItem *item = new QListWidgetItem(icon, "");
                item->setSizeHint(QSize(502, 377));
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
}

void TombRaiderLinuxLauncher::walkthroughClicked() {
    QModelIndex current = ui->listViewLevels->currentIndex();
    if (current.isValid()) {
        qint64 id = levelListModel->getLid(current);
        if (id != 0) {
            QWebEngineView* w = ui->walkthroughWebEngineView;
            w->setHtml(controller.getWalkthrough(id));
            w->show();
            ui->stackedWidget->setCurrentWidget(
                ui->stackedWidget->findChild<QWidget*>("walkthrough"));
        }
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

void TombRaiderLinuxLauncher::loadMoreCovers() {
    levelListModel->loadMoreCovers();
}

void TombRaiderLinuxLauncher::workTick() {
    int value = ui->progressBar->value();
    ui->progressBar->setValue(value + 1);
    qDebug() << ui->progressBar->value() << "%";
    if (ui->progressBar->value() >= 100) {
        QModelIndex current = ui->listViewLevels->currentIndex();
        if (current.isValid()) {
            qint64 id = levelListModel->getLid(current);
            levelListModel->setInstalled(current);
            if (levelListModel->getListType()) {  // its the original game
                settings.setValue(
                        QString("installed/game%1").arg(id),
                        "true");
                ui->pushButtonLink->setEnabled(true);
                ui->pushButtonInfo->setEnabled(false);
                ui->pushButtonDownload->setEnabled(false);
            } else {
                settings.setValue(
                        QString("installed/level%1").arg(id),
                        "true");
                ui->pushButtonLink->setEnabled(true);
                ui->pushButtonInfo->setEnabled(true);
                ui->pushButtonDownload->setEnabled(false);
            }
            ui->stackedWidgetBar->setCurrentWidget(
                ui->stackedWidgetBar->findChild<QWidget*>("navigate"));
            ui->listViewLevels->setEnabled(true);
        }
    }
}

void TombRaiderLinuxLauncher::downloadError(int status) {
    ui->progressBar->setValue(0);
    ui->pushButtonLink->setEnabled(true);
    ui->pushButtonInfo->setEnabled(true);
    ui->pushButtonDownload->setEnabled(true);
    ui->stackedWidgetBar->setCurrentWidget(
        ui->stackedWidgetBar->findChild<QWidget*>("navigate"));
    ui->listViewLevels->setEnabled(true);
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

void TombRaiderLinuxLauncher::UpdateLevelDone() {
    m_loadingIndicatorWidget->hide();
    if (m_loadingDoneGoTo == "select") {
            levelListModel->setLevels(m_availableGames);
            setInstalled();
        ui->stackedWidget->setCurrentWidget(
            ui->stackedWidget->findChild<QWidget*>("select"));
    } else if (m_loadingDoneGoTo == "info") {
        QModelIndex current = ui->listViewLevels->currentIndex();
        if (current.isValid()) {
            qint64 id = levelListModel->getLid(current);
            if (id != 0) {
                InfoData info = controller.getInfo(id);
                if (!(info.m_body == "" && info.m_imageList.size() == 0)) {
                    infoClicked();
                } else {
                    ui->stackedWidget->setCurrentWidget(
                        ui->stackedWidget->findChild<QWidget*>("select"));
                }
            }
        }
    } else {
        qDebug() << "Forgot to get UpdateLeveDoneTo maybe?";
    }
}

void TombRaiderLinuxLauncher::GlobalSaveClicked() {
    const QString newLevelPath = ui->tableWidgetSetup->item(1, 0)->text();
    const QString newGamePath = ui->tableWidgetSetup->item(0, 0)->text();

    const QString oldLevelPath = settings.value("levelPath").toString();
    const QString oldGamePath = settings.value("gamePath").toString();

    if ((newLevelPath != oldLevelPath) || (newGamePath != oldGamePath)) {
        settings.setValue("levelPath" , newLevelPath);
        settings.setValue("gamePath" , newGamePath);

        controller.setup(newLevelPath, newGamePath);
    }
}

void TombRaiderLinuxLauncher::GlobalResetClicked() {
    ui->tableWidgetSetup->item(0, 0)->setText(
        settings.value("gamePath").toString());
    ui->tableWidgetSetup->item(1, 0)->setText(
        settings.value("levelPath").toString());
}

void TombRaiderLinuxLauncher::LevelSaveClicked() {
    QModelIndex current = ui->listViewLevels->currentIndex();
    if (current.isValid()) {
        qint64 id = levelListModel->getLid(current);

        // settings.setValue(QString("level%1/CustomCommand")
        //     .arg(id), ui->lineEditCustomCommand->text());

        settings.setValue(QString("level%1/EnvironmentVariables")
            .arg(id), ui->lineEditEnvironmentVariables->text());

        settings.setValue(QString("level%1/RunnerType")
            .arg(id), ui->comboBoxRunnerType->currentIndex());
    }
}

void TombRaiderLinuxLauncher::LevelResetClicked() {
}

TombRaiderLinuxLauncher::~TombRaiderLinuxLauncher() {
    delete ui;
    QApplication::quit();
}
