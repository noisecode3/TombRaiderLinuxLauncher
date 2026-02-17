#include "view/Ui.hpp"
#include "view/Levels/Select/Filter.hpp"
#include <qcombobox.h>
#include <qnamespace.h>

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


    // Button bar signal connections
    NavigateWidgetBar* nbar = this->levels->select->stackedWidgetBar->navigateWidgetBar;
    InfoBar* ibar = this->levels->info->infoBar;

    // Button signal connections
    SetupInput* sfbar = this->setup->firstTime->setupInput;
    LevelControl* sslbar = setup->settings->frameLevelSetup->levelControl;

    connect(nbar->pushButtonDownload, &QPushButton::clicked,
            this, &Ui::downloadOrRemoveClicked);

    connect(levels, &UiLevels::downloadOrRemoveClickedSignal,
            this, &Ui::downloadOrRemoveClicked);

    connect(sfbar->setOptions, &QPushButton::clicked,
            this, &Ui::setOptionsClicked);

    connect(sslbar->commandLinkButtonLSSave, &QPushButton::clicked,
            this, &Ui::levelSaveClicked);

    connect(sslbar->commandLinkButtonLSReset, &QPushButton::clicked,
            this, &Ui::levelResetClicked);

    connect(this->levels->select->levelViewList->selectionModel(),
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

void Ui::connectShortCut(const QKeySequence& seq,
                         std::function<void()> callback)
{
    QAction* action = new QAction(this);
    action->setShortcut(seq);
    action->setShortcutContext(Qt::WindowShortcut);

    connect(action, &QAction::triggered, this, std::move(callback));

    this->tabs->addAction(action);
}

void Ui::setShortCuts() {
    // Focus Search
    connectShortCut(QKeySequence(Qt::CTRL | Qt::Key_S),
        [this]() -> void {
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
        // Setup save
        if (this->tabs->currentWidget() == this->setup) {
            this->setup->settings->frameLevelSetup->
                levelControl->commandLinkButtonLSSave->click();
        }
    });

    // Search Type
    connectShortCut(QKeySequence(Qt::CTRL | Qt::Key_T),
        [this]() -> void {
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

    // Installed Only
    connectShortCut(QKeySequence(Qt::CTRL | Qt::Key_N),
        [this]() -> void {
        if (this->tabs->currentWidget() == this->levels) {
            if (this->levels->stackedWidget->currentWidget() ==
                    this->levels->select) {
                this->levels->select->filter->
                    filterSecondInputRow->filterGroupBoxToggle->
                        checkBoxInstalled->click();
                this->levels->select->levelViewList->setFocus();
            }
        }
    });

    // Focus Level List
    connectShortCut(QKeySequence(Qt::CTRL | Qt::Key_L),
        [this]() -> void {
        if (this->tabs->currentWidget() == this->levels) {
            if (this->levels->stackedWidget->currentWidget() ==
                    this->levels->select) {
                        this->levels->select->
                            levelViewList->setFocus();
            }
        }
    });

    // Toggle Setup
    connectShortCut(QKeySequence(Qt::CTRL | Qt::Key_U),
        [this]() -> void {
        if (this->tabs->currentWidget() == this->levels) {
            if (this->levels->stackedWidget->currentWidget() ==
                        this->levels->select) {
                this->levels->select->stackedWidgetBar->
                    navigateWidgetBar->checkBoxSetup->click();
                this->levels->select->levelViewList->setFocus();
            }
        }
    });

    // Show/Hide Filter
    connectShortCut(QKeySequence(Qt::CTRL | Qt::Key_F),
        [this]() -> void {
        if (this->tabs->currentWidget() == this->levels) {
            if (this->levels->stackedWidget->currentWidget() ==
                        this->levels->select) {
                this->levels->select->stackedWidgetBar->
                    navigateWidgetBar->pushButtonFilter->click();
                this->levels->select->levelViewList->setFocus();
            }
        }
    });

    // Info
    connectShortCut(QKeySequence(Qt::CTRL | Qt::Key_I),
        [this]() -> void {
        if (this->tabs->currentWidget() == this->levels) {
            // Activate info
            if (this->levels->stackedWidget->currentWidget() ==
                        this->levels->select) {
                this->levels->select->stackedWidgetBar->
                    navigateWidgetBar->pushButtonInfo->click();
            }
            // Info text focus
            if (this->levels->stackedWidget->currentWidget() ==
                        this->levels->info) {
                this->levels->info->infoContent->infoWebEngineView->setFocus();
            }
        }
    });

    // Info/Walkthrough Go Back
    connectShortCut(QKeySequence(Qt::CTRL | Qt::Key_B),
        [this]() -> void {
        if (this->tabs->currentWidget() == this->levels) {
            if (this->levels->stackedWidget->currentWidget() ==
                        this->levels->info) {
                this->levels->info->infoBar->pushButtonBack->click();
            }
        }
    });

    // Info Go to Walkthrough
    connectShortCut(QKeySequence(Qt::CTRL | Qt::Key_W),
        [this]() -> void {
        if (this->tabs->currentWidget() == this->levels) {
            if (this->levels->stackedWidget->currentWidget() ==
                        this->levels->info) {
                this->levels->info->infoBar->pushButtonWalkthrough->click();
            }
        }
    });


    // Info coverList focus
    connectShortCut(QKeySequence(Qt::CTRL | Qt::Key_C),
        [this]() -> void {
        if (this->tabs->currentWidget() == this->levels) {
            if (this->levels->stackedWidget->currentWidget() ==
                        this->levels->info) {
                this->levels->info->infoContent->coverListWidget->setFocus();
            }
        }
    });

    // Select Filter
    connectShortCut(QKeySequence(Qt::CTRL | Qt::Key_Y),
        [this]() -> void {
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

    // Select Sort
    connectShortCut(QKeySequence(Qt::CTRL | Qt::Key_R),
        [this]() -> void {
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

        // Setup reset
        if (this->tabs->currentWidget() == this->setup) {
            this->setup->settings->frameLevelSetup->
                levelControl->commandLinkButtonLSReset->click();
        }
    });

    // Download
    connectShortCut(QKeySequence(Qt::CTRL | Qt::Key_D),
        [this]() -> void {
        if (this->tabs->currentWidget() == this->levels) {
            if (this->levels->stackedWidget->currentWidget() ==
                        this->levels->select) {
                this->levels->select->stackedWidgetBar->
                    navigateWidgetBar->pushButtonDownload->click();
            }
        }
    });

    // Level Tab
    connectShortCut(QKeySequence(Qt::ALT | Qt::Key_L),
        [this]() -> void {
        if (this->tabs->currentWidget() != this->levels) {
            this->tabs->setCurrentWidget(this->levels);
            this->levels->select->levelViewList->setFocus();
        }
    });

    // Setup Tab
    connectShortCut(QKeySequence(Qt::ALT | Qt::Key_U),
        [this]() -> void {
        if (this->tabs->currentWidget() != this->setup) {
            this->tabs->setCurrentWidget(this->setup);
            this->setup->settings->frameLevelSetup->
                frameLevelSetupSettings->widgetEnvironmentVariables->
                    lineEditEnvironmentVariables->setFocus();
        }
    });

    // Setup environment
    connectShortCut(QKeySequence(Qt::CTRL | Qt::Key_E),
        [this]() -> void {
        if (this->tabs->currentWidget() == this->setup) {
            this->setup->settings->frameLevelSetup->
                frameLevelSetupSettings->widgetEnvironmentVariables->
                    lineEditEnvironmentVariables->setFocus();
        }
    });
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

