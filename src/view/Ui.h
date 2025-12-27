#ifndef UI_H
#define UI_H

#include "../view/About.h"
#include "../view/Controller.h"
#include "../view/Levels.h"
#include "../view/Modding.h"
#include "../view/Setup.h"
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
     *      ├── Controller ->
     *      ├── Levels ->
     *      ├── Modding ->
     *      └── Setup ->
     */
    explicit Ui(QWidget *parent = nullptr);
    QTabWidget* tabs{nullptr};
    UiAbout* about{nullptr};
    UiController* controller{nullptr};
    UiLevels* levels{nullptr};
    UiModding* modding{nullptr};
    UiSetup* setup{nullptr};
private:
    QGridLayout *layout{nullptr};
};

#endif // UI_H
