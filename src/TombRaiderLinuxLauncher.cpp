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
#include <QLineEdit>
#include <QTableWidget>
#include <qapplication.h>
#include <qlogging.h>



TombRaiderLinuxLauncher::TombRaiderLinuxLauncher(QWidget *parent)
    : QMainWindow(parent) {
    ui = new Ui(this);
    setCentralWidget(ui);
    
    setMinimumSize(1280, 800);
    resize(1280, 800);

    // Button signal connections
    // List tab
    NavigateWidgetBar* nbar = ui->levels->select->stackedWidgetBar->navigateWidgetBar;
    InfoBar* ibar = ui->levels->info->infoBar;
    SetupInput* sfbar = ui->setup->firstTime->setupInput;
    connect(nbar->pushButtonRun, SIGNAL(clicked()), this, SLOT(runClicked()));
    connect(nbar->pushButtonDownload, SIGNAL(clicked()),
            this, SLOT(downloadOrRemoveClicked()));
    connect(nbar->pushButtonInfo, SIGNAL(clicked()), this, SLOT(infoClicked()));
    connect(ibar->pushButtonWalkthrough, SIGNAL(clicked()),
            this, SLOT(walkthroughClicked()));
    connect(ibar->pushButtonBack, SIGNAL(clicked()), this, SLOT(backClicked()));
    connect(ui->levels->walkthough->walkthroughBar->walkthroughBackButton, SIGNAL(clicked()),
            this, SLOT(backClicked()));
    connect(sfbar->setOptions, SIGNAL(clicked()), this, SLOT(setOptionsClicked()));

    GlobalControl* ssgbar = ui->setup->settings->frameGlobalSetup->globalControl;
    LevelControl* sslbar = ui->setup->settings->frameLevelSetup->levelControl;
    // Settings tab
    connect(ssgbar->commandLinkButtonGSSave, SIGNAL(clicked()),
            this, SLOT(GlobalSaveClicked()));
    connect(ssgbar->commandLinkButtonGSReset, SIGNAL(clicked()),
            this, SLOT(GlobalResetClicked()));
    connect(sslbar->commandLinkButtonLSSave, SIGNAL(clicked()),
            this, SLOT(LevelSaveClicked()));
    connect(sslbar->commandLinkButtonLSReset, SIGNAL(clicked()),
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
    nbar->pushButtonRun->setEnabled(false);
    nbar->pushButtonInfo->setEnabled(false);
    nbar->pushButtonDownload->setEnabled(false);
    Filter* filter_p = ui->levels->select->filter;
    QPushButton* filterButton_p = nbar->pushButtonFiler;

    filter_p->setVisible(false);

    connect(filterButton_p, &QPushButton::clicked,
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

    FilterGroupBoxSort* sortBox = filter_p->filterSecondInputRow->filterGroupBoxSort;
    FilterGroupBoxToggle* toggleBox = filter_p->filterSecondInputRow->filterGroupBoxToggle;
    connect(sortBox->radioButtonLevelName, &QRadioButton::clicked,
            this, &TombRaiderLinuxLauncher::sortByTitle);
    connect(sortBox->radioButtonDifficulty, &QRadioButton::clicked,
            this, &TombRaiderLinuxLauncher::sortByDifficulty);
    connect(sortBox->radioButtonDuration, &QRadioButton::clicked,
            this, &TombRaiderLinuxLauncher::sortByDuration);
    connect(sortBox->radioButtonClass, &QRadioButton::clicked,
            this, &TombRaiderLinuxLauncher::sortByClass);
    connect(sortBox->radioButtonType, &QRadioButton::clicked,
            this, &TombRaiderLinuxLauncher::sortByType);
    connect(sortBox->radioButtonReleaseDate, &QRadioButton::clicked,
            this, &TombRaiderLinuxLauncher::sortByReleaseDate);
    connect(toggleBox->checkBoxOriginal, &QRadioButton::clicked,
            this, &TombRaiderLinuxLauncher::showtOriginal);

    FilterGroupBoxFilter* filterBox =
        filter_p->filterFirstInputRow->filterGroupBoxFilter;
    connect(filterBox->comboBoxClass, &QComboBox::currentTextChanged,
            this, &TombRaiderLinuxLauncher::filterByClass);
    connect(filterBox->comboBoxType, &QComboBox::currentTextChanged,
            this, &TombRaiderLinuxLauncher::filterByType);
    connect(filterBox->comboBoxDifficulty, &QComboBox::currentTextChanged,
            this, &TombRaiderLinuxLauncher::filterByDifficulty);
    connect(filterBox->comboBoxDuration, &QComboBox::currentTextChanged,
            this, &TombRaiderLinuxLauncher::filterByDuration);

    levelListModel = new LevelListModel(this);
    levelListProxy = new LevelListProxy(this);
    levelListProxy->setSourceModel(levelListModel);

    LevelViewList* levelViewList = ui->levels->select->levelViewList;
    levelViewList->setModel(levelListProxy);

    CardItemDelegate* delegate = new CardItemDelegate(levelViewList);
    levelViewList->setItemDelegate(delegate);
    levelViewList->setSpacing(8);
    levelViewList->setMouseTracking(true);
    connect(
            levelViewList->selectionModel(),
            &QItemSelectionModel::currentChanged,
            this,
            &TombRaiderLinuxLauncher::onCurrentItemChanged);

    connect(toggleBox->checkBoxInstalled, &QCheckBox::toggled,
            levelListProxy, &LevelListProxy::setInstalledFilter);

    FilterGroupBoxSearch* searchBox =
        filter_p->filterFirstInputRow->filterGroupBoxSearch;
    connect(searchBox->comboBoxSearch, &QComboBox::currentTextChanged,
            levelListProxy, &LevelListProxy::setSearchType);

    connect(searchBox->lineEditSearch, &QLineEdit::textChanged,
            levelListProxy, &LevelListProxy::setSearchFilter);

    m_loadingIndicatorWidget = new LoadingIndicator(ui->levels->loading);
    //m_loadingIndicatorWidget->setFixedSize(64, 64);
    //ui->verticalLayout_15->
    //        addWidget(m_loadingIndicatorWidget, 0, Qt::AlignCenter);
    m_loadingIndicatorWidget->show();
    ui->levels->stackedWidget->setCurrentWidget(
            ui->levels->stackedWidget->findChild<QWidget*>("loading"));
    m_loadingDoneGoTo = "select";

    m_dialogWidget = new Dialog(ui->levels->dialog);
    //ui->verticalLayout_17->addWidget(m_dialogWidget);
    connect(m_dialogWidget, &Dialog::okClicked, this, [this, toggleBox, nbar]() {
        const QString selected = m_dialogWidget->selectedOption();
        qDebug() << "OK clicked, selected:" << selected;

        const quint64 lid = levelListProxy->getLid(m_current);
        const bool type = levelListProxy->getItemType(m_current);

        if (selected == "Remove Level and zip files" ||
                selected == "Remove zip file") {
            controller.deleteZip(lid);
        }

        if (selected == "Remove Level and zip files" ||
                selected == "Remove just Level files") {
            if (controller.deleteLevel(lid)) {
                levelListModel->clearInstalled(m_current);
                if (toggleBox->checkBoxInstalled->isChecked()) {
                    levelListProxy->setInstalledFilter(true);
                } else {
                    nbar->pushButtonDownload->setText("Download and install");
                }
            }
        }
        ui->levels->stackedWidget->setCurrentWidget(
                ui->levels->stackedWidget->findChild<QWidget*>("select"));
    });
    connect(m_dialogWidget, &Dialog::cancelClicked, this, [this]() {
        ui->levels->stackedWidget->setCurrentWidget(
                ui->levels->stackedWidget->findChild<QWidget*>("select"));
    });


    ui->tabs->setTabEnabled(ui->tabs->indexOf(
            ui->tabs->findChild<QWidget*>("Modding")), false);

    ui->tabs->setTabEnabled(ui->tabs->indexOf(
            ui->tabs->findChild<QWidget*>("Controller")), false);

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

    FilterGroupBoxSort* sortBox =
        ui->levels->select->filter->filterSecondInputRow->filterGroupBoxSort;
    if (m_ss.sort_id != 0) {
        if (m_ss.sort_id == 1) {
            sortBox->radioButtonLevelName->setChecked(true);
            levelListProxy->setSortMode(LevelListProxy::Title);
        } else if (m_ss.sort_id == 2) {
            sortBox->radioButtonDifficulty->setChecked(true);
            levelListProxy->setSortMode(LevelListProxy::Difficulty);
        } else if (m_ss.sort_id == 3) {
            sortBox->radioButtonDuration->setChecked(true);
            levelListProxy->setSortMode(LevelListProxy::Duration);
        } else if (m_ss.sort_id == 4) {
            sortBox->radioButtonClass->setChecked(true);
            levelListProxy->setSortMode(LevelListProxy::Class);
        } else if (m_ss.sort_id == 5) {
            sortBox->radioButtonType->setChecked(true);
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
        ui->levels->stackedWidget->setCurrentWidget(
                ui->levels->stackedWidget->findChild<QWidget*>("select"));
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

    FilterGroupBoxFilter* filterBox =
        ui->levels->select->filter->filterFirstInputRow->filterGroupBoxFilter;
    FilterGroupBoxToggle* toggleBox =
        ui->levels->select->filter->filterSecondInputRow->filterGroupBoxToggle;
    if (ss.installed == true) {
        toggleBox->checkBoxInstalled->setChecked(true);
        setProxyCoversFirst = true;
    }
    if (ss.original == true) {
        toggleBox->checkBoxOriginal->setChecked(true);
        setProxyCoversFirst = true;
    }
    if (ss.type_id != 0) {
        filterBox->comboBoxType->setCurrentIndex(ss.type_id);
        setProxyCoversFirst = true;
    }
    if (ss.class_id != 0) {
        filterBox->comboBoxClass->setCurrentIndex(ss.class_id);
        setProxyCoversFirst = true;
    }
    if (ss.difficulty_id != 0) {
        filterBox->comboBoxDifficulty->setCurrentIndex(ss.difficulty_id);
        setProxyCoversFirst = true;
    }
    if (ss.duration_id != 0) {
        filterBox->comboBoxDuration->setCurrentIndex(ss.duration_id);
        setProxyCoversFirst = true;
    }
    m_ss = ss;

    if (m_ss.sort_id != 0) {
        setProxyCoversFirst = true;
    }

    if (setProxyCoversFirst == true) {
        ui->levels->select->levelViewList->setProxyCoversFirst();
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
    const QString gamePath = g_settings.value("gamePath").toString();
    QTableWidget* table = ui->setup->settings->frameGlobalSetup->tableWidgetGlobalSetup;
    table->item(0, 0)->setText(gamePath);
    qDebug() << "Read game path value:" << gamePath;

    const QString extraGamePath = g_settings.value("extraGamePath").toString();
    table->item(1, 0)->setText(extraGamePath);
    qDebug() << "Read extra game path value:" << extraGamePath;

    const QString levelPath = g_settings.value("levelPath").toString();
    table->item(2, 0)->setText(levelPath);
    qDebug() << "Read level path value:" << levelPath;

    WidgetDeleteZip* widgetDeleteZip = ui->setup->settings->frameGlobalSetup->widgetDeleteZip;
    const bool deleteZip = g_settings.value("DeleteZip").toBool();
    widgetDeleteZip->checkBoxDeleteZip->setChecked(deleteZip);
    qDebug() << "Read level DeleteZip (after download) value:" << deleteZip;

    WidgetDefaultEnvironmentVariables* widgetDefaultEnvironmentVariables =
        ui->setup->settings->frameGlobalSetup->widgetDefaultEnvironmentVariables;
    const QString defaultEnvironmentVariables =
            g_settings.value("defaultEnvironmentVariables").toString();
    widgetDefaultEnvironmentVariables->lineEditDefaultEnvironmentVariables->
            setText(defaultEnvironmentVariables);
    qDebug() << "Read defaultEnvironmentVariables value:"
            << defaultEnvironmentVariables;

    WidgetDefaultRunnerType* widgetDefaultRunnerType =
        ui->setup->settings->frameGlobalSetup->widgetDefaultRunnerType;
    const quint64 defaultRunnerType =
            g_settings.value("defaultRunnerType").toInt();
    widgetDefaultRunnerType->comboBoxDefaultRunnerType->setCurrentIndex(defaultRunnerType);
    qDebug() << "Read defaultRunnerType value:" << defaultRunnerType;

    controller.setup();
}

void TombRaiderLinuxLauncher::setup() {
    ui->tabs->setCurrentIndex(
        ui->tabs->indexOf(ui->tabs->findChild<QWidget*>("Setup")));
    ui->setup->stackedWidget->setCurrentWidget(
        ui->setup->stackedWidget->findChild<QWidget*>("firstTime"));
    ui->tabs->setTabEnabled(
        ui->tabs->indexOf(ui->tabs->findChild<QWidget*>("Levels")), false);
    ui->tabs->setTabEnabled(
        ui->tabs->indexOf(ui->tabs->findChild<QWidget*>("Modding")), false);

    qDebug() << "Entering setup" << Qt::endl;
    const QString homeDir =
            QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
    qDebug() << "Home Directory:" << homeDir;


    SetupInput* setupInput = ui->setup->firstTime->setupInput;
    setupInput->gamePathContainer->gamePathEdit->setText(QString("%1%2")
            .arg(homeDir, "/.steam/steam/steamapps/common"));
    setupInput->extraGamePathContainer->extraGamePathEdit->setText(QString("%1%2")
            .arg(homeDir, "/.wine/drive_c/Program Files (x86)"));
    setupInput->levelPathContainer->levelPathEdit->setText(QString("%1%2")
            .arg(homeDir, "/.local/share/TombRaiderLinuxLauncher"));
}

void TombRaiderLinuxLauncher::levelDirSelected(qint64 id) {
    if (id != 0) {
        int state = controller.getItemState(id);
        // Activate or deactivate pushbuttons based on the selected item
        qDebug() << "Selected: " << id << Qt::endl;
        // if state == 0 We dont have (id).TRLE directory
        // if state == 1 We dont have Original.TR(id) directory
        // if state == 2 we have Original.TR(id) if id was negative or (id).TRLE
        // if state == -1 then de-activate all buttons

        NavigateWidgetBar* nbar =
                ui->levels->select->stackedWidgetBar->navigateWidgetBar;
        if (state == 1) {
            nbar->pushButtonRun->setEnabled(true);
            nbar->pushButtonInfo->setEnabled(false);
            nbar->pushButtonDownload->setEnabled(true);
            nbar->pushButtonDownload->setText("Remove");
        } else if (state == 2) {
            nbar->pushButtonRun->setEnabled(true);
            nbar->pushButtonInfo->setEnabled(true);
            nbar->pushButtonDownload->setEnabled(true);
            nbar->pushButtonDownload->setText("Remove");
        } else if (state == 0) {
            nbar->pushButtonRun->setEnabled(false);
            nbar->pushButtonInfo->setEnabled(true);
            nbar->pushButtonDownload->setEnabled(true);
            nbar->pushButtonDownload->setText("Download and install");
        } else {
            nbar->pushButtonRun->setEnabled(false);
            nbar->pushButtonInfo->setEnabled(false);
            nbar->pushButtonDownload->setEnabled(false);
        }
    }
}

void TombRaiderLinuxLauncher::onCurrentItemChanged(
        const QModelIndex &current, const QModelIndex &previous) {
    m_current = levelListProxy->mapToSource(current);
    if (m_current.isValid()) {
        FrameLevelSetupSettings* levelSettings =
            ui->setup->settings->frameLevelSetup->frameLevelSetupSettings;
        qint64 id = levelListProxy->getLid(m_current);
        levelDirSelected(id);
        levelSettings->widgetLevelID->lcdNumberLevelID->display(QString::number(id));
        levelSettings->widgetEnvironmentVariables->lineEditEnvironmentVariables->setEnabled(true);
        levelSettings->widgetEnvironmentVariables->lineEditEnvironmentVariables->setText(
                g_settings.value(QString("level%1/EnvironmentVariables")
                    .arg(id)).toString());
        levelSettings->widgetRunnerType->comboBoxRunnerType->setEnabled(true);
        levelSettings->widgetRunnerType->comboBoxRunnerType->setCurrentIndex(
                g_settings.value(QString("level%1/RunnerType")
                    .arg(id)).toInt());
        ui->levels->select->stackedWidgetBar->navigateWidgetBar->
                pushButtonRun->setText(
                    levelSettings->widgetRunnerType->comboBoxRunnerType->currentText());
        LevelControl* levelControl = ui->setup->settings->frameLevelSetup->levelControl;
        levelControl->commandLinkButtonLSSave->setEnabled(true);
        levelControl->commandLinkButtonLSReset->setEnabled(true);
    }
}

void TombRaiderLinuxLauncher::setOptionsClicked() {
    SetupInput* setupInput = ui->setup->firstTime->setupInput;
    QString gamePath =  setupInput->gamePathContainer->gamePathEdit->text();
    QString extraGamePath = setupInput->extraGamePathContainer->extraGamePathEdit->text();
    QString levelPath = setupInput->levelPathContainer->levelPathEdit->text();
    g_settings.setValue("gamePath", gamePath);
    g_settings.setValue("extraGamePath", extraGamePath);
    g_settings.setValue("levelPath", levelPath);
    g_settings.setValue("setup", "yes");

    ui->setup->settings->frameGlobalSetup->
            tableWidgetGlobalSetup->item(0, 0)->setText(gamePath);
    ui->setup->settings->frameGlobalSetup->
            tableWidgetGlobalSetup->item(1, 0)->setText(extraGamePath);
    ui->setup->settings->frameGlobalSetup->
            tableWidgetGlobalSetup->item(2, 0)->setText(levelPath);
    ui->tabs->setTabEnabled(ui->tabs->indexOf(
            ui->tabs->findChild<QWidget*>("Levels")), true);
    ui->tabs->setTabEnabled(ui->tabs->indexOf(
            ui->tabs->findChild<QWidget*>("Modding")), false);
    ui->tabs->show();
    ui->tabs->setCurrentIndex(ui->tabs->indexOf(
            ui->tabs->findChild<QWidget*>("Levels")));
    ui->setup->stackedWidget->setCurrentWidget(
            ui->setup->stackedWidget->findChild<QWidget*>("settings"));

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
        ui->levels->select->levelViewList->setEnabled(false);
        ui->levels->select->filter->filterFirstInputRow->
            filterGroupBoxSearch->setEnabled(false);
        ui->levels->select->filter->filterFirstInputRow->
            filterGroupBoxFilter->setEnabled(false);
        ui->levels->select->filter->filterSecondInputRow->
            filterGroupBoxToggle->setEnabled(false);
        ui->levels->select->filter->filterSecondInputRow->
            filterGroupBoxSort->setEnabled(false);
        ui->levels->select->stackedWidgetBar->navigateWidgetBar->
            pushButtonRun->setEnabled(false);
        ui->levels->select->stackedWidgetBar->navigateWidgetBar->
            checkBoxSetup->setEnabled(false);

        qint64 id = levelListProxy->getLid(m_current);
        if (levelListProxy->getItemType(m_current)) {
            id = (-1)*id;
        }
        if (id != 0) {
            qint64 type = g_settings.value(
                    QString("level%1/RunnerType").arg(id)).toInt();
            QLineEdit* input =
                ui->setup->settings->frameLevelSetup->frameLevelSetupSettings->
                    widgetEnvironmentVariables->lineEditEnvironmentVariables;
            qDebug() << "Type was: " << type;
            RunnerOptions options;

            if (type == 0) {
                options.id = id;
                options.command = UMU;
                options.setup =
                    ui->levels->select->stackedWidgetBar->
                        navigateWidgetBar->checkBoxSetup->isChecked();
                options.envList = parsToEnv(input->text());
                controller.run(options);
            } else if (type == 1) {
                options.id = id;
                options.command = WINE;
                options.setup =
                    ui->levels->select->stackedWidgetBar->
                        navigateWidgetBar->checkBoxSetup->isChecked();
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
    ui->levels->select->levelViewList->setEnabled(true);

    ui->levels->select->filter->filterFirstInputRow->
        filterGroupBoxSearch->setEnabled(true);
    ui->levels->select->filter->filterFirstInputRow->
        filterGroupBoxFilter->setEnabled(true);
    ui->levels->select->filter->filterSecondInputRow->
        filterGroupBoxToggle->setEnabled(true);
    ui->levels->select->filter->filterSecondInputRow->
        filterGroupBoxSort->setEnabled(true);
    ui->levels->select->stackedWidgetBar->navigateWidgetBar->
        pushButtonRun->setEnabled(true);
    ui->levels->select->stackedWidgetBar->navigateWidgetBar->
        checkBoxSetup->setEnabled(true);
}

void TombRaiderLinuxLauncher::downloadOrRemoveClicked() {
    if (m_current.isValid()) {
        qint64 id = levelListProxy->getLid(m_current);
        int state = controller.getItemState(id);
        if (state  > 0) {
            // TODO: add the ability to remove the level without its save files
            if (controller.checkZip(id)) {
                QString text(
                    "Select what you want to remove.\n"
                    "If you remove the level now you remove the level save files.\n"
                );
                m_dialogWidget->setMessage(text);
                m_dialogWidget->setOptions(QStringList()
                        << "Remove Level and zip files"
                        << "Remove just Level files"
                        << "Remove zip file");
            } else {
                QString text(
                    "Select OK select to continue to remove level.\n"
                    "If you remove the level now you remove the level save files.\n"
                );
                m_dialogWidget->setMessage(text);
                m_dialogWidget->setOptions(QStringList()
                        << "Remove just Level files");
            }

            ui->levels->stackedWidget->setCurrentWidget(
                    ui->levels->stackedWidget->findChild<QWidget*>("dialog"));
        } else {
            qDebug() << "void TombRaiderLinuxLauncher"
                     << "::downloadClicked() qint64 id: "
                     << id;

            // Set ui state for downloading
            ui->levels->select->levelViewList->setEnabled(false);
            ui->levels->select->filter->filterFirstInputRow->filterGroupBoxSearch->setEnabled(false);
            ui->levels->select->filter->filterFirstInputRow->filterGroupBoxFilter->setEnabled(false);
            ui->levels->select->filter->filterSecondInputRow->filterGroupBoxToggle->setEnabled(false);
            ui->levels->select->filter->filterSecondInputRow->filterGroupBoxSort->setEnabled(false);
            ui->levels->select->stackedWidgetBar->progressWidgetBar->progressBar->setValue(0);
            ui->levels->select->stackedWidgetBar->setCurrentWidget(
                    ui->levels->select->stackedWidgetBar->findChild<QWidget*>("progress"));

            ui->setup->settings->frameLevelSetup->frameLevelSetupSettings->
                widgetEnvironmentVariables->lineEditEnvironmentVariables->setText(
                    g_settings.value("defaultEnvironmentVariables").toString());
            g_settings.setValue(QString("level%1/EnvironmentVariables")
                    .arg(id), ui->setup->settings->frameLevelSetup->frameLevelSetupSettings->
                widgetEnvironmentVariables->lineEditEnvironmentVariables->text());

            ui->setup->settings->frameLevelSetup->frameLevelSetupSettings->
                widgetRunnerType->comboBoxRunnerType->setCurrentIndex(
                    g_settings.value("defaultRunnerType").toInt());
            g_settings.setValue(QString("level%1/RunnerType")
                    .arg(id), ui->setup->settings->frameLevelSetup->frameLevelSetupSettings->
                widgetRunnerType->comboBoxRunnerType->currentIndex());

            ui->levels->select->stackedWidgetBar->navigateWidgetBar->
                pushButtonRun->setText(ui->setup->settings->frameLevelSetup->
                    frameLevelSetupSettings->widgetRunnerType->comboBoxRunnerType->currentText());

            if (levelListProxy->getItemType(m_current)) {
                controller.setupGame(id);
            } else {
                controller.setupLevel(id);
            }
        }
    }
}

void TombRaiderLinuxLauncher::infoClicked() {
    if (m_current.isValid()) {
        qint64 id = levelListProxy->getLid(m_current);
        if (id != 0) {
            InfoData info = controller.getInfo(id);
            if (info.m_body == "" && info.m_imageList.size() == 0) {
                m_loadingIndicatorWidget->show();
                ui->levels->stackedWidget->setCurrentWidget(
                        ui->levels->stackedWidget->findChild<QWidget*>("loading"));
                controller.updateLevel(id);
                m_loadingDoneGoTo = "info";
                return;
            }

            ui->levels->info->infoContent->infoWebEngineView->setHtml(info.m_body);

            // Get the vertical scrollbar size
            // to center the images for all themes
            // int scrollbarWidth = ui->infoListWidget->
            //         style()->pixelMetric(QStyle::PM_ScrollBarExtent);
            //QMargins margins = ui->infoListWidget->contentsMargins();
            //int left = margins.left();
            //int right = margins.right();

            QListWidget *w = ui->levels->info->infoContent->coverListWidget;
            //w->setMinimumWidth(left+502+scrollbarWidth+right);
            //w->setMaximumWidth(left+502+scrollbarWidth+right);

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
                w->addItem(item);
            }
            ui->levels->info->infoContent->infoWebEngineView->show();
            ui->levels->stackedWidget->setCurrentWidget(
                    ui->levels->stackedWidget->findChild<QWidget*>("info"));
            if (controller.getWalkthrough(id) != "") {
                ui->levels->info->infoBar->pushButtonWalkthrough->setEnabled(true);
            } else {
                ui->levels->info->infoBar->pushButtonWalkthrough->setEnabled(false);
            }
        }
    }
}

void TombRaiderLinuxLauncher::walkthroughClicked() {
    if (m_current.isValid()) {
        qint64 id = levelListProxy->getLid(m_current);
        if (id != 0) {
            QWebEngineView* w =
                ui->levels->walkthough->walkthroughWebEngineView;
            w->setHtml(controller.getWalkthrough(id));
            w->show();
            ui->levels->stackedWidget->setCurrentWidget(
                    ui->levels->stackedWidget->findChild<QWidget*>("walkthrough"));
        }
    }
}

void TombRaiderLinuxLauncher::backClicked() {
    ui->levels->info->infoContent->infoWebEngineView->setHtml("");
    ui->levels->walkthough->walkthroughWebEngineView->setHtml("");
    if (ui->levels->stackedWidget->currentWidget() ==
        ui->levels->stackedWidget->findChild<QWidget*>("info")) {
        ui->levels->stackedWidget->setCurrentWidget(
                ui->levels->stackedWidget->findChild<QWidget*>("select"));
    } else if (ui->levels->stackedWidget->currentWidget() ==
        ui->levels->stackedWidget->findChild<QWidget*>("walkthrough")) {
        ui->levels->stackedWidget->setCurrentWidget(
                ui->levels->stackedWidget->findChild<QWidget*>("select"));
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
    int value = ui->levels->select->stackedWidgetBar->progressWidgetBar->
        progressBar->value();
    ui->levels->select->stackedWidgetBar->progressWidgetBar->
        progressBar->setValue(value + 1);
    qDebug() << ui->levels->select->stackedWidgetBar->
        progressWidgetBar->progressBar->value() << "%";
    if (ui->levels->select->stackedWidgetBar->
            progressWidgetBar->progressBar->value() >= 100) {
        if (m_current.isValid()) {
            qint64 id = levelListProxy->getLid(m_current);
            levelListModel->setInstalled(m_current);
            if (levelListProxy->getItemType(m_current)) {
                g_settings.setValue(
                        QString("installed/game%1").arg(id),
                            "true");
            } else {
                g_settings.setValue(
                        QString("installed/level%1").arg(id),
                            "true");
            }
            ui->levels->select->stackedWidgetBar->setCurrentWidget(
                    ui->levels->select->stackedWidgetBar->findChild<QWidget*>("navigate"));
            ui->levels->select->levelViewList->setEnabled(true);
            ui->levels->select->filter->filterFirstInputRow->
                filterGroupBoxSearch->setEnabled(true);
            ui->levels->select->filter->filterFirstInputRow->
                filterGroupBoxFilter->setEnabled(true);
            ui->levels->select->filter->filterSecondInputRow->
                filterGroupBoxToggle->setEnabled(true);
            ui->levels->select->filter->filterSecondInputRow->
                filterGroupBoxSort->setEnabled(true);
            levelDirSelected(id);
        }
    }
}

void TombRaiderLinuxLauncher::downloadError(int status) {
    ui->levels->select->stackedWidgetBar->progressWidgetBar->progressBar->setValue(0);
    ui->levels->select->stackedWidgetBar->navigateWidgetBar->pushButtonRun->setEnabled(true);
    ui->levels->select->stackedWidgetBar->navigateWidgetBar->pushButtonInfo->setEnabled(true);
    ui->levels->select->stackedWidgetBar->navigateWidgetBar->pushButtonDownload->setEnabled(true);
    ui->levels->select->stackedWidgetBar->setCurrentWidget(
            ui->levels->select->stackedWidgetBar->findChild<QWidget*>("navigate"));
    ui->levels->select->levelViewList->setEnabled(true);
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
        ui->levels->stackedWidget->setCurrentWidget(
                ui->levels->stackedWidget->findChild<QWidget*>("select"));
    } else if (m_loadingDoneGoTo == "info") {
        if (m_current.isValid()) {
            qint64 id = levelListProxy->getLid(m_current);
            if (id != 0) {
                InfoData info = controller.getInfo(id);
                if (!(info.m_body == "" && info.m_imageList.size() == 0)) {
                    infoClicked();
                } else {
                    ui->levels->stackedWidget->setCurrentWidget(
                            ui->levels->stackedWidget->findChild<QWidget*>("select"));
                }
            }
        }
    } else {
        qDebug() << "Forgot to set m_loadingDoneGoTo?";
    }
}

void TombRaiderLinuxLauncher::GlobalSaveClicked() {
    const QString newGamePath =
        ui->setup->settings->frameGlobalSetup->
                tableWidgetGlobalSetup->item(0, 0)->text();
    const QString newExtraGamePath =
        ui->setup->settings->frameGlobalSetup->
                tableWidgetGlobalSetup->item(1, 0)->text();
    const QString newLevelPath =
        ui->setup->settings->frameGlobalSetup->
                tableWidgetGlobalSetup->item(2, 0)->text();

    const QString oldGamePath = g_settings.value("gamePath").toString();
    const QString oldExtraGamePath = g_settings.value("extraGamePath").toString();
    const QString oldLevelPath = g_settings.value("levelPath").toString();

    if ((newGamePath != oldGamePath) ||
            (newExtraGamePath != oldExtraGamePath) ||
            (newLevelPath != oldLevelPath)) {
        g_settings.setValue("gamePath" , newGamePath);
        g_settings.setValue("extraGamePath" , newExtraGamePath);
        g_settings.setValue("levelPath" , newLevelPath);
        controller.setup();
    }

    const bool newDeleteZip =
        ui->setup->settings->frameGlobalSetup->
            widgetDeleteZip->checkBoxDeleteZip->isChecked();
    g_settings.setValue("DeleteZip" , newDeleteZip);

    g_settings.setValue("defaultEnvironmentVariables",
                        ui->setup->settings->frameGlobalSetup->widgetDefaultEnvironmentVariables->
                        lineEditDefaultEnvironmentVariables->text());

    g_settings.setValue("defaultRunnerType",
                        ui->setup->settings->frameGlobalSetup->widgetDefaultRunnerType->
                        comboBoxDefaultRunnerType->currentIndex());
}

void TombRaiderLinuxLauncher::GlobalResetClicked() {
    ui->setup->settings->frameGlobalSetup->
        tableWidgetGlobalSetup->item(0, 0)->setText(
            g_settings.value("gamePath").toString());
    ui->setup->settings->frameGlobalSetup->
        tableWidgetGlobalSetup->item(1, 0)->setText(
            g_settings.value("extraGamePath").toString());
    ui->setup->settings->frameGlobalSetup->
        tableWidgetGlobalSetup->item(2, 0)->setText(
            g_settings.value("levelPath").toString());

    ui->setup->settings->frameGlobalSetup->
        widgetDeleteZip->checkBoxDeleteZip->setChecked(
            g_settings.value("DeleteZip").toBool());

    ui->setup->settings->frameGlobalSetup->widgetDefaultEnvironmentVariables->
        lineEditDefaultEnvironmentVariables->setText(
            g_settings.value("defaultEnvironmentVariables").toString());

    ui->setup->settings->frameGlobalSetup->widgetDefaultRunnerType->
        comboBoxDefaultRunnerType->setCurrentIndex(
            g_settings.value("defaultRunnerType").toInt());
}

void TombRaiderLinuxLauncher::LevelSaveClicked() {
    if (m_current.isValid()) {
        qint64 id = levelListProxy->getLid(m_current);

        g_settings.setValue(QString("level%1/EnvironmentVariables")
                .arg(id), ui->setup->settings->
                    frameLevelSetup->frameLevelSetupSettings->widgetEnvironmentVariables->
                            lineEditEnvironmentVariables->text());

        g_settings.setValue(QString("level%1/RunnerType")
                                .arg(id), ui->setup->settings->frameLevelSetup->
                            frameLevelSetupSettings->widgetRunnerType->
                            comboBoxRunnerType->currentIndex());
    }
    ui->levels->select->stackedWidgetBar->navigateWidgetBar->pushButtonRun->setText(
        ui->setup->settings->frameLevelSetup->frameLevelSetupSettings->
        widgetRunnerType->comboBoxRunnerType->currentText());
}

void TombRaiderLinuxLauncher::LevelResetClicked() {
    if (m_current.isValid()) {
        qint64 id = levelListProxy->getLid(m_current);

        ui->setup->settings->frameLevelSetup->frameLevelSetupSettings->widgetEnvironmentVariables->
            lineEditEnvironmentVariables->setText(
                g_settings.value(
                    QString("level%1/EnvironmentVariables")
                        .arg(id)).toString());

        ui->setup->settings->frameLevelSetup->frameLevelSetupSettings->widgetRunnerType->
            comboBoxRunnerType->setCurrentIndex(
                g_settings.value(
                    QString("level%1/RunnerType")
                        .arg(id)).toInt());
    }
}

TombRaiderLinuxLauncher::~TombRaiderLinuxLauncher() {
    delete ui;
}
