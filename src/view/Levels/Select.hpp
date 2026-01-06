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

    void setItemChanged(const QModelIndex &current);
    void setSortMode(LevelListProxy::SortMode mode);
    void setRemovedLevel();
    void setInstalledLevel();
    bool getType();
    quint64 getLid();
    void setLevels(QVector<QSharedPointer<ListItemData>> &list);
    bool stop();
    void  reset();
    QVector<QSharedPointer<ListItemData>> getDataBuffer(quint64 lenght);

private:
    QModelIndex m_current;
    LevelListModel *levelListModel;
    LevelListProxy *levelListProxy;
    QVBoxLayout *layout{nullptr};
};

#endif // VIEW_LEVELS_SELECT_HPP_
