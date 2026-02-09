#ifndef SETUP_HPP_
#define SETUP_HPP_

#include "../src/settings.hpp"
#include "../src/Controller.hpp"
#include "view/UiState.hpp"
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QStackedWidget>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QComboBox>
#include <QCheckBox>
#include <QLCDNumber>
#include <QTableWidget>
#include <QCommandLinkButton>
#include <QtSvg/QSvgRenderer>


class LevelPathContainer : public QWidget
{
    Q_OBJECT
public:
    /*
     * (ui->tabs->setup->stackedWidget)
     * LevelPathContainer
     * ├── extraGamePathEdit
     * └── extraGamePathLabel
     */
    explicit LevelPathContainer(QWidget *parent);
    QLineEdit *levelPathEdit{nullptr};
    QLabel *levelPathLabel{nullptr};
private:
    QHBoxLayout *layout{nullptr};
};

class ExtraGamePathContainer : public QWidget
{
    Q_OBJECT
public:
    /*
     * (ui->tabs->setup->stackedWidget)
     * ExtraGamePathContainer
     * ├── extraGamePathEdit
     * └── extraGamePathLabel
     */
    explicit ExtraGamePathContainer(QWidget* parent);
    QLineEdit *extraGamePathEdit{nullptr};
    QLabel *extraGamePathLabel{nullptr};
private:
    QHBoxLayout *layout{nullptr};
};

class GamePathContainer : public QWidget
{
    Q_OBJECT
public:
    /*
     * (ui->tabs->setup->stackedWidget)
     * GamePathContainer
     * ├── gamePathEdit
     * └── gamePathLabel
     */
    explicit GamePathContainer(QWidget *parent);
    QLineEdit *gamePathEdit{nullptr};
    QLabel *gamePathLabel{nullptr};
private:
    QHBoxLayout *layout{nullptr};
};

class SetupInput : public QWidget
{
    Q_OBJECT
public:
    /*
     * (ui->tabs->setup->stackedWidget->fistTime)
     * SetupInput
     * ├── labelSetupDirectoryInfo
     * ├── labelGameDirectoryInfo
     * ├── gamePathContainer ->
     * ├── labelExtraGameDirectoryInfo
     * ├── extraGamePathContainer ->
     * ├── labelLevelDirectoryInfo
     * ├── levelPathContainer ->
     * └── setOptions
     */
    explicit SetupInput(QWidget *parent);
    QLabel *labelSetupDirectoryInfo{nullptr};
    QLabel *labelGameDirectoryInfo{nullptr};
    GamePathContainer *gamePathContainer{nullptr};
    QLabel *labelExtraGameDirectoryInfo{nullptr};
    ExtraGamePathContainer *extraGamePathContainer{nullptr};
    QLabel *labelLevelDirectoryInfo{nullptr};
    LevelPathContainer *levelPathContainer{nullptr};
    QPushButton *setOptions{nullptr};
private:
    QVBoxLayout *layout{nullptr};
};

class SetupImage : public QWidget
{
    Q_OBJECT
public:
    /*
     * (ui->tabs->setup->stackedWidget->fistTime)
     * SetupImage
     * └── lara
     */
    explicit SetupImage(QWidget *parent);
    QLabel *lara{nullptr};
private:
    QVBoxLayout *layout{nullptr};
};

class FirstTime : public QWidget
{
    Q_OBJECT
public:
    /*
     * (ui->tabs->setup->stackedWidget)
     * FirstTime
     * ├── setupImage ->
     * └── setupInput ->
     */
    explicit FirstTime(QWidget *parent);
    SetupImage *setupImage{nullptr};
    SetupInput *setupInput{nullptr};
private:
    QHBoxLayout *layout{nullptr};
};

class LevelControl : public QWidget
{
    Q_OBJECT
public:
    /*
     * (ui->tabs->setup->stackedWidget->settings->frameLevelSetup)
     * LevelControl
     * ├── commandLinkButtonLSReset
     * └── commandLinkButtonLSSave
     */
    explicit LevelControl(QWidget *parent);
    QCommandLinkButton *commandLinkButtonLSReset{nullptr};
    QCommandLinkButton *commandLinkButtonLSSave{nullptr};
private:
    QIcon themeIcon(QString iconPath);
    QHBoxLayout *layout{nullptr};
};

class WidgetEnvironmentVariables : public QWidget
{
    Q_OBJECT
public:
    /*
     * (ui->tabs->setup->stackedWidget->settings->
     * 		frameLevelSetup->frameLevelSetupSettings)
     * LevelControl
     * ├── labelEnvironmentVariables
     * └── lineEditEnvironmentVariables
     */
    explicit WidgetEnvironmentVariables(QWidget *parent);
    QLabel *labelEnvironmentVariables{nullptr};
    QLineEdit *lineEditEnvironmentVariables{nullptr};
private:
    QHBoxLayout *layout{nullptr};
};

class WidgetLevelID : public QWidget
{
    Q_OBJECT
public:
    /*
     * (ui->tabs->setup->stackedWidget->settings->
     * 		frameLevelSetup->frameLevelSetupSettings)
     * LevelControl
     * ├── labelLevelID
     * └── icdNumberLevelID
     */
    explicit WidgetLevelID(QWidget *parent);
    QLabel *labelLevelID{nullptr};
    QLCDNumber *lcdNumberLevelID{nullptr};
private:
    QHBoxLayout *layout{nullptr};
};

class WidgetRunnerType : public QWidget
{
    Q_OBJECT
public:
    /*
     * (ui->tabs->setup->stackedWidget->settings->
     * 		frameLevelSetup->frameLevelSetupSettings)
     * LevelControl
     * ├── labelRunnerType
     * ├── comboBoxRunnerType
     * ├── labelWinePath
     * └── lineEditWinePath
     */
    explicit WidgetRunnerType(QWidget* parent);
    QLabel *labelRunnerType{nullptr};
    QComboBox *comboBoxRunnerType{nullptr};
    QLabel *labelWinePath{nullptr};
    QLineEdit *lineEditWinePath{nullptr};
private:
    QHBoxLayout *layout{nullptr};
    UiState& g_uistate = UiState::getInstance();
};

class FrameLevelSetupSettings : public QFrame
{
    Q_OBJECT
public:
    /*
     * (ui->tabs->setup->stackedWidget->settings->frameLevelSetup)
     * FrameLevelSetupSettings
     * ├── headerLabel
     * ├── widgetEnvironmentVariables ->
     * ├── widgetRunnerType ->
     * ├── widgetLevelID ->
     * └── infoLabel
     */
    explicit FrameLevelSetupSettings(QWidget *parent);
    QLabel *headerLabel{nullptr};
    WidgetEnvironmentVariables *widgetEnvironmentVariables{nullptr};
    WidgetRunnerType *widgetRunnerType{nullptr};
    WidgetLevelID *widgetLevelID{nullptr};
    QLabel *infoLabel{nullptr};
private:
    QVBoxLayout *layout{nullptr};
};

class FrameLevelSetup : public QFrame
{
    Q_OBJECT
public:
    /*
     * (ui->tabs->setup->stackedWidget->settings)
     * FrameLevelSetup
     * ├── frameLevelSetupSettings ->
     * └── levelControl ->
     */
    explicit FrameLevelSetup(QWidget *parent);
    FrameLevelSetupSettings *frameLevelSetupSettings{nullptr};
    LevelControl *levelControl{nullptr};
private:
    QVBoxLayout *layout{nullptr};
};

class GlobalControl : public QWidget
{
    Q_OBJECT
public:
    /*
     * (ui->tabs->setup->stackedWidget->settings->frameGlobalSetup)
     * GlobalControl
     * ├── commandLinkButtonGSReset
     * └── commandLinkButtonGSSave
     */
    explicit GlobalControl(QWidget *parent);
    QCommandLinkButton *commandLinkButtonGSReset{nullptr};
    QCommandLinkButton *commandLinkButtonGSSave{nullptr};
    QIcon themeIcon(QString iconPath);
private:
    QHBoxLayout *layout{nullptr};
};

class WidgetDefaultEnvironmentVariables : public QWidget
{
    Q_OBJECT
public:
    /*
     * (ui->tabs->setup->stackedWidget->settings->frameGlobalSetup)
     * WidgetDefaultEnvironmentVariables
     * ├── LabelDefaultEnvironmentVariables
     * └── LineEditDefaultEnvironmentVariables
     */
    explicit WidgetDefaultEnvironmentVariables(QWidget *parent);
    QLabel *labelDefaultEnvironmentVariables{nullptr};
    QLineEdit *lineEditDefaultEnvironmentVariables{nullptr};
private:
    QHBoxLayout *layout{nullptr};
};

class WidgetDefaultRunnerType : public QWidget
{
    Q_OBJECT
public:
    /*
     * (ui->tabs->setup->stackedWidget->settings->frameGlobalSetup)
     * WidgetDefaultRunnerType
     * ├── labelDefaultRunnerType
     * └── comboBoxDefaultRunnerType
     */
    explicit WidgetDefaultRunnerType(QWidget *parent);
    QLabel *labelDefaultRunnerType{nullptr};
    QComboBox *comboBoxDefaultRunnerType{nullptr};
private:
    QHBoxLayout *layout{nullptr};
};

class WidgetDeleteZip : public QWidget
{
    Q_OBJECT
public:
    /*
     * (ui->tabs->setup->stackedWidget->settings->frameGlobalSetup)
     * WidgetDeleteZip
     * └── checkBoxDeleteZip
     */
    explicit WidgetDeleteZip(QWidget *parent);
    QCheckBox *checkBoxDeleteZip{nullptr};
private:
    QHBoxLayout *layout{nullptr};
};

class FrameGlobalSetup : public QFrame
{
    Q_OBJECT
public:
    /*
     * (ui->tabs->setup->stackedWidget->settings)
     * FrameGlobalSetup
     * ├── headerLabel
     * ├── tableWidgetGlobalSetup
     * ├── widgetDefaultEnvironmentVariables ->
     * ├── widgetDefaultRunnerType ->
     * ├── widgetDeleteZip ->
     * ├── labelGlobalSetupPicture
     * └── globalControl ->
     */
    explicit FrameGlobalSetup(QWidget *parent);
    QLabel *headerLabel{nullptr};
    QTableWidget *tableWidgetGlobalSetup{nullptr};
    WidgetDefaultEnvironmentVariables *widgetDefaultEnvironmentVariables{nullptr};
    WidgetDefaultRunnerType *widgetDefaultRunnerType{nullptr};
    WidgetDeleteZip *widgetDeleteZip{nullptr};
    QLabel *labelGlobalSetupPicture{nullptr};
    GlobalControl *globalControl{nullptr};
private:
    QVBoxLayout *layout{nullptr};
};

class Settings : public QFrame
{
    Q_OBJECT
public:
    /*
     * (ui->tabs->setup->stackedWidget->settings)
     * Settings
     * ├── FrameGlobalSetup ->
     * └── FrameLevelSetup ->
     */
    explicit Settings(QWidget *parent);
    FrameGlobalSetup *frameGlobalSetup{nullptr};
    FrameLevelSetup *frameLevelSetup{nullptr};
private:
    QHBoxLayout *layout{nullptr};
};

class UiSetup : public QWidget
{
    Q_OBJECT
public:
    /*
     * (Ui->Tabs)
     * Setup
     * └── stackedWidget
     *      ├── firstTime ->
     *      └── settings ->
     */
    explicit UiSetup(QWidget *parent);
    QStackedWidget *stackedWidget{nullptr};
    FirstTime *firstTime{nullptr};
    Settings *settings{nullptr};

    QString getRunnerTypeState();
    void setState(qint64 id);
    void downloadClicked(qint64 id);
    void readSavedSettings();
    void setOptionsClicked();

public slots:
    void globalSaveClicked();
    void globalResetClicked();
    void levelSaveClicked(qint64 id);
    void levelResetClicked(qint64 id);

private:
    QVBoxLayout *layout{nullptr};
    QSettings& g_settings = getSettingsInstance();
    Controller& controller = Controller::getInstance();
};

#endif // SETUP_HPP_
