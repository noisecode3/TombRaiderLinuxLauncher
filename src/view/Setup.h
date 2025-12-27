#ifndef SETUP_H
#define SETUP_H

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
     * ├── comboBoxRunnerType
     * └── labelRunnerType
     */
    explicit WidgetRunnerType(QWidget* parent);
    QComboBox *comboBoxRunnerType{nullptr};
    QLabel *labelRunnerType{nullptr};
private:
    QHBoxLayout *layout{nullptr};
};

class FrameLevelSetupSettings : public QFrame
{
    Q_OBJECT
public:
    /*
     * (ui->tabs->setup->stackedWidget->settings->frameLevelSetup)
     * FrameLevelSetupSettings
     * ├── widgetEnvironmentVariables ->
     * ├── widgetRunnerType ->
     * ├── widgetLevelID ->
     * └── label
     */
    explicit FrameLevelSetupSettings(QWidget *parent);
    WidgetEnvironmentVariables *widgetEnvironmentVariables{nullptr};
    WidgetRunnerType *widgetRunnerType{nullptr};
    WidgetLevelID *widgetLevelID{nullptr};
    QLabel *label{nullptr};
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
     * ├── labelLevelSetup
     * ├── frameLevelSetupSettings ->
     * └── levelControl ->
     */
    explicit FrameLevelSetup(QWidget *parent);
    QLabel *labelLevelSetup{nullptr};
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
     * ├── labelGlobalSetup
     * ├── tableWidgetGlobalSetup
     * ├── widgetDefaultEnvironmentVariables ->
     * ├── widgetDefaultRunnerType ->
     * ├── widgetDeleteZip ->
     * ├── labelGlobalSetupPicture
     * └── globalControl ->
     */
    explicit FrameGlobalSetup(QWidget *parent);
    QLabel *labelGlobalSetup{nullptr};
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
private:
    QVBoxLayout *layout{nullptr};
};

#endif // SETUP_H
