#include "view/Levels.hpp"
#include <qapplication.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <qlayoutitem.h>
#include <qlogging.h>
#include <qnamespace.h>
#include <qradiobutton.h>
#include <qwidget.h>
#include <qboxlayout.h>
#include <qpushbutton.h>
#include <qlineedit.h>
#include <qwebengineview.h>

UiLevels::UiLevels(QWidget *parent)
    : QWidget{parent},
    stackedWidget(new QStackedWidget(this)),
    dialog(new Dialog(stackedWidget)),
    info(new Info(stackedWidget)),
    loading(new Loading(stackedWidget)),
    select(new Select(stackedWidget)),
    walkthough(new Walkthrough(stackedWidget))
{
    setObjectName("Levels");
    layout = new QGridLayout(this);
    layout->setContentsMargins(6, 6, 6, 6);
    layout->setSpacing(8);

    layout->addWidget(stackedWidget,0,0);

    stackedWidget->addWidget(dialog);
    stackedWidget->setCurrentWidget(dialog);
    stackedWidget->addWidget(info);
    stackedWidget->addWidget(loading);
    stackedWidget->addWidget(select);
    stackedWidget->addWidget(walkthough);

    connect(dialog, &Dialog::cancelClicked, this, [this]() {
        this->setStackedWidget("select");
    });

    Filter* filter_p = select->filter;
    filter_p->setVisible(false);

    QPushButton *pushButtonFilter =
        select->stackedWidgetBar->navigateWidgetBar->pushButtonFilter;
    connect(pushButtonFilter, &QPushButton::clicked,
            this, [filter_p, pushButtonFilter]() -> void {
        bool isVisible = !filter_p->isVisible();
        filter_p->setVisible(isVisible);

        QIcon arrowDownIcon(":/icons/down-arrow.svg");
        QIcon arrowUpIcon(":/icons/up-arrow.svg");

        if (isVisible) {
            pushButtonFilter->setIcon(arrowUpIcon);
        } else {
            pushButtonFilter->setIcon(arrowDownIcon);
        }
        pushButtonFilter->setIconSize(QSize(16, 16));
    });

    QIcon arrowDownIcon(":/icons/down-arrow.svg");

    pushButtonFilter->setIcon(arrowDownIcon);
    pushButtonFilter->setIconSize(QSize(16, 16));


    connect(dialog, &Dialog::setLevelsState,
        this, &UiLevels::callbackDialog);

    m_loadingIndicatorWidget = new LoadingIndicator(loading);
    //m_loadingIndicatorWidget->setFixedSize(64, 64);
    //ui->verticalLayout_15->
    //        addWidget(m_loadingIndicatorWidget, 0, Qt::AlignCenter);
    m_loadingIndicatorWidget->show();
    stackedWidget->setCurrentWidget(
            stackedWidget->findChild<QWidget*>("loading"));
    m_loadingDoneGoTo = "select";

    // Arrive with next batch of level icons
    connect(&Controller::getInstance(), SIGNAL(controllerReloadLevelList()),
            this, SLOT(loadMoreCovers()));

    // Thread work done signal connections
    connect(&Controller::getInstance(),
            SIGNAL(controllerGenerateList(QList<int>)),
            this, SLOT(generateList(QList<int>)));


    // Progress bar signal connection
    connect(&Controller::getInstance(), SIGNAL(controllerTickSignal()),
            this, SLOT(workTick()));


    // Error signal connections
    connect(&Controller::getInstance(), SIGNAL(controllerDownloadError(int)),
            this, SLOT(downloadError(int)));

    // Loading done signal connections
    connect(&Controller::getInstance(), SIGNAL(controllerLoadingDone()),
            this, SLOT(updateLevelDone()));

    // Loading done signal connections
    connect(&Controller::getInstance(), SIGNAL(controllerRunningDone()),
            this, SLOT(runningLevelDone()));
}
void UiLevels::backClicked() {
    this->info->infoContent->infoWebEngineView->setHtml("");
    this->walkthough->walkthroughWebEngineView->setHtml("");
    this->select->stackedWidgetBar->backClicked();
}


void UiLevels::setStackedWidget(const QString &qwidget) {
    this->stackedWidget->setCurrentWidget(
        this->stackedWidget->findChild<QWidget*>(qwidget));
}

Loading::Loading(QWidget *parent)
    : QWidget(parent)
{
    setObjectName("loading");
    layout = new QVBoxLayout(this);
    layout->setContentsMargins(6, 6, 6, 6);
    layout->setSpacing(8);
}

void UiLevels::callbackDialog(QString selected) {
    const quint64 lid = select->getLid();

    if (selected == "Remove Level and zip files" ||
            selected == "Remove zip file") {
        controller.deleteZip(lid);
    }
    if (selected == "Remove Level and zip files" ||
            selected == "Remove just Level files") {
        if (controller.deleteLevel(lid)) {
            select->setRemovedLevel();
        }
    }
    this->setStackedWidget("select");
}

void UiLevels::generateList(const QList<int>& availableGames) {
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
        stackedWidget->setCurrentWidget(
                stackedWidget->findChild<QWidget*>("select"));
    }
}

qint64 UiLevels::getItemId() {
    return select->getLid();
}

void UiLevels::setItemChanged(const QModelIndex &current) {
    select->setItemChanged(current);
    qint64 id = select->getLid();
    qDebug() << "setItemChanged id:" << id;
    levelDirSelected(id);
}


void UiLevels::levelDirSelected(qint64 id) {
    if (id != 0) {
        int state = controller.getItemState(id);
        // Activate or deactivate pushbuttons based on the selected item
        qDebug() << "Selected: " << id << Qt::endl;
        // if state == 0 We dont have (id).TRLE directory
        // if state == 1 We dont have Original.TR(id) directory
        // if state == 2 we have Original.TR(id) if id was negative or (id).TRLE
        // if state == -1 then de-activate all buttons

        NavigateWidgetBar* nbar =
                this->select->stackedWidgetBar->navigateWidgetBar;
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

void UiLevels::setSortMode(LevelListProxy::SortMode mode) {
    select->setSortMode(mode);
}

void UiLevels::setList() {
    QVector<QSharedPointer<ListItemData>> list;
    controller.getList(&list);
    InstalledStatus installedStatus = getInstalled();

    for (const auto &item : list) {
        Q_ASSERT(item != nullptr);
        bool trle = installedStatus.trle.value(item->m_trle_id, false);
        item->m_installed = trle;
    }

    select->setLevels(list);
    loadMoreCovers();
}

void UiLevels::loadMoreCovers() {
    if (!select->stop()) {
        QVector<QSharedPointer<ListItemData>> buffer =
                select->getDataBuffer(20);
        if (!buffer.isEmpty()) {
            controller.getCoverList(buffer);
        }
    }
    static bool firstTime = true;
    if (!firstTime) {
        select->reset();
    } else {
        firstTime = false;
    }
}


void UiLevels::infoClicked() {
    qint64 id = select->getLid();
    if (id != 0) {
        InfoData info = controller.getInfo(id);
        if (info.m_body == "" && info.m_imageList.size() == 0) {
            m_loadingIndicatorWidget->show();
            stackedWidget->setCurrentWidget(
                    stackedWidget->findChild<QWidget*>("loading"));
            controller.updateLevel(id);
            m_loadingDoneGoTo = "info";
            return;
        }

        this->info->infoContent->infoWebEngineView->setHtml(info.m_body);

        QListWidget *coverListWidget = this->info->infoContent->coverListWidget;
        coverListWidget->clear();
        for (int i = 0; i < info.m_imageList.size(); ++i) {
            const QIcon &icon = info.m_imageList.at(i);
            QListWidgetItem *item = new QListWidgetItem(icon, "");
            item->setSizeHint(QSize(502, 377));
            coverListWidget->addItem(item);
        }
        this->info->infoContent->infoWebEngineView->show();
        stackedWidget->setCurrentWidget(
                stackedWidget->findChild<QWidget*>("info"));
        if (controller.getWalkthrough(id) != "") {
            this->info->infoBar->pushButtonWalkthrough->setEnabled(true);
        } else {
            this->info->infoBar->pushButtonWalkthrough->setEnabled(false);
        }
    }
}

void UiLevels::setStartupSetting(const StartupSetting startupSetting) {
    bool setProxyCoversFirst = false;

    FilterGroupBoxFilter* filterBox =
        this->select->filter->filterFirstInputRow->filterGroupBoxFilter;
    FilterGroupBoxToggle* toggleBox =
        this->select->filter->filterSecondInputRow->filterGroupBoxToggle;
    if (startupSetting.installed == true) {
        toggleBox->checkBoxInstalled->setChecked(true);
        setProxyCoversFirst = true;
    }
    if (startupSetting.original == true) {
        toggleBox->checkBoxOriginal->setChecked(true);
        setProxyCoversFirst = true;
    }
    if (startupSetting.type_id != 0) {
        filterBox->comboBoxType->setCurrentIndex(startupSetting.type_id);
        setProxyCoversFirst = true;
    }
    if (startupSetting.class_id != 0) {
        filterBox->comboBoxClass->setCurrentIndex(startupSetting.class_id);
        setProxyCoversFirst = true;
    }
    if (startupSetting.difficulty_id != 0) {
        filterBox->comboBoxDifficulty->setCurrentIndex(startupSetting.difficulty_id);
        setProxyCoversFirst = true;
    }
    if (startupSetting.duration_id != 0) {
        filterBox->comboBoxDuration->setCurrentIndex(startupSetting.duration_id);
        setProxyCoversFirst = true;
    }

    if (startupSetting.sort_id != 0) {
        setProxyCoversFirst = true;
    }

    if (setProxyCoversFirst == true) {
        this->select->levelViewList->setProxyCoversFirst();
    }

}

void UiLevels::walkthroughClicked() {
    qint64 id = select->getLid();
    if (id != 0) {
        QWebEngineView* w =
            this->walkthough->walkthroughWebEngineView;
        w->setHtml(controller.getWalkthrough(id));
        w->show();
        this->stackedWidget->setCurrentWidget(
                this->stackedWidget->findChild<QWidget*>("walkthrough"));
    }
}

void UiLevels::updateLevelDone() {
    m_loadingIndicatorWidget->hide();
    if (m_loadingDoneGoTo == "select") {
        setList();
        this->stackedWidget->setCurrentWidget(
                this->stackedWidget->findChild<QWidget*>("select"));
    } else if (m_loadingDoneGoTo == "info") {
        qint64 id = select->getLid();
        if (id != 0) {
            InfoData info = controller.getInfo(id);
            if (!(info.m_body == "" && info.m_imageList.size() == 0)) {
                infoClicked();
            } else {
                this->stackedWidget->setCurrentWidget(
                        this->stackedWidget->findChild<QWidget*>("select"));
            }
        }
    } else {
        qDebug() << "Forgot to set m_loadingDoneGoTo?";
    }
}

void UiLevels::removeClicked(qint64 id) {
    // TODO: add the ability to remove the level without its save files
    if (controller.checkZip(id)) {
        QString text(
            "Select what you want to remove.\n"
            "If you remove the level now you remove the level save files.\n"
        );
        dialog->setMessage(text);
        dialog->setOptions(QStringList()
                << "Remove Level and zip files"
                << "Remove just Level files"
                << "Remove zip file");
    } else {
        QString text(
            "Select OK select to continue to remove level.\n"
            "If you remove the level now you remove the level save files.\n"
        );
        dialog->setMessage(text);
        dialog->setOptions(QStringList()
                << "Remove just Level files");
    }

    stackedWidget->setCurrentWidget(
            stackedWidget->findChild<QWidget*>("dialog"));
}

void UiLevels::downloadClicked(qint64 id) {
    qDebug() << "void TombRaiderLinuxLauncher"
             << "::downloadClicked() qint64 id: "
             << id;

    // Set ui state for downloading
    select->levelViewList->setEnabled(false);
    select->filter->filterFirstInputRow->filterGroupBoxSearch->setEnabled(false);
    select->filter->filterFirstInputRow->filterGroupBoxFilter->setEnabled(false);
    select->filter->filterSecondInputRow->filterGroupBoxToggle->setEnabled(false);
    select->filter->filterSecondInputRow->filterGroupBoxSort->setEnabled(false);
    select->stackedWidgetBar->progressWidgetBar->progressBar->setValue(0);
    select->stackedWidgetBar->setCurrentWidget(
            select->stackedWidgetBar->findChild<QWidget*>("progress"));

}

void UiLevels::setpushButtonRunText(const QString &text) {
    select->stackedWidgetBar->navigateWidgetBar->
        pushButtonRun->setText(text);
}

void UiLevels::setupGameOrLevel(qint64 id) {
    if (select->getType()) {
        controller.setupGame(id);
    } else {
        controller.setupLevel(id);
    }
}

void UiLevels::workTick() {
    int value = this->select->stackedWidgetBar->progressWidgetBar->
        progressBar->value();
    this->select->stackedWidgetBar->progressWidgetBar->
        progressBar->setValue(value + 1);
    qDebug() << this->select->stackedWidgetBar->
        progressWidgetBar->progressBar->value() << "%";
    if (this->select->stackedWidgetBar->
            progressWidgetBar->progressBar->value() >= 100) {
        qint64 id = select->getLid();
        select->setInstalledLevel();
        if (select->getType()) {
            g_settings.setValue(
                    QString("installed/game%1").arg(id),
                        "true");
        } else {
            g_settings.setValue(
                    QString("installed/level%1").arg(id),
                        "true");
        }
        this->select->stackedWidgetBar->setCurrentWidget(
                this->select->stackedWidgetBar->findChild<QWidget*>("navigate"));
        this->select->levelViewList->setEnabled(true);
        this->select->filter->filterFirstInputRow->
            filterGroupBoxSearch->setEnabled(true);
        this->select->filter->filterFirstInputRow->
            filterGroupBoxFilter->setEnabled(true);
        this->select->filter->filterSecondInputRow->
            filterGroupBoxToggle->setEnabled(true);
        this->select->filter->filterSecondInputRow->
            filterGroupBoxSort->setEnabled(true);
        levelDirSelected(id);
    }
}
void UiLevels::runClicked() {

    qint64 id = select->getLid();
    if (id != 0) {
        this->select->levelViewList->setEnabled(false);
        this->select->filter->filterFirstInputRow->
            filterGroupBoxSearch->setEnabled(false);
        this->select->filter->filterFirstInputRow->
            filterGroupBoxFilter->setEnabled(false);
        this->select->filter->filterSecondInputRow->
            filterGroupBoxToggle->setEnabled(false);
        this->select->filter->filterSecondInputRow->
            filterGroupBoxSort->setEnabled(false);
        this->select->stackedWidgetBar->navigateWidgetBar->
            pushButtonRun->setEnabled(false);
        this->select->stackedWidgetBar->navigateWidgetBar->
            checkBoxSetup->setEnabled(false);

        if (select->getType()) {
            id = (-1)*id;
        }
        if (id != 0) {
            qint64 type = g_settings.value(
                    QString("level%1/RunnerType").arg(id)).toInt();
            QString input = g_settings.value(
                    QString("level%1/EnvironmentVariables").arg(id)).toString();
            qDebug() << "Type was: " << type;
            RunnerOptions options;

            if (type == 0) {
                options.id = id;
                options.command = UMU;
                options.setup =
                    this->select->stackedWidgetBar->
                        navigateWidgetBar->checkBoxSetup->isChecked();
                options.envList = parsToEnv(input);
                controller.run(options);
            } else if (type == 1) {
                options.id = id;
                options.command = WINE;
                options.setup =
                    this->select->stackedWidgetBar->
                        navigateWidgetBar->checkBoxSetup->isChecked();
                options.envList = parsToEnv(input);
                controller.run(options);
            } else if (type == 2) {
                options.id = id;
                options.command = LUTRIS;
                options.arguments = parsToArg(input);
                controller.run(options);
            } else if (type == 3) {
                if (!controller.link(id)) {
                    qDebug() << "link error";
                    runningLevelDone();
                } else {
                    options.id = id;
                    options.command = LUTRIS;
                    options.arguments = parsToArg(input);
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

void UiLevels::runningLevelDone() {
    this->select->levelViewList->setEnabled(true);

    this->select->filter->filterFirstInputRow->
        filterGroupBoxSearch->setEnabled(true);
    this->select->filter->filterFirstInputRow->
        filterGroupBoxFilter->setEnabled(true);
    this->select->filter->filterSecondInputRow->
        filterGroupBoxToggle->setEnabled(true);
    this->select->filter->filterSecondInputRow->
        filterGroupBoxSort->setEnabled(true);
    this->select->stackedWidgetBar->navigateWidgetBar->
        pushButtonRun->setEnabled(true);
    this->select->stackedWidgetBar->navigateWidgetBar->
        checkBoxSetup->setEnabled(true);
}

QStringList UiLevels::parsToArg(const QString& str) {
    static const QRegularExpression re = QRegularExpression("\\s+");
    QStringList list = str.split(re, Qt::SkipEmptyParts);
    return list;
}

QVector<QPair<QString, QString>>
    UiLevels::parsToEnv(const QString& str) {
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

UiLevels::InstalledStatus UiLevels::getInstalled() {
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
