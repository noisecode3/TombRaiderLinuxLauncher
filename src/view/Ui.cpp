#include "view/Ui.hpp"
#include "view/Levels/Select/Filter.hpp"
#include <qcombobox.h>

Ui::Ui(QWidget *parent)
    : QWidget(parent),
    tabs(new QTabWidget(this)),
    levels(new UiLevels(tabs)),
    modding(new UiModding(tabs)),
    uicontroller(new UiController(tabs)),
    setup(new UiSetup(tabs)),
    about(new UiAbout(tabs)),
    layout(new QGridLayout(this))
{
    this->setObjectName("centralwidget");

    layout->setContentsMargins(9, 9, 9, 9);
    layout->setSpacing(8);
    tabs->addTab(levels, tr("Levels"));
    tabs->addTab(modding, tr("Modding"));
    tabs->setTabEnabled(tabs->indexOf(
            tabs->findChild<QWidget*>("Modding")), false);
    tabs->addTab(uicontroller, tr("Controller"));
    tabs->setTabEnabled(tabs->indexOf(
            tabs->findChild<QWidget*>("Controller")), false);
    tabs->addTab(setup, tr("Setup"));
    tabs->addTab(about, tr("About"));
    layout->addWidget(tabs);


    // Button signal connections
    // List tab
    NavigateWidgetBar* nbar = this->levels->select->stackedWidgetBar->navigateWidgetBar;
    InfoBar* ibar = this->levels->info->infoBar;
    SetupInput* sfbar = this->setup->firstTime->setupInput;
    connect(nbar->pushButtonDownload, SIGNAL(clicked()),
            this, SLOT(downloadOrRemoveClicked()));
    connect(levels, SIGNAL(downloadOrRemoveClickedSignal()),
            this, SLOT(downloadOrRemoveClicked()));

    connect(sfbar->setOptions, SIGNAL(clicked()), this, SLOT(setOptionsClicked()));

    LevelControl* sslbar = setup->settings->frameLevelSetup->levelControl;
    connect(sslbar->commandLinkButtonLSSave, SIGNAL(clicked()),
            this, SLOT(levelSaveClicked()));
    connect(sslbar->commandLinkButtonLSReset, SIGNAL(clicked()),
            this, SLOT(levelResetClicked()));

    connect(
            this->levels->select->levelViewList->selectionModel(),
            &QItemSelectionModel::currentChanged,
            this, &Ui::onCurrentItemChanged);

    // Key shortCuts
    setShortCuts();

    // Read settings
    QString value = g_settings.value("setup").toString();
    if (value != "yes") {
        this->startUpSetup();
    } else {
        setup->readSavedSettings();
        controller.setup();
    }
}

void Ui::setShortCuts() {
    // Focus Search
    QAction * focusSearchAction = new QAction(this);
    focusSearchAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_S));
    focusSearchAction->setShortcutContext(Qt::WindowShortcut);
    connect(focusSearchAction, &QAction::triggered,
            this, [this]() -> void {
        if (this->tabs->currentWidget() == this->levels) {
            if (this->levels->stackedWidget->currentWidget() ==
                    this->levels->select) {
                if (this->levels->select->filter->isHidden()) {
                    this->levels->select->stackedWidgetBar->
                        navigateWidgetBar->pushButtonFilter->click();
                }
                this->levels->select->filter->filterFirstInputRow->
                filterGroupBoxSearch->lineEditSearch->setFocus();
            }
        }
    });
    tabs->addAction(focusSearchAction);

    // Search Type
    QAction * setSearchTypeAction = new QAction(this);
    setSearchTypeAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_T));
    setSearchTypeAction->setShortcutContext(Qt::WindowShortcut);
    connect(setSearchTypeAction, &QAction::triggered,
            this, [this]() -> void {
        if (this->tabs->currentWidget() == this->levels) {
            if (this->levels->stackedWidget->currentWidget() ==
                    this->levels->select) {
                if (this->levels->select->filter->isHidden()) {
                    this->levels->select->stackedWidgetBar->
                        navigateWidgetBar->pushButtonFilter->click();
                }
                QComboBox *cb =
                this->levels->select->filter->filterFirstInputRow->
                    filterGroupBoxSearch->comboBoxSearch;
                cb->setFocus(Qt::ShortcutFocusReason);
                cb->showPopup();
            }
        }
    });
    tabs->addAction(setSearchTypeAction);

    // Installed Only
    QAction * setInstalledOnlyAction = new QAction(this);
    setInstalledOnlyAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_N));
    setInstalledOnlyAction->setShortcutContext(Qt::WindowShortcut);
    connect(setInstalledOnlyAction, &QAction::triggered,
            this, [this]() -> void {
        if (this->tabs->currentWidget() == this->levels) {
            if (this->levels->stackedWidget->currentWidget() ==
                    this->levels->select) {
                this->levels->select->filter->
                    filterSecondInputRow->filterGroupBoxToggle->
                        checkBoxInstalled->click();
            }
        }
    });
    tabs->addAction(setInstalledOnlyAction);

    // Focus Level List
    QAction * focusListAction = new QAction(this);
    focusListAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_L));
    focusListAction->setShortcutContext(Qt::WindowShortcut);
    connect(focusListAction, &QAction::triggered,
            this, [this]() -> void {
        if (this->tabs->currentWidget() == this->levels) {
            if (this->levels->stackedWidget->currentWidget() ==
                    this->levels->select) {
                        this->levels->select->
                            levelViewList->setFocus();
            }
        }
    });
    tabs->addAction(focusListAction);

    // Toggle Setup
    QAction * toggleSetupAction = new QAction(this);
    toggleSetupAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_U));
    toggleSetupAction->setShortcutContext(Qt::WindowShortcut);
    connect(toggleSetupAction, &QAction::triggered,
            this, [this]() -> void {
        if (this->tabs->currentWidget() == this->levels) {
            if (this->levels->stackedWidget->currentWidget() ==
                        this->levels->select) {
                    this->levels->select->stackedWidgetBar->
                        navigateWidgetBar->checkBoxSetup->click();
            }
        }
    });
    tabs->addAction(toggleSetupAction);

    // Show/Hide Filter
    QAction * toggleFilterAction = new QAction(this);
    toggleFilterAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_F));
    toggleFilterAction->setShortcutContext(Qt::WindowShortcut);
    connect(toggleFilterAction, &QAction::triggered,
            this, [this]() -> void {
        if (this->tabs->currentWidget() == this->levels) {
            if (this->levels->stackedWidget->currentWidget() ==
                        this->levels->select) {
                    this->levels->select->stackedWidgetBar->
                        navigateWidgetBar->pushButtonFilter->click();
            }
        }
    });
    tabs->addAction(toggleFilterAction);

    // Info
    QAction * infoAction = new QAction(this);
    infoAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_I));
    infoAction->setShortcutContext(Qt::WindowShortcut);
    connect(infoAction, &QAction::triggered,
            this, [this]() -> void {
        if (this->tabs->currentWidget() == this->levels) {
            if (this->levels->stackedWidget->currentWidget() ==
                        this->levels->select) {
                this->levels->select->stackedWidgetBar->
                    navigateWidgetBar->pushButtonInfo->click();
            }
        }
    });
    tabs->addAction(infoAction);

    // Go Back
    QAction * backAction = new QAction(this);
    backAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_B));
    backAction->setShortcutContext(Qt::WindowShortcut);
    connect(backAction, &QAction::triggered,
            this, [this]() -> void {
        if (this->tabs->currentWidget() == this->levels) {
            if (this->levels->stackedWidget->currentWidget() ==
                        this->levels->info) {
                this->levels->info->infoBar->pushButtonBack->click();
            }
        }
    });
    tabs->addAction(backAction);

    // Walkthrough
    QAction * walkthroughAction = new QAction(this);
    walkthroughAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_W));
    walkthroughAction->setShortcutContext(Qt::WindowShortcut);
    connect(walkthroughAction, &QAction::triggered,
            this, [this]() -> void {
        if (this->tabs->currentWidget() == this->levels) {
            if (this->levels->stackedWidget->currentWidget() ==
                        this->levels->info) {
                this->levels->info->infoBar->pushButtonWalkthrough->click();
            }
        }
    });
    tabs->addAction(walkthroughAction);

    // Select Filter
    QAction * selectFilterAction = new QAction(this);
    selectFilterAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Y));
    selectFilterAction->setShortcutContext(Qt::WindowShortcut);
    connect(selectFilterAction, &QAction::triggered,
            this, [this]() -> void {
        if (this->tabs->currentWidget() == this->levels) {
            if (this->levels->stackedWidget->currentWidget() ==
                        this->levels->select) {
                if (this->levels->select->filter->isHidden()) {
                    this->levels->select->stackedWidgetBar->
                        navigateWidgetBar->pushButtonFilter->click();
                }
                this->levels->select->filter->filterFirstInputRow->
                    filterGroupBoxFilter->showFilterSelectionMenu();
            }
        }
    });
    tabs->addAction(selectFilterAction);

    // Select Sort
    QAction * selectSortAction = new QAction(this);
    selectSortAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_R));
    selectSortAction->setShortcutContext(Qt::WindowShortcut);
    connect(selectSortAction, &QAction::triggered,
            this, [this]() -> void {
        if (this->tabs->currentWidget() == this->levels) {
            if (this->levels->stackedWidget->currentWidget() ==
                        this->levels->select) {
                if (this->levels->select->filter->isHidden()) {
                    this->levels->select->stackedWidgetBar->
                        navigateWidgetBar->pushButtonFilter->click();
                }
                this->levels->select->filter->filterSecondInputRow->
                    filterGroupBoxSort->focusSelected();
            }
        }
    });
    tabs->addAction(selectSortAction);

    // Download
    QAction * downloadAction = new QAction(this);
    downloadAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_D));
    downloadAction->setShortcutContext(Qt::WindowShortcut);
    connect(downloadAction, &QAction::triggered,
            this, [this]() -> void {
        if (this->tabs->currentWidget() == this->levels) {
            if (this->levels->stackedWidget->currentWidget() ==
                        this->levels->select) {
                this->levels->select->stackedWidgetBar->
                    navigateWidgetBar->pushButtonDownload->click();
            }
        }
    });
    tabs->addAction(downloadAction);

}

void Ui::onCurrentItemChanged(
        const QModelIndex &current, const QModelIndex &previous) {
    if (current.isValid()) {
        levels->setItemChanged(current);
        qint64 id = levels->getItemId();
        setup->setState(id);
        QString runnerTypeText = g_uistate.getRunnerTypeText();
        g_uistate.setRunText(runnerTypeText);
    }
}

void Ui::levelSaveClicked() {
    qint64 id = levels->select->getLid();
    if (id != 0) {
        setup->levelSaveClicked(id);
    }
    QString runnerTypeText = g_uistate.getRunnerTypeText();
    g_uistate.setRunText(runnerTypeText);
}

void Ui::levelResetClicked() {
    qint64 id = levels->getItemId();
    if (id != 0) {
        setup->levelResetClicked(id);
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

void Ui::setOptionsClicked() {
    this->tabs->setTabEnabled(this->tabs->indexOf(
            this->tabs->findChild<QWidget*>("Levels")), true);
    this->tabs->setTabEnabled(this->tabs->indexOf(
            this->tabs->findChild<QWidget*>("Modding")), false);
    this->tabs->show();
    this->tabs->setCurrentIndex(this->tabs->indexOf(
            this->tabs->findChild<QWidget*>("Levels")));
    this->setup->setOptionsClicked();
    setup->readSavedSettings();
    controller.setup();
}
