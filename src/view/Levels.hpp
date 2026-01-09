#ifndef LEVELS_HPP_
#define LEVELS_HPP_

#include "../src/Controller.hpp"
#include "../src/CommandLineParser.hpp"
#include "view/Levels/Info.hpp"
#include "view/Levels/LoadingIndicator.hpp"
#include "view/Levels/Dialog.hpp"
#include "view/Levels/Select.hpp"
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
#include <QListWidget>


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

class UiLevels : public QWidget
{
    Q_OBJECT
public:
    /*
     * (ui->tabs)
     * Levels
     * └── stackedWidget
     *      ├── Dialog ->
     *      ├── Info ->
     *      ├── Loading ->
     *      └── Select ->
     */
    explicit UiLevels(QWidget *parent);
    QStackedWidget *stackedWidget{nullptr};
    Dialog *dialog{nullptr};
    Info *info{nullptr};
    Loading *loading{nullptr};
    LoadingIndicator* m_loadingIndicatorWidget;
    Select *select{nullptr};

    void setSortMode(LevelListProxy::SortMode mode);
    void setItemChanged(const QModelIndex &current);
    qint64 getItemId();
    void setState(const QString &text);
    void setStartupSetting(const StartupSetting startupSetting);
    void setpushButtonRunText(const QString &text);
    void removeClicked(qint64 id);
    void downloadClicked(qint64 id);
    void setupGameOrLevel(qint64 id);

public slots:
   /**
     * Opens the Info thru the navigation bar.
     */
    void infoClicked();

    /**
     * Returns to the first navigation state, the list.
     */
    void backClicked();

    /**
     * Generates the initial level list after file analysis.
     */
    void generateList(const QList<int>& availableGames);

    /**
     * Switch back to level list or info page after network/python work.
     */
    void updateLevelDone();

    /**
     *  Try loading 100 more levels cards by calling for more cover pictures.
     */
    void loadMoreCovers();

    /**
     * Updates progress by 1% of total work steps.
     */
    void workTick();

    /**
     * Switch back to list selection state after running the game.
     */
    void runningLevelDone();
private:
    QSettings& g_settings = getSettingsInstance();
    QString m_loadingDoneGoTo;
    QList<int> m_availableGames;
    Controller& controller = Controller::getInstance();

    struct InstalledStatus {
        QHash<quint64, bool> game;
        QHash<quint64, bool> trle;
    };

    void runClicked();
    void walkthroughClicked();
    QStringList parsToArg(const QString& str);
    QVector<QPair<QString, QString>> parsToEnv(const QString& str);
    void downloadOrRemoveClicked();
    InstalledStatus getInstalled();
    void setList();
    void levelDirSelected(qint64 id);
    void callbackDialog(QString selected);
    void setStackedWidget(const QString &qwidget);
    QGridLayout *layout{nullptr};
};

#endif // LEVELS_HPP_
