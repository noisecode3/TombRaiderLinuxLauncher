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

#include "../src/Data.hpp"


class LevelListModel : public QAbstractListModel {
    Q_OBJECT

 public:
    explicit LevelListModel(QObject *parent = nullptr)
        : QAbstractListModel(parent) {}

    void setLevels(const QVector<ListItemData>& levels);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role) const override;

 private:
    QVector<ListItemData> m_levels;
};


class LevelListProxy : public QSortFilterProxyModel {
    Q_OBJECT

 public:
    explicit LevelListProxy(QObject *parent = nullptr)
        : QSortFilterProxyModel(parent) {}

    quint64 getLid(const QModelIndex &i);

    bool getItemType(const QModelIndex &i);

    bool getInstalled(const QModelIndex &i);

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

    SortMode m_sortMode = ReleaseDate;
    QString m_class, m_type, m_difficulty, m_duration, m_search;
    bool m_installed = false;
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
