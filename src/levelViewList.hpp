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

class LevelListModel : public QAbstractListModel {
    Q_OBJECT

 public:
    explicit LevelListModel(QObject *parent): QAbstractListModel(parent) {}

    void setLevels() {
        beginResetModel();
        infoList.clear();
        controller.getList(&infoList);
        for (ListItemData& item : infoList) {
            filterList.append(&item);
            list.append(&item);
        }
        endResetModel();
        a = createIndex(0, 0);
        b = createIndex(99, 0);
        getMoreCovers();
    }

    int rowCount(const QModelIndex &parent = QModelIndex()) const override {
        Q_UNUSED(parent);
        return filterList.count();
    }

    QVariant data(const QModelIndex &index, int role) const override {
        QVariant result;
        int row = index.row();

        if (index.isValid() && row < filterList.count()) {
            const ListItemData* item = filterList.at(row);
            switch (role) {
                case Qt::DisplayRole:
                    return item->m_title;
                case Qt::UserRole + 1:
                    return item->m_authors;
                case Qt::UserRole + 2:
                    return item->m_type;
                case Qt::UserRole + 3:
                    return item->m_class;
                case Qt::UserRole + 4:
                    return item->m_difficulty;
                case Qt::UserRole + 5:
                    return item->m_releaseDate;
                case Qt::UserRole + 6:
                    return item->m_duration;
                case Qt::UserRole + 7:
                    return item->m_cover;
                default:
                    return QVariant();
            }
        }

        return result;
    }

    int getLid(const QModelIndex &index) const {
        const ListItemData* item = filterList.at(index.row());
        return item->m_trle_id;
    }

    void getMoreCovers() {
        cache.clear();
        for (qint64 i = a.row(); i <= b.row(); i++) {
            cache.append(list[i]);
        }
        controller.getCoverList(&cache);
    }

    void loadMoreCovers() {
        qDebug() << a;
        qDebug() << b;
        emit dataChanged(a, b, {Qt::DecorationRole});
        if (!m_covers_loaded) {
            a = createIndex(b.row() + 1, 0);
            qint64 plus100 = b.row() + 100;
            if (list.count() > plus100) {
                b = createIndex(plus100, 0);
            } else {
                b = createIndex(list.count() - 1, 0);
                m_covers_loaded = true;
            }
            getMoreCovers();
        }
    }

    void sortItems(
            std::function<bool(ListItemData*, ListItemData*)> compare) {
        beginResetModel();
        std::sort(filterList.begin(), filterList.end(), compare);
        endResetModel();
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

 private:
    QVector<ListItemData> infoList;
    QVector<ListItemData*> filterList;
    QVector<ListItemData*> list;
    QVector<ListItemData*> cache;
    bool m_covers_loaded = false;
    QModelIndex a;
    QModelIndex b;
    // QVector<ListOriginalData> originalInfoList;
    Controller& controller = Controller::getInstance();
};

class CardItemDelegate : public QStyledItemDelegate {
 public:
    using QStyledItemDelegate::QStyledItemDelegate;

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const override {
        painter->save();

        // Card background
        painter->setRenderHint(QPainter::Antialiasing);
        QRectF cardRect = option.rect.adjusted(4, 4, -4, -4);
        QColor bgColor = option.state & QStyle::State_Selected ? QColor("#e0f7fa") : QColor("#ffffff");
        painter->setBrush(bgColor);
        painter->setPen(Qt::NoPen);
        painter->drawRoundedRect(cardRect, 10, 10);

        // Picture space (left side)
        QRect imageRect = QRect(cardRect.left() + 10, cardRect.top() + 10, 320, 240);
        painter->setBrush(QColor("#cccccc"));  // Placeholder color
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
        QString authors = index.data(Qt::UserRole + 1).toStringList().join(", ");
        QString type = index.data(Qt::UserRole + 2).toString();
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

        painter->restore();
    }

    QSize sizeHint(const QStyleOptionViewItem &, const QModelIndex &) const override {
        return QSize(600, 300);  // Adjust size as needed
    }
};

#endif  // SRC_LEVELVIEWLIST_HPP_
