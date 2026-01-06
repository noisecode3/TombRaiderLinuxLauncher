#include "Setup.hpp"
#include <qnamespace.h>

UiSetup::UiSetup(QWidget *parent)
    : QWidget{parent},
    stackedWidget(new QStackedWidget(this)),
    firstTime(new FirstTime(stackedWidget)),
    settings(new Settings(stackedWidget))
{
    setObjectName("Setup");

    layout = new QVBoxLayout(this);
    layout->setContentsMargins(6, 6, 6, 6);
    layout->setSpacing(8);

    layout->addWidget(stackedWidget);

    stackedWidget->addWidget(firstTime);
    stackedWidget->addWidget(settings);
    stackedWidget->setCurrentWidget(settings);
}


void UiSetup::readSavedSettings() {
    const QString gamePath = g_settings.value("gamePath").toString();
    QTableWidget* table = this->settings->frameGlobalSetup->tableWidgetGlobalSetup;
    table->item(0, 0)->setText(gamePath);
    qDebug() << "Read game path value:" << gamePath;

    const QString extraGamePath = g_settings.value("extraGamePath").toString();
    table->item(1, 0)->setText(extraGamePath);
    qDebug() << "Read extra game path value:" << extraGamePath;

    const QString levelPath = g_settings.value("levelPath").toString();
    table->item(2, 0)->setText(levelPath);
    qDebug() << "Read level path value:" << levelPath;

    WidgetDeleteZip* widgetDeleteZip = this->settings->frameGlobalSetup->widgetDeleteZip;
    const bool deleteZip = g_settings.value("DeleteZip").toBool();
    widgetDeleteZip->checkBoxDeleteZip->setChecked(deleteZip);
    qDebug() << "Read level DeleteZip (after download) value:" << deleteZip;

    WidgetDefaultEnvironmentVariables* widgetDefaultEnvironmentVariables =
        this->settings->frameGlobalSetup->widgetDefaultEnvironmentVariables;
    const QString defaultEnvironmentVariables =
            g_settings.value("defaultEnvironmentVariables").toString();
    widgetDefaultEnvironmentVariables->lineEditDefaultEnvironmentVariables->
            setText(defaultEnvironmentVariables);
    qDebug() << "Read defaultEnvironmentVariables value:"
            << defaultEnvironmentVariables;

    WidgetDefaultRunnerType* widgetDefaultRunnerType =
        this->settings->frameGlobalSetup->widgetDefaultRunnerType;
    const quint64 defaultRunnerType =
            g_settings.value("defaultRunnerType").toInt();
    widgetDefaultRunnerType->comboBoxDefaultRunnerType->setCurrentIndex(defaultRunnerType);
    qDebug() << "Read defaultRunnerType value:" << defaultRunnerType;

}

void UiSetup::downloadClicked(qint64 id) {
    this->settings->frameLevelSetup->frameLevelSetupSettings->
        widgetEnvironmentVariables->lineEditEnvironmentVariables->setText(
            g_settings.value("defaultEnvironmentVariables").toString());
    g_settings.setValue(QString("level%1/EnvironmentVariables")
            .arg(id), this->settings->frameLevelSetup->frameLevelSetupSettings->
        widgetEnvironmentVariables->lineEditEnvironmentVariables->text());

    this->settings->frameLevelSetup->frameLevelSetupSettings->
        widgetRunnerType->comboBoxRunnerType->setCurrentIndex(
            g_settings.value("defaultRunnerType").toInt());
    g_settings.setValue(QString("level%1/RunnerType")
            .arg(id), this->settings->frameLevelSetup->frameLevelSetupSettings->
        widgetRunnerType->comboBoxRunnerType->currentIndex());

}

void UiSetup::setOptionsClicked() {
    SetupInput* setupInput = this->firstTime->setupInput;
    QString gamePath =  setupInput->gamePathContainer->gamePathEdit->text();
    QString extraGamePath = setupInput->extraGamePathContainer->extraGamePathEdit->text();
    QString levelPath = setupInput->levelPathContainer->levelPathEdit->text();
    g_settings.setValue("gamePath", gamePath);
    g_settings.setValue("extraGamePath", extraGamePath);
    g_settings.setValue("levelPath", levelPath);
    g_settings.setValue("setup", "yes");

    this->settings->frameGlobalSetup->
            tableWidgetGlobalSetup->item(0, 0)->setText(gamePath);
    this->settings->frameGlobalSetup->
            tableWidgetGlobalSetup->item(1, 0)->setText(extraGamePath);
    this->settings->frameGlobalSetup->
            tableWidgetGlobalSetup->item(2, 0)->setText(levelPath);
}

void UiSetup::GlobalSaveClicked() {
    const QString newGamePath =
        this->settings->frameGlobalSetup->
                tableWidgetGlobalSetup->item(0, 0)->text();
    const QString newExtraGamePath =
        this->settings->frameGlobalSetup->
                tableWidgetGlobalSetup->item(1, 0)->text();
    const QString newLevelPath =
        this->settings->frameGlobalSetup->
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
        this->settings->frameGlobalSetup->
            widgetDeleteZip->checkBoxDeleteZip->isChecked();
    g_settings.setValue("DeleteZip" , newDeleteZip);

    g_settings.setValue("defaultEnvironmentVariables",
        this->settings->frameGlobalSetup->widgetDefaultEnvironmentVariables->
            lineEditDefaultEnvironmentVariables->text());

    g_settings.setValue("defaultRunnerType",
        this->settings->frameGlobalSetup->widgetDefaultRunnerType->
            comboBoxDefaultRunnerType->currentIndex());
}

void UiSetup::GlobalResetClicked() {
    this->settings->frameGlobalSetup->
        tableWidgetGlobalSetup->item(0, 0)->setText(
            g_settings.value("gamePath").toString());
    this->settings->frameGlobalSetup->
        tableWidgetGlobalSetup->item(1, 0)->setText(
            g_settings.value("extraGamePath").toString());
    this->settings->frameGlobalSetup->
        tableWidgetGlobalSetup->item(2, 0)->setText(
            g_settings.value("levelPath").toString());

    this->settings->frameGlobalSetup->
        widgetDeleteZip->checkBoxDeleteZip->setChecked(
            g_settings.value("DeleteZip").toBool());

    this->settings->frameGlobalSetup->widgetDefaultEnvironmentVariables->
        lineEditDefaultEnvironmentVariables->setText(
            g_settings.value("defaultEnvironmentVariables").toString());

    this->settings->frameGlobalSetup->widgetDefaultRunnerType->
        comboBoxDefaultRunnerType->setCurrentIndex(
            g_settings.value("defaultRunnerType").toInt());
}

void UiSetup::setState(qint64 id) {
        FrameLevelSetupSettings* levelSettings =
            this->settings->frameLevelSetup->frameLevelSetupSettings;
        levelSettings->widgetLevelID->lcdNumberLevelID->display(QString::number(id));
        levelSettings->widgetEnvironmentVariables->lineEditEnvironmentVariables->setEnabled(true);
        levelSettings->widgetEnvironmentVariables->lineEditEnvironmentVariables->setText(
                g_settings.value(QString("level%1/EnvironmentVariables")
                    .arg(id)).toString());
        levelSettings->widgetRunnerType->comboBoxRunnerType->setEnabled(true);
        levelSettings->widgetRunnerType->comboBoxRunnerType->setCurrentIndex(
                g_settings.value(QString("level%1/RunnerType")
                    .arg(id)).toInt());
        LevelControl* levelControl = this->settings->frameLevelSetup->levelControl;
        levelControl->commandLinkButtonLSSave->setEnabled(true);
        levelControl->commandLinkButtonLSReset->setEnabled(true);
}
QString UiSetup::getRunnerTypeState() {
        FrameLevelSetupSettings* levelSettings =
            this->settings->frameLevelSetup->frameLevelSetupSettings;
        return levelSettings->widgetRunnerType->comboBoxRunnerType->currentText();
}

FirstTime::FirstTime(QWidget *parent)
    : QWidget{parent},
    setupImage(new SetupImage(this)),
    setupInput(new SetupInput(this)),
    layout(new QHBoxLayout(this))
{
    setObjectName("firstTime");

    layout->setContentsMargins(6, 6, 6, 6);
    layout->setSpacing(8);

    layout->addWidget(setupImage);
    layout->addWidget(setupInput);
}

SetupImage::SetupImage(QWidget *parent)
    : QWidget(parent),
    lara(new QLabel(this)),
    layout(new QVBoxLayout(this))
{
    layout->setContentsMargins(6, 6, 6, 6);
    layout->setSpacing(8);
    this->setMinimumWidth(620);

    QPixmap pix(":/pictures/Lara.png");

    lara->setPixmap(pix);
    lara->setFixedSize(pix.size());
    lara->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

    layout->addWidget(lara);
}

GamePathContainer::GamePathContainer(QWidget *parent)
    : QWidget(parent),
    gamePathLabel(new QLabel(tr("Game Directory"), this)),
    gamePathEdit(new QLineEdit(this)),
    layout(new QHBoxLayout(this))
{
    layout->setContentsMargins(6, 6, 6, 6);
    layout->setSpacing(8);

    layout->addWidget(gamePathLabel);

    gamePathEdit->setMaximumWidth(553);
    layout->addWidget(gamePathEdit);

    this->setMaximumHeight(42);
}

ExtraGamePathContainer::ExtraGamePathContainer(QWidget *parent)
    : QWidget(parent),
    extraGamePathLabel(new QLabel(tr("Extra Game Directory"), this)),
    extraGamePathEdit(new QLineEdit(this)),
    layout(new QHBoxLayout(this))
{
    layout->setContentsMargins(6, 6, 6, 6);
    layout->setSpacing(8);

    layout->addWidget(extraGamePathLabel);

    extraGamePathEdit->setMaximumWidth(521);
    layout->addWidget(extraGamePathEdit);

    this->setMaximumHeight(42);
}

LevelPathContainer::LevelPathContainer(QWidget *parent)
    : QWidget(parent),
    levelPathLabel(new QLabel(tr("Level Directory"), this)),
    levelPathEdit(new QLineEdit(this)),
    layout(new QHBoxLayout(this))
{
    layout->setContentsMargins(6, 6, 6, 6);
    layout->setSpacing(8);

    layout->addWidget(levelPathLabel);

    levelPathEdit->setMaximumWidth(571);
    layout->addWidget(levelPathEdit);

    this->setMaximumHeight(42);
}

SetupInput::SetupInput(QWidget *parent)
    : QWidget(parent),
    labelSetupDirectoryInfo(new QLabel(this)),
    labelGameDirectoryInfo(new QLabel(this)),
    gamePathContainer(new GamePathContainer(this)),
    labelExtraGameDirectoryInfo(new QLabel(this)),
    extraGamePathContainer(new ExtraGamePathContainer(this)),
    labelLevelDirectoryInfo(new QLabel(this)),
    levelPathContainer(new LevelPathContainer(this)),
    setOptions(new QPushButton(tr("set options"),this)),
    layout(new QVBoxLayout(this))
{
    layout->setContentsMargins(6, 6, 6, 6);
    layout->setSpacing(8);
    this->setMaximumWidth(556);

    QFont font;
    font.setFamily("Sans");

    labelSetupDirectoryInfo->setWordWrap(true);
    labelSetupDirectoryInfo->setAlignment(Qt::AlignCenter);

    labelSetupDirectoryInfo->setText(
        "The path must not contain escape sequences.\n"
        "For example, \"TombRaider (III)/\" instead of \"TombRaider\\ \\(III\\)/\".\n"
        "It should also be a full path, meaning it must start with \"/\".\n\n"
        "Example:/home/lara/.steam/steam/steamapps/common/TombRaider (III)/\n\n"
        "The default values should already be correct."
        );
    labelSetupDirectoryInfo->setFont(font);

    layout->addWidget(labelSetupDirectoryInfo);

    labelGameDirectoryInfo->setWordWrap(true);
    labelGameDirectoryInfo->setAlignment(Qt::AlignCenter | Qt::AlignBottom);

    labelGameDirectoryInfo->setText(
        "This is usually the Steam “common” folder:\n"
        "/home/username/.steam/steam/steamapps/common"
        );
    labelGameDirectoryInfo->setFont(font);

    layout->addWidget(labelGameDirectoryInfo);

    layout->addWidget(gamePathContainer);

    labelExtraGameDirectoryInfo->setWordWrap(true);
    labelExtraGameDirectoryInfo->setAlignment(Qt::AlignCenter | Qt::AlignBottom);

    labelExtraGameDirectoryInfo->setText(
        "This is where you would install Core Design games from a CD:\n"
        "/home/username/.wine/drive_c/Program Files (x86)"
        );
    labelExtraGameDirectoryInfo->setFont(font);
    layout->addWidget(labelExtraGameDirectoryInfo);

    layout->addWidget(extraGamePathContainer);

    labelLevelDirectoryInfo->setWordWrap(true);
    labelLevelDirectoryInfo->setAlignment(Qt::AlignCenter | Qt::AlignBottom);

    labelLevelDirectoryInfo->setText(
        "In this directory, custom level and game save files will be stored.\n"
        "A database for indexing file and level data will also be stored here.\n"
        "Each custom map from trle.net has its own game directory,\n"
        "with custom asset files and various builds or modifications for every map."
        );
    labelLevelDirectoryInfo->setFont(font);
    layout->addWidget(labelLevelDirectoryInfo);

    layout->addWidget(levelPathContainer);

    setOptions->setMinimumSize(0, 32);
    setOptions->setMaximumSize(242, 32);
    layout->addWidget(setOptions, Qt::AlignBottom);
}

Settings::Settings(QWidget *parent)
    : QFrame(parent),
    frameGlobalSetup(new FrameGlobalSetup(this)),
    frameLevelSetup(new FrameLevelSetup(this)),
    layout(new QHBoxLayout(this))
{
    layout->setContentsMargins(6, 6, 6, 6);
    layout->setSpacing(8);

    layout->addWidget(frameGlobalSetup);
    layout->addWidget(frameLevelSetup);
}

FrameGlobalSetup::FrameGlobalSetup(QWidget *parent)
    : QFrame(parent),
    tableWidgetGlobalSetup(new QTableWidget(this)),
    widgetDefaultEnvironmentVariables(new WidgetDefaultEnvironmentVariables(this)),
    widgetDefaultRunnerType(new WidgetDefaultRunnerType(this)),
    widgetDeleteZip(new WidgetDeleteZip(this)),
    labelGlobalSetupPicture(new QLabel(this)),
    globalControl(new GlobalControl(this)),
    layout(new QVBoxLayout(this))
{
    layout->setContentsMargins(6, 6, 6, 6);
    layout->setSpacing(8);


    tableWidgetGlobalSetup->setRowCount(3);
    tableWidgetGlobalSetup->setColumnCount(1);
    tableWidgetGlobalSetup->setSizePolicy(
        QSizePolicy::Preferred,
        QSizePolicy::Fixed
    );

    tableWidgetGlobalSetup->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    tableWidgetGlobalSetup->setSizeAdjustPolicy(
        QAbstractScrollArea::AdjustToContents
    );

    tableWidgetGlobalSetup->setTabKeyNavigation(false);
    tableWidgetGlobalSetup->setSelectionMode(QAbstractItemView::NoSelection);

    tableWidgetGlobalSetup->setHorizontalHeaderItem(
        0, new QTableWidgetItem(tr("Paths"))
    );

    tableWidgetGlobalSetup->setVerticalHeaderItem(
        0, new QTableWidgetItem("GAMEDIR")
    );
    tableWidgetGlobalSetup->setVerticalHeaderItem(
        1, new QTableWidgetItem("EXTRAGAMEDIR")
    );
    tableWidgetGlobalSetup->setVerticalHeaderItem(
        2, new QTableWidgetItem("RESOURCEDIR")
    );

    for (int row = 0; row < 3; ++row) {
        QTableWidgetItem *item = new QTableWidgetItem();
        item->setFlags(Qt::ItemIsEditable | Qt::ItemIsEnabled);
        tableWidgetGlobalSetup->setItem(row, 0, item);
    }

    layout->addWidget(tableWidgetGlobalSetup);
    layout->addWidget(widgetDefaultEnvironmentVariables);
    layout->addWidget(widgetDefaultRunnerType);
    layout->addWidget(widgetDeleteZip);
    layout->addWidget(labelGlobalSetupPicture);
    layout->addWidget(globalControl);
}

WidgetDefaultEnvironmentVariables::WidgetDefaultEnvironmentVariables(QWidget *parent)
    : QWidget(parent),
    labelDefaultEnvironmentVariables(new QLabel(this)),
    lineEditDefaultEnvironmentVariables(new QLineEdit(this)),
    layout(new QHBoxLayout(this))
{
    layout->setContentsMargins(6, 6, 6, 6);
    layout->setSpacing(8);

    layout->addWidget(labelDefaultEnvironmentVariables);
    layout->addWidget(lineEditDefaultEnvironmentVariables);
}

WidgetDefaultRunnerType::WidgetDefaultRunnerType(QWidget *parent)
    : QWidget(parent),
    labelDefaultRunnerType(new QLabel(this)),
    comboBoxDefaultRunnerType(new QComboBox(this)),
    layout(new QHBoxLayout(this))
{
    layout->setContentsMargins(6, 6, 6, 6);
    layout->setSpacing(8);

    layout->addWidget(labelDefaultRunnerType);

    comboBoxDefaultRunnerType->setMinimumHeight(120);
    comboBoxDefaultRunnerType->setEnabled(false);
    comboBoxDefaultRunnerType->addItems(QStringList()
        << "Umu-launcher"
        << "Wine"
        << "Lutris"
        << "Link and launch Lutris"
        << "Link and launch Steam"
        << "Link and exit"
        << "Just link"
        << "Bash"
        );
    layout->addWidget(comboBoxDefaultRunnerType);
}

WidgetDeleteZip::WidgetDeleteZip(QWidget *parent)
    : QWidget(parent),
    checkBoxDeleteZip(new QCheckBox(this)),
    layout(new QHBoxLayout(this))
{
    layout->setContentsMargins(6, 6, 6, 6);
    layout->setSpacing(8);

    layout->addWidget(checkBoxDeleteZip);
}

FrameLevelSetup::FrameLevelSetup(QWidget *parent)
    : QFrame(parent),
    labelLevelSetup(new QLabel(this)),
    frameLevelSetupSettings(new FrameLevelSetupSettings(this)),
    levelControl(new LevelControl(this)),
    layout(new QVBoxLayout(this))
{
    layout->setContentsMargins(6, 6, 6, 6);
    layout->setSpacing(8);

    layout->addWidget(labelLevelSetup);
    layout->addWidget(frameLevelSetupSettings);
    layout->addWidget(levelControl);
}

FrameLevelSetupSettings::FrameLevelSetupSettings(QWidget *parent)
    : QFrame(parent),
    widgetEnvironmentVariables(new WidgetEnvironmentVariables(this)),
    widgetRunnerType(new WidgetRunnerType(this)),
    widgetLevelID(new WidgetLevelID(this)),
    label(new QLabel(this)),
    layout(new QVBoxLayout(this))
{
    layout->setContentsMargins(6, 6, 6, 6);
    layout->setSpacing(8);

    layout->addWidget(widgetEnvironmentVariables);
    layout->addWidget(widgetRunnerType);
    layout->addWidget(widgetLevelID);
    label->setWordWrap(true);
    label->setAlignment(Qt::AlignCenter);

    label->setText(
        "Runs system Wine/Proton from RESOURCEDIR/lid.TREL\n"
        "Set environment variables like:\n"
        "WINEDLLOVERRIDES=winmm=n,b;ddraw=n,b WINEFSYNC=1\n\n"
        "With Umu-launcher:\n"
        "WINEPREFIX=~/.tombfix PROTONPATH=~/GE-Proton8-27/proton\n\n"
        "With Lutris (check lutris -h)"
        "Use game ID, e.g., tomb-raider-2\n\n"
        "With Bash:\n"
        "Executes RESOURCEDIR/lid.TREL/start.sh (-setup might be in $1)\n\n"
        "With Link:\n"
        "Creates symlink to RESOURCEDIR/lid.TREL/game.exe in GAMEDIR\n"
        "Chooses correct game type\n\n"
        "Link and Exit: links then exits\n\n"
        "Link and Launch Steam: also launches Steam game based on level type\n"
        );
    layout->addWidget(label);
}

WidgetEnvironmentVariables::WidgetEnvironmentVariables(QWidget* parent)
    : QWidget(parent),
    labelEnvironmentVariables(new QLabel(this)),
    lineEditEnvironmentVariables(new QLineEdit(this)),
    layout(new QHBoxLayout(this))
{
    layout->setContentsMargins(6, 6, 6, 6);
    layout->setSpacing(8);

    layout->addWidget(labelEnvironmentVariables);
    layout->addWidget(lineEditEnvironmentVariables);
}

WidgetRunnerType::WidgetRunnerType(QWidget* parent)
    : QWidget(parent),
    labelRunnerType(new QLabel(this)),
    comboBoxRunnerType(new QComboBox(this)),
    layout(new QHBoxLayout(this))
{
    layout->setContentsMargins(6, 6, 6, 6);
    layout->setSpacing(8);

    layout->addWidget(labelRunnerType);

    comboBoxRunnerType->setMinimumHeight(120);
    comboBoxRunnerType->setEnabled(false);
    comboBoxRunnerType->insertItems(0, QStringList()
        << "Umu-launcher"
        << "Wine"
        << "Lutris"
        << "Link and launch Lutris"
        << "Link and launch Steam"
        << "Link and exit"
        << "Just link"
        << "Bash"
        );

    layout->addWidget(comboBoxRunnerType);
}

WidgetLevelID::WidgetLevelID(QWidget* parent)
    : QWidget(parent),
    labelLevelID(new QLabel(this)),
    lcdNumberLevelID(new QLCDNumber(this)),
    layout(new QHBoxLayout(this))
{
    layout->setContentsMargins(6, 6, 6, 6);
    layout->setSpacing(8);

    layout->addWidget(labelLevelID);
    layout->addWidget(lcdNumberLevelID);
}

LevelControl::LevelControl(QWidget *parent)
    : QWidget(parent),
    commandLinkButtonLSReset(new QCommandLinkButton(this)),
    commandLinkButtonLSSave(new QCommandLinkButton(this)),
    layout(new QHBoxLayout(this))
{
    layout->setContentsMargins(6, 6, 6, 6);
    layout->setSpacing(8);

    layout->addWidget(commandLinkButtonLSReset);
    layout->addWidget(commandLinkButtonLSSave);
}

GlobalControl::GlobalControl(QWidget *parent)
    : QWidget(parent),
    commandLinkButtonGSReset(new QCommandLinkButton(this)),
    commandLinkButtonGSSave(new QCommandLinkButton(this)),
    layout(new QHBoxLayout(this))
{
    layout->setContentsMargins(6, 6, 6, 6);
    layout->setSpacing(8);

    layout->addWidget(commandLinkButtonGSReset);
    layout->addWidget(commandLinkButtonGSSave);
}
