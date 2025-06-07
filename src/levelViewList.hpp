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

#include <algorithm>

#include "../src/Controller.hpp"
#include "../src/staticData.hpp"

class LevelListModel : public QAbstractListModel {
    Q_OBJECT

 public:
    explicit LevelListModel(QObject *parent): QAbstractListModel(parent) {}

    void setLevels() {
        beginResetModel();
        m_mainList.clear();
        controller.getList(&m_mainList);
        for (ListItemData& item : m_mainList) {
            m_filterList.append(&item);
            m_sortList.append(&item);
        }
        endResetModel();
        m_a = createIndex(0, 0);
        m_b = createIndex(99, 0);
        getMoreCovers();
    }

    int rowCount(const QModelIndex &parent = QModelIndex()) const override {
        Q_UNUSED(parent);
        return m_filterList.count();
    }

    QVariant data(const QModelIndex &index, int role) const override {
        QVariant result;
        int row = index.row();

        if ((index.isValid()) && (row < m_filterList.count())) {
            const ListItemData* item = m_filterList.at(row);
            switch (role) {
                case Qt::DisplayRole:
                    result = item->m_title;
                    break;
                case Qt::UserRole + 1:
                    result = item->m_authors;
                    break;
                case Qt::UserRole + 2:
                    result = item->m_type;
                    break;
                case Qt::UserRole + 3:
                    result = item->m_class;
                    break;
                case Qt::UserRole + 4:
                    result = item->m_difficulty;
                    break;
                case Qt::UserRole + 5:
                    result = item->m_releaseDate;
                    break;
                case Qt::UserRole + 6:
                    result = item->m_duration;
                    break;
                case Qt::UserRole + 7:
                    result = item->m_cover;
                    break;
            }
        }

        return result;
    }

    int getLid(const QModelIndex &index) const {
        const ListItemData* item = m_filterList.at(index.row());
        return item->m_trle_id;
    }

    void getMoreCovers() {
        m_cache.clear();
        for (qint64 i = m_a.row(); i <= m_b.row(); i++) {
            m_cache.append(&m_mainList[i]);
        }
        controller.getCoverList(&m_cache);
    }

    void loadMoreCovers() {
        emit dataChanged(m_a, m_b, {Qt::DecorationRole});
        if (!m_covers_loaded) {
            m_a = createIndex(m_b.row() + 1, 0);
            qint64 plus100 = m_b.row() + 100;
            if (m_mainList.count() > plus100) {
                m_b = createIndex(plus100, 0);
            } else {
                m_b = createIndex(m_mainList.count() - 1, 0);
                m_covers_loaded = true;
            }
            getMoreCovers();
        }
    }

    void sortItems(
            std::function<bool(ListItemData*, ListItemData*)> compare) {
        beginResetModel();
        std::sort(m_sortList.begin(), m_sortList.end(), compare);
        endResetModel();
        reFilter();
    }

    static bool compareTitles(const ListItemData* a, const ListItemData* b) {
        return a->m_title.toLower() < b->m_title.toLower();
    }

    static bool compareDifficulties(
            const ListItemData* a, const ListItemData* b) {
        return a->m_difficulty > b->m_difficulty;
    }

    static bool compareDurations(const ListItemData* a, const ListItemData* b) {
        return a->m_duration > b->m_duration;
    }

    static bool compareClasses(const ListItemData* a, const ListItemData* b) {
        return a->m_class > b->m_class;
    }

    static bool compareTypes(const ListItemData* a, const ListItemData* b) {
        return a->m_type > b->m_type;
    }

    static bool compareReleaseDates(
            const ListItemData* a, const ListItemData* b) {
        // descending order
        return a->m_releaseDate > b->m_releaseDate;
    }

    void filterClass(const QString &class_) {
        beginResetModel();
        m_filter = QString("class:%1").arg(class_);
        m_filterList.clear();

        for (ListItemData* item : m_sortList) {
            if (StaticData().getClass()[item->m_class] == class_) {
                m_filterList.append(item);
            }
        }

        endResetModel();
    }

    void filterType(const QString &type) {
        beginResetModel();
        m_filter = QString("type:%1").arg(type);
        m_filterList.clear();

        for (ListItemData* item : m_sortList) {
            if (StaticData().getType()[item->m_type] == type) {
                m_filterList.append(item);
            }
        }

        endResetModel();
    }

    void filterDifficulty(const QString &difficulty) {
        beginResetModel();
        m_filter = QString("difficulty:%1").arg(difficulty);
        m_filterList.clear();

        for (ListItemData* item : m_sortList) {
            if (StaticData()
                    .getDifficulty()[item->m_difficulty] == difficulty) {
                m_filterList.append(item);
            }
        }

        endResetModel();
    }

    void filterDuration(const QString &duration) {
        beginResetModel();
        m_filter = QString("duration:%1").arg(duration);
        m_filterList.clear();

        for (ListItemData* item : m_sortList) {
            if (StaticData().getType()[item->m_duration] == duration) {
                m_filterList.append(item);
            }
        }

        endResetModel();
    }

    void reFilter() {
        if (m_filter.startsWith("class:")) {
            QString const class_ = m_filter.mid(6);
            filterClass(class_);
        } else if (m_filter.startsWith("type:")) {
            QString const type = m_filter.mid(5);
            filterType(type);
        } else if (m_filter.startsWith("difficulty:")) {
            QString const difficulty = m_filter.mid(11);
            filterDifficulty(difficulty);
        } else if (m_filter.startsWith("duration:")) {
            QString const duration = m_filter.mid(9);
            filterDuration(duration);
        } else {
            beginResetModel();
            m_filterList.clear();
            for (ListItemData* item : m_sortList) {
                m_filterList.append(item);
            }
            endResetModel();
        }
    }

 private:
    QVector<ListItemData> m_mainList;
    QVector<ListItemData*> m_sortList;
    QVector<ListItemData*> m_filterList;
    QString m_filter;
    QString m_search;
    QVector<ListItemData*> m_cache;
    bool m_covers_loaded = false;
    QModelIndex m_a;
    QModelIndex m_b;
    // QVector<ListOriginalData> originalInfoList;
    Controller& controller = Controller::getInstance();
};

class CardItemDelegate : public QStyledItemDelegate {
 public:
    using QStyledItemDelegate::QStyledItemDelegate;

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const override {
        painter->save();
        const bool selected = option.state & QStyle::State_Selected;

        // Card background
        painter->setRenderHint(QPainter::Antialiasing);
        QRectF cardRect = option.rect.adjusted(4, 4, -4, -4);
        QColor bgColor = selected ? QColor("#e0f7fa") : QColor("#ffffff");
        painter->setBrush(bgColor);
        painter->setPen(Qt::NoPen);
        painter->drawRoundedRect(cardRect, 10, 10);

        // Picture space (left side)
        qint64 x = cardRect.left() + 10;
        qint64 y = cardRect.top() + 10;
        QRect imageRect = QRect(x, y, 320, 240);
        painter->setBrush(QColor("#cccccc"));
        QPixmap cover = index.data(Qt::UserRole + 7).value<QPixmap>();
        if (!cover.isNull()) {
            painter->drawPixmap(imageRect, cover);
        } else {
            painter->drawRect(imageRect);
        }

        // Text section (right side of image)
        int textX = imageRect.right() + 10;
        int textY = cardRect.top() + 10;

        QString title = index.data(Qt::DisplayRole).toString();
        QStringList authorsList = index.data(Qt::UserRole + 1).toStringList();
        QString authors = authorsList.join(", ");
        if (authors.size() > 100) {
            authors = "Various";
        }

        qint64 typeId = index.data(Qt::UserRole + 2).toInt();
        QString type = StaticData().getType()[typeId];
        QString release = index.data(Qt::UserRole + 5).toString();

        QFont boldFont = option.font;
        boldFont.setBold(true);
        painter->setFont(boldFont);
        painter->setPen(Qt::black);
        painter->drawText(QPoint(textX, textY + 15), title);

        QFont normalFont = option.font;
        normalFont.setPointSizeF(option.font.pointSizeF() - 1);
        painter->setFont(normalFont);

        painter->drawText(QPoint(textX, textY + 35), "By: " + authors);
        painter->drawText(QPoint(textX, textY + 50), "Type: " + type);
        painter->drawText(QPoint(textX, textY + 65), "Released: " + release);
        painter->drawText(QPoint(textX, textY + 65), "Released: " + release);

        painter->restore();
    }

    QSize sizeHint(
            const QStyleOptionViewItem&, const QModelIndex&) const override {
        return QSize(600, 300);
    }
};

#endif  // SRC_LEVELVIEWLIST_HPP_
