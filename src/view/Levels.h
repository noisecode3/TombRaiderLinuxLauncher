#ifndef LEVELS_H
#define LEVELS_H

#include "LevelViewList.hpp"
#include <QWidget>
#include <QGridLayout>
#include <QStackedWidget>
#include <QCheckBox>
#include <QComboBox>
#include <QGroupBox>
#include <QLabel>
#include <QProgressBar>
#include <QPushButton>
#include <QRadioButton>
#include <QWebEngineView>

class UiDialog : public QWidget
{
    Q_OBJECT
public:
    /*
     * (ui->tabs->levels->stackedWidget)
     * UiDialog
     */
    explicit UiDialog(QWidget *parent);
private:
    QVBoxLayout* layout{nullptr};
};

class InfoBar : public QWidget
{
    Q_OBJECT
public:
    /*
     * (ui->tabs->levels->stackedWidget->info)
     * InfoBar
     * ├── pushButtonWalkthrough
     * └── pushButtonBack
     */
    explicit InfoBar(QWidget *parent);
    QPushButton *pushButtonWalkthrough{nullptr};
    QPushButton *pushButtonBack{nullptr};
private:
    QHBoxLayout *layout{nullptr};
};

class InfoContent : public QWidget
{
    Q_OBJECT
public:
    /*
     * (ui->tabs->levels->stackedWidget->info)
     * InfoContent
     * ├── coverListWidget
     * └── infoWebEngineView
     */
    explicit InfoContent(QWidget *parent);
    QListWidget *coverListWidget{nullptr};
    QWebEngineView *infoWebEngineView{nullptr};
private:
    QHBoxLayout *layout{nullptr};
};

class Info : public QWidget
{
    Q_OBJECT
public:
    /*
     * (ui->tabs->levels->stackedWidget)
     * Info
     * ├── infoContent ->
     * └── infoBar ->
     */
    explicit Info(QWidget *parent);
    InfoContent *infoContent{nullptr};
    InfoBar *infoBar{nullptr};
private:
    QVBoxLayout *layout{nullptr};
};

class Loading : public QWidget
{
    Q_OBJECT
public:
    /**
     * (ui->tabs->levels->stackedWidget)
     * Loading
     */
    explicit Loading(QWidget *parent);
private:
    QVBoxLayout *layout{nullptr};
};

class FilterGroupBoxFilter : public QGroupBox
{
    Q_OBJECT
public:
    /*
     * (ui->tabs->levels->stackedWidget->select->filter->filterFirstInputRow)
     * FilterGroupBoxFilter
     * ├── labelClass
     * ├── comboBoxClass
     * ├── labelDifficulty
     * ├── comboBoxDifficulty
     * ├── labelDuration
     * ├── comboBoxDuration
     * ├── labelType
     * └── comboBoxType
     */
    explicit FilterGroupBoxFilter(QWidget *parent);
    QLabel *labelClass{nullptr};
    QComboBox *comboBoxClass{nullptr};
    QLabel *labelDifficulty{nullptr};
    QComboBox *comboBoxDifficulty{nullptr};
    QLabel *labelDuration{nullptr};
    QComboBox *comboBoxDuration{nullptr};
    QLabel *labelType{nullptr};
    QComboBox *comboBoxType{nullptr};
private:
    QHBoxLayout *layout{nullptr};
};

class FilterGroupBoxSearch : public QGroupBox
{
    Q_OBJECT
public:
    /*
     * (ui->tabs->levels->stackedWidget->select->filter->filterFirstInputRow)
     * FilterGroupBoxSearch
     * ├── comboBoxSearch
     * └── lineEditSearch
     */
    explicit FilterGroupBoxSearch(QWidget *parent);
    QComboBox *comboBoxSearch{nullptr};
    QLineEdit *lineEditSearch{nullptr};
private:
    QHBoxLayout *layout{nullptr};
};

class FilterGroupBoxSort : public QGroupBox
{
    Q_OBJECT
public:
    /*
     * (ui->tabs->levels->stackedWidget->select->filter->filterSecondInputRow)
     * FilterGroupBoxSort
     * ├── radioButtonClass
     * ├── radioButtonDifficulty
     * ├── radioButtonDuration
     * ├── radioButtonLevelName
     * ├── radioButtonReleaseDate
     * └── radioButtonType
     */
    explicit FilterGroupBoxSort(QWidget *parent);
    QRadioButton *radioButtonClass{nullptr};
    QRadioButton *radioButtonDifficulty{nullptr};
    QRadioButton *radioButtonDuration{nullptr};
    QRadioButton *radioButtonLevelName{nullptr};
    QRadioButton *radioButtonReleaseDate{nullptr};
    QRadioButton *radioButtonType{nullptr};
private:
    QHBoxLayout *layout{nullptr};
};

class FilterGroupBoxToggle : public QGroupBox
{
    Q_OBJECT
public:
    /*
     * (ui->tabs->levels->stackedWidget->select->filter-filterSecondInputRow)
     * FilterGroupBoxToggle
     * ├── checkBoxInstalled
     * └── checkBoxOriginal
     */
    explicit FilterGroupBoxToggle(QWidget *parent);
    QCheckBox *checkBoxInstalled{nullptr};
    QCheckBox *checkBoxOriginal{nullptr};
private:
    QHBoxLayout *layout{nullptr};
};

class FilterFirstInputRow : public QWidget
{
    Q_OBJECT
public:
    /*
     * (ui->tabs->levels->stackedWidget->select->filter)
     * FilterFirstInputRow
     * ├── filterGroupBoxFilter ->
     * └── filterGroupBoxSearch ->
     */
    explicit FilterFirstInputRow(QWidget* parent);
    FilterGroupBoxFilter *filterGroupBoxFilter;
    FilterGroupBoxSearch *filterGroupBoxSearch;
private:
    QHBoxLayout *layout{nullptr};
};

class FilterSecondInputRow : public QWidget
{
    Q_OBJECT
public:
    /*
     * (ui->tabs->levels->stackedWidget->select->filter)
     * FilterSecondInputRow
     * ├── filterGroupBoxSort ->
     * └── filterGroupBoxToggle ->
     */
    explicit FilterSecondInputRow(QWidget *parent);
    FilterGroupBoxSort *filterGroupBoxSort;
    FilterGroupBoxToggle *filterGroupBoxToggle;
private:
    QHBoxLayout *layout{nullptr};
};

class Filter : public QWidget
{
    Q_OBJECT
public:
    /*
     * (ui->tabs->levels->stackedWidget->select)
     * Filter
     * ├── filterFirstInputRow ->
     * └── filterSecondInputRow ->
     */
    explicit Filter(QWidget *parent);
    FilterFirstInputRow *filterFirstInputRow{nullptr};
    FilterSecondInputRow *filterSecondInputRow{nullptr};
private:
    QVBoxLayout *layout{nullptr};
};

class NavigateWidgetBar : public QWidget
{
    Q_OBJECT
public:
    /*
     * (ui->tabs->levels->stackedWidget->select->stackedWidgetBar)
     * NavigateWidgetBar
     * ├── checkBoxSetup
     * ├── pushButtonFiler
     * ├── pushButtonDownload
     * ├── pushButtonInfo
     * └── pushButtonRun
     */
    explicit NavigateWidgetBar(QWidget *parent);
    QCheckBox *checkBoxSetup{nullptr};
    QPushButton *pushButtonFiler{nullptr};
    QPushButton *pushButtonDownload{nullptr};
    QPushButton *pushButtonInfo{nullptr};
    QPushButton *pushButtonRun{nullptr};
private:
    QHBoxLayout *layout{nullptr};
};

class ProgressWidgetBar : public QWidget
{
    Q_OBJECT
public:
    /*
     * (ui->tabs->levels->stackedWidget->select->StackedWidgetBar)
     * ProgressWidgetBar
     * └── progressWidgetBar ->
     */
    explicit ProgressWidgetBar(QWidget *parent);
    QProgressBar *progressBar{nullptr};
private:
    QHBoxLayout *layout{nullptr};
};

class StackedWidgetBar : public QStackedWidget
{
    Q_OBJECT
public:
    /*
     * (ui->tabs->levels->stackedWidget->select)
     * StackedWidgetBar
     * ├── navigateWidgetBar ->
     * └── progressWidgetBar ->
     */
    explicit StackedWidgetBar(QWidget *parent);
    NavigateWidgetBar *navigateWidgetBar{nullptr};
    ProgressWidgetBar *progressWidgetBar{nullptr};
};

class Select : public QWidget
{
    Q_OBJECT
public:
    /*
     * (ui->tabs->levels->stackedWidget)
     * Select
     * ├── filter ->
     * ├── levelViewList
     * └── stackedWidgetBar ->
     */
    explicit Select(QWidget *parent);
    Filter *filter{nullptr};
    LevelViewList *levelViewList{nullptr};
    StackedWidgetBar *stackedWidgetBar{nullptr};
private:
    QVBoxLayout *layout{nullptr};
};

class WalkthroughBar : public QWidget
{
    Q_OBJECT
public:
    /*
     * (ui->tabs->levels->stackedWidget->walkthrough)
     * WalkthroughBar
     * └── walkthroughBackButton
     */
    explicit WalkthroughBar(QWidget *parent);
    QPushButton *walkthroughBackButton{nullptr};
private:
    QHBoxLayout *layout{nullptr};
};

class Walkthrough : public QWidget
{
    Q_OBJECT
public:
    /*
     * (ui->tabs->levels->stackedWidget)
     * Walkthrough
     * ├── walkthroughWebEngineView
     * └── walkthroughBar ->
     */
    explicit Walkthrough(QWidget *parent);
    QWebEngineView *walkthroughWebEngineView{nullptr};
    WalkthroughBar *walkthroughBar{nullptr};
private:
    QVBoxLayout *layout{nullptr};
};

class UiLevels : public QWidget
{
    Q_OBJECT
public:
    /*
     * (ui->tabs)
     * Levels
     * └── stackedWidget
     *      ├── UiDialog ->
     *      ├── Info ->
     *      ├── Loading ->
     *      ├── Select ->
     *      └── Walkthrought ->
     */
    explicit UiLevels(QWidget *parent);
    QStackedWidget *stackedWidget{nullptr};
    UiDialog *dialog{nullptr};
    Info *info{nullptr};
    Loading *loading{nullptr};
    Select *select{nullptr};
    Walkthrough *walkthough{nullptr};
private:
    QGridLayout *layout{nullptr};
};

#endif // LEVELS_H
