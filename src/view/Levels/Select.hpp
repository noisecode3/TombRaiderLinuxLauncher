#ifndef VIEW_LEVELS_SELECT_HPP_
#define VIEW_LEVELS_SELECT_HPP_
#include <QWidget>
#include <QVBoxLayout>
#include "view/Levels/Filter.hpp"
#include "view/Levels/LevelViewList.hpp"
#include "view/Levels/StackedWidgetBar.hpp"

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

#endif // VIEW_LEVELS_SELECT_HPP_
