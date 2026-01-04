#include "view/Ui.hpp"

Ui::Ui(QWidget *parent)
    : QWidget(parent)
{
    this->setObjectName("centralwidget");

    layout = new QGridLayout(this);
    layout->setContentsMargins(9, 9, 9, 9);
    layout->setSpacing(8);

    tabs = new QTabWidget(this);

    levels = new UiLevels(tabs);
    tabs->addTab(levels, tr("Levels"));

    modding = new UiModding(tabs);
    tabs->addTab(modding, tr("Modding"));
    tabs->setTabEnabled(tabs->indexOf(
            tabs->findChild<QWidget*>("Modding")), false);

    uicontroller = new UiController(tabs);
    tabs->addTab(uicontroller, tr("Controller"));
    tabs->setTabEnabled(tabs->indexOf(
            tabs->findChild<QWidget*>("Controller")), false);

    setup = new UiSetup(tabs);
    tabs->addTab(setup, tr("Setup"));

    about = new UiAbout(tabs);
    tabs->addTab(about, tr("About"));

    layout->addWidget(tabs);


    // Button signal connections
    // List tab
    NavigateWidgetBar* nbar = this->levels->select->stackedWidgetBar->navigateWidgetBar;
    InfoBar* ibar = this->levels->info->infoBar;
    SetupInput* sfbar = this->setup->firstTime->setupInput;
    connect(nbar->pushButtonRun, SIGNAL(clicked()), this, SLOT(runClicked()));
    connect(nbar->pushButtonDownload, SIGNAL(clicked()),
            this, SLOT(downloadOrRemoveClicked()));
    connect(nbar->pushButtonInfo, SIGNAL(clicked()), this, SLOT(infoClicked()));
    connect(ibar->pushButtonWalkthrough, SIGNAL(clicked()),
            this, SLOT(walkthroughClicked()));
    connect(ibar->pushButtonBack, SIGNAL(clicked()), this, SLOT(backClicked()));
    connect(this->levels->walkthough->walkthroughBar->walkthroughBackButton, SIGNAL(clicked()),
            this, SLOT(backClicked()));
    connect(sfbar->setOptions, SIGNAL(clicked()), this, SLOT(setOptionsClicked()));

    GlobalControl* ssgbar = this->setup->settings->frameGlobalSetup->globalControl;
    LevelControl* sslbar = this->setup->settings->frameLevelSetup->levelControl;
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

    // Read settings
    QString value = g_settings.value("setup").toString();
    if (value != "yes") {
        this->startUpSetup();
    } else {
        setup->readSavedSettings();
    }
}

void Ui::onCurrentItemChanged(const QModelIndex &current) {
    if (current.isValid()) {
        levels->setItemChanged(current);
        qint64 id = levels->getItemId();
    }
}

void Ui::downloadOrRemoveClicked() {
    qint64 id = levels->getItemId();
    int state = controller.getItemState(id);
    if (state  > 0) {
        levels->removeClicked(id);
    } else {
        levels->downloadClicked(id);
        setup->downloadClicked(id);
        levels->setpushButtonRunText(setup->getRunnerTypeState());
        levels->setupGameOrLevel(id);
    }
}

void Ui::startUpSetup() {
    this->tabs->setCurrentIndex(
        this->tabs->indexOf(this->tabs->findChild<QWidget*>("Setup")));
    this->setup->stackedWidget->setCurrentWidget(
        this->setup->stackedWidget->findChild<QWidget*>("firstTime"));
    this->tabs->setTabEnabled(
        this->tabs->indexOf(this->tabs->findChild<QWidget*>("Levels")), false);
    this->tabs->setTabEnabled(
        this->tabs->indexOf(this->tabs->findChild<QWidget*>("Modding")), false);

    qDebug() << "Entering setup" << Qt::endl;
    const QString homeDir =
            QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
    qDebug() << "Home Directory:" << homeDir;


    SetupInput* setupInput = this->setup->firstTime->setupInput;
    setupInput->gamePathContainer->gamePathEdit->setText(QString("%1%2")
            .arg(homeDir, "/.steam/steam/steamapps/common"));
    setupInput->extraGamePathContainer->extraGamePathEdit->setText(QString("%1%2")
            .arg(homeDir, "/.wine/drive_c/Program Files (x86)"));
    setupInput->levelPathContainer->levelPathEdit->setText(QString("%1%2")
            .arg(homeDir, "/.local/share/TombRaiderLinuxLauncher"));
}

void Ui::LevelSaveClicked() {
    qint64 id = levels->getItemId();
    if (id != 0) {
        g_settings.setValue(QString("level%1/EnvironmentVariables")
                .arg(id), this->setup->settings->
                    frameLevelSetup->frameLevelSetupSettings->widgetEnvironmentVariables->
                            lineEditEnvironmentVariables->text());

        g_settings.setValue(QString("level%1/RunnerType")
                                .arg(id), this->setup->settings->frameLevelSetup->
                            frameLevelSetupSettings->widgetRunnerType->
                            comboBoxRunnerType->currentIndex());
    }
    this->levels->select->stackedWidgetBar->navigateWidgetBar->pushButtonRun->setText(
        this->setup->settings->frameLevelSetup->frameLevelSetupSettings->
        widgetRunnerType->comboBoxRunnerType->currentText());
}

void Ui::LevelResetClicked() {
    qint64 id = levels->getItemId();
    if (id != 0) {
        this->setup->settings->frameLevelSetup->frameLevelSetupSettings->widgetEnvironmentVariables->
            lineEditEnvironmentVariables->setText(
                g_settings.value(
                    QString("level%1/EnvironmentVariables")
                        .arg(id)).toString());

        this->setup->settings->frameLevelSetup->frameLevelSetupSettings->widgetRunnerType->
            comboBoxRunnerType->setCurrentIndex(
                g_settings.value(
                    QString("level%1/RunnerType")
                        .arg(id)).toInt());
    }
}

void Ui::setOptionsClicked() {
    this->tabs->setTabEnabled(this->tabs->indexOf(
            this->tabs->findChild<QWidget*>("Levels")), true);
    this->tabs->setTabEnabled(this->tabs->indexOf(
            this->tabs->findChild<QWidget*>("Modding")), false);
    this->tabs->show();
    this->tabs->setCurrentIndex(this->tabs->indexOf(
            this->tabs->findChild<QWidget*>("Levels")));
    this->setup->stackedWidget->setCurrentWidget(
            this->setup->stackedWidget->findChild<QWidget*>("settings"));

    setup->readSavedSettings();
}
