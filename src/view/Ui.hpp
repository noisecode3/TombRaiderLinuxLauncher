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
private:
    Controller& controller = Controller::getInstance();
    QSettings& g_settings = getSettingsInstance();
    void LevelResetClicked();
    void LevelSaveClicked();
    void setOptionsClicked();
    void onCurrentItemChanged(const QModelIndex &current);
    void startUpSetup();
    void downloadOrRemoveClicked();
    QGridLayout *layout{nullptr};
};

#endif // UI_HPP_
