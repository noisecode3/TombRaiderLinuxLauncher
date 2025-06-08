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

    //
    connect(&Controller::getInstance(), SIGNAL(controllerReloadLevelList()),
        this, SLOT(loadMoreCovers()));

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

    connect(ui->comboBoxClass, &QComboBox::currentTextChanged,
            this, &TombRaiderLinuxLauncher::filterByClass);
    connect(ui->comboBoxType, &QComboBox::currentTextChanged,
            this, &TombRaiderLinuxLauncher::filterByType);
    connect(ui->comboBoxDifficulty, &QComboBox::currentTextChanged,
            this, &TombRaiderLinuxLauncher::filterByDifficulty);
    connect(ui->comboBoxDuration, &QComboBox::currentTextChanged,
            this, &TombRaiderLinuxLauncher::filterByDuration);

    model = new LevelListModel(this);
    ui->listViewLevels->setModel(model);
    CardItemDelegate* delegate = new CardItemDelegate(ui->listViewLevels);
    ui->listViewLevels->setItemDelegate(delegate);
    ui->listViewLevels->setSpacing(8);
    connect(
        ui->listViewLevels->selectionModel(),
        &QItemSelectionModel::currentChanged,
        this,
        &TombRaiderLinuxLauncher::onCurrentItemChanged);

    // Read settings
    QString value = m_settings.value("setup").toString();
    if (value != "yes") {
        setup();
    } else {
        readSavedSettings();
    }
}

void TombRaiderLinuxLauncher::generateList(const QList<int>& availableGames) {
    model->setLevels();
}

void TombRaiderLinuxLauncher::sortByTitle() {
    model->sortItems(model->compareTitles);
}

void TombRaiderLinuxLauncher::sortByDifficulty() {
    model->sortItems(model->compareDifficulties);
}

void TombRaiderLinuxLauncher::sortByDuration() {
    model->sortItems(model->compareDurations);
}

void TombRaiderLinuxLauncher::sortByClass() {
    model->sortItems(model->compareClasses);
}

void TombRaiderLinuxLauncher::sortByType() {
    model->sortItems(model->compareTypes);
}

void TombRaiderLinuxLauncher::sortByReleaseDate() {
    model->sortItems(model->compareReleaseDates);
}

void TombRaiderLinuxLauncher::filterByClass(const QString& class_) {
    model->filterClass(class_);
}

void TombRaiderLinuxLauncher::filterByType(const QString& type) {
    model->filterType(type);
}

void TombRaiderLinuxLauncher::filterByDifficulty(const QString& difficulty) {
    model->filterDifficulty(difficulty);
}

void TombRaiderLinuxLauncher::filterByDuration(const QString& duration) {
    model->filterDuration(duration);
}

void TombRaiderLinuxLauncher::readSavedSettings() {
    const QString gamePathValue = m_settings.value("gamePath").toString();
    ui->tableWidgetSetup->item(0, 0)->setText(gamePathValue);
    qDebug() << "Read game path value:" << gamePathValue;
    const QString levelPathValue = m_settings.value("levelPath").toString();
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
    const QString s = "/.steam/root/steamapps/common/";
    const QString l = "/.local/share/TombRaiderLinuxLauncher";
    ui->gamePathEdit->setText(homeDir + s);
    ui->levelPathEdit->setText(homeDir + l);
}

void TombRaiderLinuxLauncher::originalSelected(qint64 id) {
    if (id != 0) {
        // the game directory was a symbolic link and it has a level directory
        if ((controller.checkGameDirectory(id) == 2)
                && (controller.getItemState(id) == 1)) {
            ui->pushButtonLink->setEnabled(true);
            ui->pushButtonDownload->setEnabled(false);
        } else {
            ui->pushButtonLink->setEnabled(false);
            ui->pushButtonDownload->setEnabled(true);
        }
        ui->pushButtonInfo->setEnabled(false);
    }
}

void TombRaiderLinuxLauncher::levelDirSelected(qint64 id) {
    if (id != 0) {
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
        qint64 id = model->getLid(current);
        if (id < 0) {  // its the original game
            originalSelected(id);
        } else {
            levelDirSelected(id);
        }
        ui->lineEditCustomCommand->setEnabled(true);
        ui->lineEditCustomCommand->setText(
            m_settings.value(QString("level%1/CustomCommand")
                .arg(id)).toString());
        ui->lineEditEnvironmentVariables->setEnabled(true);
        ui->lineEditEnvironmentVariables->setText(
            m_settings.value(QString("level%1/EnvironmentVariables")
                .arg(id)).toString());
        ui->comboBoxRunnerType->setEnabled(true);
        ui->comboBoxRunnerType->setCurrentIndex(
            m_settings.value(QString("level%1/RunnerType")
                .arg(id)).toInt());
        ui->commandLinkButtonLSSave->setEnabled(true);
        ui->commandLinkButtonLSReset->setEnabled(true);
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
    QModelIndex current = ui->listViewLevels->currentIndex();
    if (current.isValid()) {
        qint64 id = model->getLid(current);
        if (id != 0) {
            if (m_settings.value(QString("level%1/RunnerType").arg(id)) == 2) {
                Model::getInstance().runWine(id);
            } else {
                if (!controller.link(id)) {
                    qDebug() << "link error";
                }
                QApplication::quit();
            }
        }
    }
}

void TombRaiderLinuxLauncher::downloadClicked() {
    QModelIndex current = ui->listViewLevels->currentIndex();
    if (current.isValid()) {
        qint64 id = model->getLid(current);
        if (id < 0) {
            ui->listViewLevels->setEnabled(false);
            ui->progressBar->setValue(0);
            ui->stackedWidgetBar->setCurrentWidget(
            ui->stackedWidgetBar->findChild<QWidget*>("progress"));
            // debugStop(QString("%1").arg(id*(-1)));
            controller.setupGame(id*(-1));
        } else if (id > 0) {
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
        qint64 id = model->getLid(current);
        if (id != 0) {
            InfoData info = controller.getInfo(id);
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
        qint64 id = model->getLid(current);
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
    model->loadMoreCovers();
}

void TombRaiderLinuxLauncher::workTick() {
    int value = ui->progressBar->value();
    ui->progressBar->setValue(value + 1);
    qDebug() << ui->progressBar->value() << "%";
    if (ui->progressBar->value() >= 100) {
        QModelIndex current = ui->listViewLevels->currentIndex();
        if (current.isValid()) {
            qint64 id = model->getLid(current);
            if (id < 0) {  // its the original game
                ui->pushButtonLink->setEnabled(true);
                ui->pushButtonInfo->setEnabled(false);
                ui->pushButtonDownload->setEnabled(false);
            } else if (id > 0) {
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

void TombRaiderLinuxLauncher::GlobalSaveClicked() {
    const QString newLevelPath = ui->tableWidgetSetup->item(1, 0)->text();
    const QString newGamePath = ui->tableWidgetSetup->item(0, 0)->text();

    const QString oldLevelPath = m_settings.value("levelPath").toString();
    const QString oldGamePath = m_settings.value("gamePath").toString();

    if ((newLevelPath != oldLevelPath) || (newGamePath != oldGamePath)) {
        m_settings.setValue("levelPath" , newLevelPath);
        m_settings.setValue("gamePath" , newGamePath);

        controller.setup(newLevelPath, newGamePath);
    }
}
void TombRaiderLinuxLauncher::GlobalResetClicked() {
    ui->tableWidgetSetup->item(0, 0)->setText(
        m_settings.value("gamePath").toString());
    ui->tableWidgetSetup->item(1, 0)->setText(
        m_settings.value("levelPath").toString());
}

void TombRaiderLinuxLauncher::LevelSaveClicked() {
    QModelIndex current = ui->listViewLevels->currentIndex();
    if (current.isValid()) {
        qint64 id = model->getLid(current);

        m_settings.setValue(QString("level%1/CustomCommand")
            .arg(id), ui->lineEditCustomCommand->text());

        m_settings.setValue(QString("level%1/EnvironmentVariables")
            .arg(id), ui->lineEditEnvironmentVariables->text());

        m_settings.setValue(QString("level%1/RunnerType")
            .arg(id), ui->comboBoxRunnerType->currentIndex());
    }
}

void TombRaiderLinuxLauncher::LevelResetClicked() {
}

TombRaiderLinuxLauncher::~TombRaiderLinuxLauncher() {
    delete ui;
}
