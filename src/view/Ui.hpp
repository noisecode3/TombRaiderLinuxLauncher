#ifndef UI_HPP_
#define UI_HPP_

#include "../view/About.hpp"
#include "../view/Controller.hpp"
#include "../view/Levels.hpp"
#include "../view/Modding.hpp"
#include "../view/Setup.hpp"
#include <QWidget>
#include <QTabWidget>
#include <QGridLayout>

class Ui : public QWidget
{
    Q_OBJECT
public:
    /*
     * Ui (Centeral Widget)
     * └── Tabs
     *      ├── About ->
     *      ├── UiController ->
     *      ├── Levels ->
     *      ├── Modding ->
     *      └── Setup ->
     */
    explicit Ui(QWidget *parent = nullptr);
    QTabWidget* tabs{nullptr};
    UiAbout* about{nullptr};
    UiController* uicontroller{nullptr};
    UiLevels* levels{nullptr};
    UiModding* modding{nullptr};
    UiSetup* setup{nullptr};
public slots:
    /**
     * Updates selected menu level state.
     */
    void onCurrentItemChanged(const QModelIndex &current);

    /**
     * Set setup level UI settings to the previous saved state.
     */
    void levelResetClicked();

    /**
     * Set setup level saved state to the UI .
     */
    void levelSaveClicked();
private:
    Controller& controller = Controller::getInstance();
    QSettings& g_settings = getSettingsInstance();
    void setOptionsClicked();
    void startUpSetup();
    void downloadOrRemoveClicked();
    QGridLayout *layout{nullptr};
};

#endif // UI_HPP_
