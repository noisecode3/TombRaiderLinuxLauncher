#ifndef VIEW_LEVELS_STACKEDWIDGET_HPP_
#define VIEW_LEVELS_STACKEDWIDGET_HPP_

#include "view/UiState.hpp"
#include <QWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QCheckBox>
#include <QPushButton>
#include <QProgressBar>
#include <QStackedWidget>
#include <QListWidget>
#include <QWebEngineView>

class NavigateWidgetBar : public QWidget
{
    Q_OBJECT
public:
    /*
     * (ui->tabs->levels->stackedWidget->select->stackedWidgetBar)
     * NavigateWidgetBar
     * ├── checkBoxSetup
     * ├── pushButtonFilter
     * ├── pushButtonDownload
     * ├── pushButtonInfo
     * └── pushButtonRun
     */
    explicit NavigateWidgetBar(QWidget *parent);
    QCheckBox *checkBoxSetup{nullptr};
    QPushButton *pushButtonFilter{nullptr};
    QPushButton *pushButtonDownload{nullptr};
    QPushButton *pushButtonInfo{nullptr};
    QPushButton *pushButtonRun{nullptr};
private:
    UiState& g_uistate = UiState::getInstance();
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
    enum index {
        Navigate,
        Progress
    };
    void setCurrentWidgetIndex(const index widget);
};

#endif // VIEW_LEVELS_STACKEDWIDGET_HPP_
