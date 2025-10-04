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
#include "../src/globalTypes.hpp"
#include "ui_TombRaiderLinuxLauncher.h"
#include <QLineEdit>

TombRaiderLinuxLauncher::TombRaiderLinuxLauncher(QWidget *parent)
    : QMainWindow(parent) {
    ui = new Ui::TombRaiderLinuxLauncher;
    ui->setupUi(this);

    // Button signal connections
    // List tab
    connect(ui->pushButtonRun, SIGNAL(clicked()), this, SLOT(runClicked()));
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
        SIGNAL(controllerGenerateList(QList<int>)),
        this, SLOT(generateList(QList<int>)));

    // Error signal connections
    connect(&Controller::getInstance(), SIGNAL(controllerDownloadError(int)),
        this, SLOT(downloadError(int)));

    // Loading done signal connections
    connect(&Controller::getInstance(), SIGNAL(controllerLoadingDone()),
        this, SLOT(updateLevelDone()));

    // Loading done signal connections
    connect(&Controller::getInstance(), SIGNAL(controllerRunningDone()),
        this, SLOT(runningLevelDone()));

    // Set init state
    ui->pushButtonRun->setEnabled(false);
    ui->pushButtonInfo->setEnabled(false);
    ui->pushButtonDownload->setEnabled(false);
    QWidget* filter_p = ui->filter;
    QPushButton* filterButton_p = ui->filterButton;
    filter_p->setVisible(false);

    connect(ui->filterButton, &QPushButton::clicked,
            this, [filter_p, filterButton_p]() -> void {
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
    levelListProxy = new LevelListProxy(this);
    levelListProxy->setSourceModel(levelListModel);
    ui->listViewLevels->setModel(levelListProxy);

    CardItemDelegate* delegate = new CardItemDelegate(ui->listViewLevels);
    ui->listViewLevels->setItemDelegate(delegate);
    ui->listViewLevels->setSpacing(8);
    ui->listViewLevels->setMouseTracking(true);
    connect(
        ui->listViewLevels->selectionModel(),
        &QItemSelectionModel::currentChanged,
        this,
        &TombRaiderLinuxLauncher::onCurrentItemChanged);

    connect(ui->checkBoxInstalled, &QCheckBox::toggled,
            levelListProxy, &LevelListProxy::setInstalledFilter);

    connect(ui->comboBoxSearch, &QComboBox::currentTextChanged,
            levelListProxy, &LevelListProxy::setSearchType);

    connect(ui->lineEditSearch, &QLineEdit::textChanged,
            levelListProxy, &LevelListProxy::setSearchFilter);

    m_loadingIndicatorWidget = new LoadingIndicator(ui->loading);
    m_loadingIndicatorWidget->setFixedSize(64, 64);
    ui->verticalLayout_15->
            addWidget(m_loadingIndicatorWidget, 0, Qt::AlignCenter);
    m_loadingIndicatorWidget->show();
    ui->stackedWidget->setCurrentWidget(
        ui->stackedWidget->findChild<QWidget*>("loading"));
    m_loadingDoneGoTo = "select";

    m_dialogWidget = new Dialog(ui->dialog);
    ui->verticalLayout_17->addWidget(m_dialogWidget);
    connect(m_dialogWidget, &Dialog::okClicked, this, [this]() {
        qDebug() << "OK clicked, selected:" << m_dialogWidget->selectedOption();
        // handle it here
    });


    ui->Tabs->setTabEnabled(
        ui->Tabs->indexOf(ui->Tabs->findChild<QWidget*>("Modding")), false);

    ui->Tabs->setTabEnabled(
        ui->Tabs->indexOf(ui->Tabs->findChild<QWidget*>("Controller")), false);

    // Read settings
    QString value = g_settings.value("setup").toString();
    if (value != "yes") {
        setup();
    } else {
        readSavedSettings();
    }

}

void TombRaiderLinuxLauncher::setList() {
    QVector<QSharedPointer<ListItemData>> list;
    controller.getList(&list);
    InstalledStatus installedStatus = getInstalled();

    for (const auto &item : list) {
        Q_ASSERT(item != nullptr);
        bool trle = installedStatus.trle.value(item->m_trle_id, false);
        item->m_installed = trle;
    }

    levelListModel->setLevels(list);

    if (m_ss.sort_id != 0) {
        if (m_ss.sort_id == 1) {
            ui->radioButtonLevelName->setChecked(true);
            levelListProxy->setSortMode(LevelListProxy::Title);
        } else if (m_ss.sort_id == 2) {
            ui->radioButtonDifficulty->setChecked(true);
            levelListProxy->setSortMode(LevelListProxy::Difficulty);
        } else if (m_ss.sort_id == 3) {
            ui->radioButtonDuration->setChecked(true);
            levelListProxy->setSortMode(LevelListProxy::Duration);
        } else if (m_ss.sort_id == 4) {
            ui->radioButtonClass->setChecked(true);
            levelListProxy->setSortMode(LevelListProxy::Class);
        } else if (m_ss.sort_id == 5) {
            ui->radioButtonType->setChecked(true);
            levelListProxy->setSortMode(LevelListProxy::Type);
        }
    }

    loadMoreCovers();
}

void TombRaiderLinuxLauncher::generateList(const QList<int>& availableGames) {
    // Update list only when 24 hours past
    QDateTime now = QDateTime::currentDateTime();
    QString lastUpdatedStr = g_settings.value("lastUpdated").toString();
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
        g_settings.setValue("lastUpdated", now.toString(Qt::ISODate));
        m_availableGames = availableGames;
    } else {
        setList();
        ui->stackedWidget->setCurrentWidget(
            ui->stackedWidget->findChild<QWidget*>("select"));
    }
}

InstalledStatus TombRaiderLinuxLauncher::getInstalled() {
    QStringList keys = g_settings.allKeys();
    InstalledStatus list;
    for (const auto &key : std::as_const(keys)) {
        if (key.startsWith("installed/game")) {
            quint64 id =
                    key.mid(QString("installed/game").length()).toUInt();
            list.game.insert(id, g_settings.value(key).toBool());
        } else if (key.startsWith("installed/level")) {
            quint64 id =
                    key.mid(QString("installed/level").length()).toUInt();
            list.trle.insert(id, g_settings.value(key).toBool());
        }
    }
    return list;
}

void TombRaiderLinuxLauncher::setStartupSetting(const StartupSetting ss) {
    bool setProxyCoversFirst = false;
    if (ss.installed == true) {
        ui->checkBoxInstalled->setChecked(true);
        setProxyCoversFirst = true;
    }
    if (ss.original == true) {
        ui->checkBoxOriginal->setChecked(true);
        setProxyCoversFirst = true;
    }
    if (ss.type_id != 0) {
        ui->comboBoxType->setCurrentIndex(ss.type_id);
        setProxyCoversFirst = true;
    }
    if (ss.class_id != 0) {
        ui->comboBoxClass->setCurrentIndex(ss.class_id);
        setProxyCoversFirst = true;
    }
    if (ss.difficulty_id != 0) {
        ui->comboBoxDifficulty->setCurrentIndex(ss.difficulty_id);
        setProxyCoversFirst = true;
    }
    if (ss.duration_id != 0) {
        ui->comboBoxDuration->setCurrentIndex(ss.duration_id);
        setProxyCoversFirst = true;
    }
    m_ss = ss;

    if (m_ss.sort_id != 0) {
        setProxyCoversFirst = true;
    }

    if (setProxyCoversFirst == true) {
        ui->listViewLevels->setProxyCoversFirst();
    }
}

void TombRaiderLinuxLauncher::sortByTitle() {
    levelListProxy->setSortMode(LevelListProxy::Title);
}
void TombRaiderLinuxLauncher::sortByDifficulty() {
    levelListProxy->setSortMode(LevelListProxy::Difficulty);
}
void TombRaiderLinuxLauncher::sortByDuration() {
    levelListProxy->setSortMode(LevelListProxy::Duration);
}
void TombRaiderLinuxLauncher::sortByClass() {
    levelListProxy->setSortMode(LevelListProxy::Class);
}
void TombRaiderLinuxLauncher::sortByType() {
    levelListProxy->setSortMode(LevelListProxy::Type);
}
void TombRaiderLinuxLauncher::sortByReleaseDate() {
    levelListProxy->setSortMode(LevelListProxy::ReleaseDate);
}

void TombRaiderLinuxLauncher::showtOriginal() {
    // levelListModel->showOriginal();
}

void TombRaiderLinuxLauncher::filterByClass(const QString &class_) {
    levelListProxy->setClassFilter(class_);
}
void TombRaiderLinuxLauncher::filterByType(const QString &type) {
    levelListProxy->setTypeFilter(type);
}
void TombRaiderLinuxLauncher::filterByDifficulty(const QString &difficulty) {
    levelListProxy->setDifficultyFilter(difficulty);
}
void TombRaiderLinuxLauncher::filterByDuration(const QString &duration) {
    levelListProxy->setDurationFilter(duration);
}

void TombRaiderLinuxLauncher::readSavedSettings() {
    const QString gamePathValue = g_settings.value("gamePath").toString();
    ui->tableWidgetSetup->item(0, 0)->setText(gamePathValue);
    qDebug() << "Read game path value:" << gamePathValue;
    const QString levelPathValue = g_settings.value("levelPath").toString();
    ui->tableWidgetSetup->item(1, 0)->setText(levelPathValue);
    qDebug() << "Read level path value:" << levelPathValue;
    controller.setup();
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
            ui->pushButtonRun->setEnabled(true);
            ui->pushButtonInfo->setEnabled(false);
            ui->pushButtonDownload->setEnabled(false);
        } else if (state == 2) {
            ui->pushButtonRun->setEnabled(true);
            ui->pushButtonInfo->setEnabled(true);
            ui->pushButtonDownload->setEnabled(false);
        } else if (state == 0) {
            ui->pushButtonRun->setEnabled(false);
            ui->pushButtonInfo->setEnabled(true);
            ui->pushButtonDownload->setEnabled(true);
        } else {
            ui->pushButtonRun->setEnabled(false);
            ui->pushButtonInfo->setEnabled(false);
            ui->pushButtonDownload->setEnabled(false);
        }
    }
}

void TombRaiderLinuxLauncher::onCurrentItemChanged(
        const QModelIndex &current, const QModelIndex &previous) {
    m_current = levelListProxy->mapToSource(current);
    if (m_current.isValid()) {
        qint64 id = levelListProxy->getLid(m_current);
        ui->lcdNumberLevelID->display(QString::number(id));
        if (levelListProxy->getItemType(m_current)) {  // its the original game
            if (levelListProxy->getInstalled(m_current)) {
                ui->pushButtonRun->setEnabled(true);
                ui->pushButtonDownload->setEnabled(false);
                ui->pushButtonDownload->setText("Remove");

            } else {
                ui->pushButtonRun->setEnabled(false);
                ui->pushButtonDownload->setText("Download and install");
                ui->pushButtonDownload->setEnabled(true);
            }
            ui->pushButtonInfo->setEnabled(false);
        } else {
            levelDirSelected(id);
        }
        ui->lineEditEnvironmentVariables->setEnabled(true);
        ui->lineEditEnvironmentVariables->setText(
            g_settings.value(QString("level%1/EnvironmentVariables")
                .arg(id)).toString());
        ui->comboBoxRunnerType->setEnabled(true);
        ui->comboBoxRunnerType->setCurrentIndex(
            g_settings.value(QString("level%1/RunnerType")
                .arg(id)).toInt());
        ui->pushButtonRun->setText(ui->comboBoxRunnerType->currentText());
        ui->commandLinkButtonLSSave->setEnabled(true);
        ui->commandLinkButtonLSReset->setEnabled(true);
    }
}

void TombRaiderLinuxLauncher::setOptionsClicked() {
    QString gamePath = ui->gamePathEdit->text();
    QString levelPath = ui->levelPathEdit->text();
    g_settings.setValue("gamePath" , gamePath);
    g_settings.setValue("levelPath" , levelPath);
    g_settings.setValue("setup" , "yes");

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
    static const QRegularExpression re = QRegularExpression("\\s+");
    QStringList list = str.split(re, Qt::SkipEmptyParts);
    return list;
}

QVector<QPair<QString, QString>>
    TombRaiderLinuxLauncher::parsToEnv(const QString& str) {
    static const QRegularExpression re("(\\w+)\\s*=\\s*\"([^\"]*)\"");
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

void TombRaiderLinuxLauncher::runClicked() {
    if (m_current.isValid()) {
        ui->listViewLevels->setEnabled(false);
        ui->groupBoxSearch->setEnabled(false);
        ui->groupBoxFilter->setEnabled(false);
        ui->groupBoxToggle->setEnabled(false);
        ui->groupBoxSort->setEnabled(false);
        ui->pushButtonRun->setEnabled(false);
        ui->checkBoxSetup->setEnabled(false);

        qint64 id = levelListProxy->getLid(m_current);
        if (levelListProxy->getItemType(m_current)) {
            id = (-1)*id;
        }
        if (id != 0) {
            qint64 type = g_settings.value(
                    QString("level%1/RunnerType").arg(id)).toInt();
            QLineEdit* input = ui->lineEditEnvironmentVariables;
            qDebug() << "Type was: " << type;
            RunnerOptions options;

            if (type == 0) {
                options.id = id;
                options.command = UMU;
                options.setup = ui->checkBoxSetup->isChecked();
                options.envList = parsToEnv(input->text());
                controller.run(options);
            } else if (type == 1) {
                options.id = id;
                options.command = WINE;
                options.setup = ui->checkBoxSetup->isChecked();
                options.envList = parsToEnv(input->text());
                controller.run(options);
            } else if (type == 2) {
                options.id = id;
                options.command = LUTRIS;
                options.arguments = parsToArg(input->text());
                controller.run(options);
            } else if (type == 3) {
                if (!controller.link(id)) {
                    qDebug() << "link error";
                    runningLevelDone();
                } else {
                    options.id = id;
                    options.command = LUTRIS;
                    options.arguments = parsToArg(input->text());
                    controller.run(options);
                }
            } else if (type == 4) {
                if (!controller.link(id)) {
                    qDebug() << "link error";
                    runningLevelDone();
                } else {
                    options.id = id;
                    options.command = STEAM;
                    controller.run(options);
                }
            } else if (type == 5) {
                if (!controller.link(id)) {
                    qDebug() << "link error";
                }
                QApplication::quit();
            } else if (type == 6) {
                if (!controller.link(id)) {
                    qDebug() << "link error";
                }
                runningLevelDone();
            } else if (type == 7) {
                options.id = id;
                options.command = BASH;
                controller.run(options);
            }
        }
    }
}

void TombRaiderLinuxLauncher::runningLevelDone() {
    ui->listViewLevels->setEnabled(true);
    ui->groupBoxSearch->setEnabled(true);
    ui->groupBoxFilter->setEnabled(true);
    ui->groupBoxToggle->setEnabled(true);
    ui->groupBoxSort->setEnabled(true);
    ui->pushButtonRun->setEnabled(true);
    ui->checkBoxSetup->setEnabled(true);
}

void TombRaiderLinuxLauncher::downloadClicked() {
    if (m_current.isValid()) {
        qint64 id = levelListProxy->getLid(m_current);
        qDebug() << "void TombRaiderLinuxLauncher" <<
                    "::downloadClicked() qint64 id: " << id;
        ui->listViewLevels->setEnabled(false);
        ui->groupBoxSearch->setEnabled(false);
        ui->groupBoxFilter->setEnabled(false);
        ui->groupBoxToggle->setEnabled(false);
        ui->groupBoxSort->setEnabled(false);
        ui->progressBar->setValue(0);
        ui->stackedWidgetBar->setCurrentWidget(
            ui->stackedWidgetBar->findChild<QWidget*>("progress"));
        if (levelListProxy->getItemType(m_current)) {
            controller.setupGame(id);
        } else {
            controller.setupLevel(id);
        }
    }
}

void TombRaiderLinuxLauncher::infoClicked() {
    if (m_current.isValid()) {
        qint64 id = levelListProxy->getLid(m_current);  // getLid(current);
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
    if (m_current.isValid()) {
        qint64 id = levelListProxy->getLid(m_current);
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
    if (!levelListModel->stop()) {
        QVector<QSharedPointer<ListItemData>> buffer =
                levelListModel->getDataBuffer(20);
        if (!buffer.isEmpty()) {
            controller.getCoverList(buffer);
        }
    }
    static bool firstTime = true;
    if (!firstTime) {
        levelListModel->reset();
    } else {
        firstTime = false;
    }
}

void TombRaiderLinuxLauncher::workTick() {
    int value = ui->progressBar->value();
    ui->progressBar->setValue(value + 1);
    qDebug() << ui->progressBar->value() << "%";
    if (ui->progressBar->value() >= 100) {
        if (m_current.isValid()) {
            qint64 id = levelListProxy->getLid(m_current);
            levelListModel->setInstalled(m_current);
            if (levelListProxy->getItemType(m_current)) {
                g_settings.setValue(
                        QString("installed/game%1").arg(id),
                        "true");
                ui->pushButtonRun->setEnabled(true);
                ui->pushButtonInfo->setEnabled(false);
                ui->pushButtonDownload->setEnabled(false);
            } else {
                g_settings.setValue(
                        QString("installed/level%1").arg(id),
                        "true");
                ui->pushButtonRun->setEnabled(true);
                ui->pushButtonInfo->setEnabled(true);
                ui->pushButtonDownload->setEnabled(false);
            }
            ui->stackedWidgetBar->setCurrentWidget(
                ui->stackedWidgetBar->findChild<QWidget*>("navigate"));
            ui->listViewLevels->setEnabled(true);
            ui->groupBoxSearch->setEnabled(true);
            ui->groupBoxFilter->setEnabled(true);
            ui->groupBoxToggle->setEnabled(true);
            ui->groupBoxSort->setEnabled(true);
        }
    }
}

void TombRaiderLinuxLauncher::downloadError(int status) {
    ui->progressBar->setValue(0);
    ui->pushButtonRun->setEnabled(true);
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

void TombRaiderLinuxLauncher::updateLevelDone() {
    m_loadingIndicatorWidget->hide();
    if (m_loadingDoneGoTo == "select") {
        setList();
        ui->stackedWidget->setCurrentWidget(
            ui->stackedWidget->findChild<QWidget*>("select"));
    } else if (m_loadingDoneGoTo == "info") {
        if (m_current.isValid()) {
            qint64 id = levelListProxy->getLid(m_current);
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

    const QString oldLevelPath = g_settings.value("levelPath").toString();
    const QString oldGamePath = g_settings.value("gamePath").toString();

    if ((newLevelPath != oldLevelPath) || (newGamePath != oldGamePath)) {
        g_settings.setValue("levelPath" , newLevelPath);
        g_settings.setValue("gamePath" , newGamePath);
        controller.setup();
    }
}

void TombRaiderLinuxLauncher::GlobalResetClicked() {
    ui->tableWidgetSetup->item(0, 0)->setText(
        g_settings.value("gamePath").toString());
    ui->tableWidgetSetup->item(1, 0)->setText(
        g_settings.value("levelPath").toString());
}

void TombRaiderLinuxLauncher::LevelSaveClicked() {
    if (m_current.isValid()) {
        qint64 id = levelListProxy->getLid(m_current);

        // settings.setValue(QString("level%1/CustomCommand")
        //     .arg(id), ui->lineEditCustomCommand->text());

        g_settings.setValue(QString("level%1/EnvironmentVariables")
            .arg(id), ui->lineEditEnvironmentVariables->text());

        g_settings.setValue(QString("level%1/RunnerType")
            .arg(id), ui->comboBoxRunnerType->currentIndex());
    }
}

void TombRaiderLinuxLauncher::LevelResetClicked() {
}

TombRaiderLinuxLauncher::~TombRaiderLinuxLauncher() {
    delete ui;
    QApplication::quit();
}
