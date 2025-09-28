/* TombRaiderLinuxLauncher
 * Martin Bångens Copyright (C) 2025
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef SRC_LEVELVIEWLIST_HPP_
#define SRC_LEVELVIEWLIST_HPP_

#include <QListWidgetItem>
#include <QStyledItemDelegate>
#include <QSortFilterProxyModel>
#include <QAbstractItemModel>
#include <qobject.h>

#include "../src/Data.hpp"

class LevelViewList : public QListView {
    Q_OBJECT

public:
    explicit LevelViewList(QWidget *parent = nullptr);

    QModelIndexList visibleIndexes() const;

protected:
    void scrollContentsBy(int dx, int dy) override;
    void resizeEvent(QResizeEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

private:
    void updateVisibleItems();
};


class LevelListModel : public QAbstractListModel {
    Q_OBJECT

 public:
    explicit LevelListModel(QObject *parent = nullptr)
        : QAbstractListModel(parent),
        m_scrollCursorChanged(false),
        m_cursor_a(0),
        m_cursor_b(0),
        m_roleTable({
            { Qt::DisplayRole,  [](const ListItemData &i){ return i.m_title; }},
            { Qt::UserRole+1,   [](const ListItemData &i){ return i.m_game_id; }},
            { Qt::UserRole+2,   [](const ListItemData &i){ return i.m_type; }},
            { Qt::UserRole+3,   [](const ListItemData &i){ return i.m_releaseDate; }},
            { Qt::UserRole+4,   [](const ListItemData &i){ return i.m_cover; }},
            { Qt::UserRole+5,   [](const ListItemData &i){ return i.m_shortBody; }},
            { Qt::UserRole+6,   [](const ListItemData &i){ return i.m_authors; }},
            { Qt::UserRole+7,   [](const ListItemData &i){ return i.m_class; }},
            { Qt::UserRole+8,   [](const ListItemData &i){ return i.m_difficulty; }},
            { Qt::UserRole+9,   [](const ListItemData &i){ return i.m_duration; }},
            { Qt::UserRole+10,  [](const ListItemData &i){ return i.m_trle_id; }},
            { Qt::UserRole+11,  [](const ListItemData &i){ return i.m_installed; }},
        })
    {}

    QVector<QSharedPointer<ListItemData>> getChunk(const quint64 cursor,
                                                    const quint64 items);
    QVector<QSharedPointer<ListItemData>> getDataBuffer(const quint64 items);
    void setLevels(const QVector<QSharedPointer<ListItemData>>& levels);
    void setScrollChange();
    void addScrollItem(const quint64 index);
    void setInstalled(const QModelIndex &index);
    quint64 indexInBounds(quint64 index) const;
    bool stop() const;
    void updateCovers(quint64 a, quint64 b);
    void reset();
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;

 private:
    const QHash<int, std::function<QVariant(const ListItemData&)>> m_roleTable;
    QVector<QSharedPointer<ListItemData>> m_levels;
    bool m_scrollCursorChanged;
    quint64 m_cursor_a;
    quint64 m_cursor_b;
};


class LevelListProxy : public QSortFilterProxyModel {
    Q_OBJECT

 public:
    explicit LevelListProxy(QObject *parent = nullptr)
        : QSortFilterProxyModel(parent),
        m_class(0),
        m_type(0),
        m_difficulty(0),
        m_duration(0),
        m_all(" - All -"),
        m_installed(false),
        m_sortMode(ReleaseDate)
    {}

    quint64 getLid(const QModelIndex &i) const;
    bool getItemType(const QModelIndex &i) const;
    bool getInstalled(const QModelIndex &i) const;

    void setClassFilter(const QString &c);
    void setTypeFilter(const QString &t);
    void setDifficultyFilter(const QString &d);
    void setDurationFilter(const QString &d);
    void setSearchFilter(const QString &s);
    void setInstalledFilter(bool on);

    enum SortMode {
        Title,
        Difficulty,
        Duration,
        Class,
        Type,
        ReleaseDate
    };

    void setSortMode(SortMode mode);

 protected:
    bool lessThan(const QModelIndex &left,
                    const QModelIndex &right) const override;
    bool filterAcceptsRow(int sourceRow,
            const QModelIndex &parent) const override;

 private:
    int roleForMode() const;
    static constexpr int m_roleForModeTable[6] = {
        Qt::DisplayRole,
        Qt::UserRole+8,
        Qt::UserRole+9,
        Qt::UserRole+7,
        Qt::UserRole+2,
        Qt::UserRole+3
    };
    SortMode m_sortMode = ReleaseDate;
    quint64 m_class, m_type, m_difficulty, m_duration;
    const QString m_all;
    QString m_search;
    bool m_installed;
};


class CardItemDelegate : public QStyledItemDelegate {
 public:
    using QStyledItemDelegate::QStyledItemDelegate;

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const override;

    QSize sizeHint(
            const QStyleOptionViewItem&, const QModelIndex&) const override;
};

#endif  // SRC_LEVELVIEWLIST_HPP_
