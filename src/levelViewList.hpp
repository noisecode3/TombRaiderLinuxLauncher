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
        infoList.clear();
        controller.getList(&infoList);
        beginResetModel();
        expandRange();
    }

    int rowCount(const QModelIndex &parent = QModelIndex()) const override {
        Q_UNUSED(parent);
        return m_loadedRows;
    }

    QVariant data(const QModelIndex &index, int role) const override {
        QVariant result;
        int row = index.row();

        if (index.isValid() && row < infoList.count()) {
            const ListItemData &item = infoList.at(row);
            switch (role) {
                case Qt::DisplayRole:
                    return item.m_title;
                case Qt::UserRole + 1:
                    return item.m_authors;
                case Qt::UserRole + 2:
                    return item.m_type;
                case Qt::UserRole + 3:
                    return item.m_class;
                case Qt::UserRole + 4:
                    return item.m_difficulty;
                case Qt::UserRole + 5:
                    return item.m_releaseDate;
                case Qt::UserRole + 6:
                    return item.m_duration;
                case Qt::UserRole + 7:
                    return item.m_cover;
                default:
                    return QVariant();
            }
        }

        return result;
    }

    int getLid(const QModelIndex &index) const {
        const ListItemData &item = infoList.at(index.row());
        return item.m_trle_id;
    }

    void expandRange() {
        qint64 a = m_loadedRows;
        qint64 b = m_loadedRows + 100;

        // Set states
        if (infoList.count() > b) {  // we have more then 100 extra
            m_loadedRows = b;
            m_stop = false;
        } else {  // we have less or equal then 100 extra
            b = infoList.count() - 1;
            m_loadedRows = b;
            m_stop = true;
            endResetModel();
        }
        qDebug() << "expandRange a:" << a;
        qDebug() << "expandRange b:" << b;
        qDebug() << "expandRange m_loadedRows:" << m_loadedRows;

        // Add the covers
        pictureList.clear();
        for (qint64 i = a; i < b; i++) {
            pictureList.append(&infoList[i]);
        }
        controller.getCoverList(&pictureList);
    }

    void loadMoreLevels() {
        if (!m_stop) {
            // we stop to prevent extra call while in here.
            m_stop = true;
            endResetModel();
            beginResetModel();
            expandRange();
        }
    }

    void sortItems(
        std::function<bool(ListItemData, ListItemData)> compare) {
        beginResetModel();
        std::sort(infoList.begin(), infoList.end(), compare);
        endResetModel();
    }

    static bool compareTitles(const ListItemData &a, const ListItemData &b) {
        return a.m_title.toLower() < b.m_title.toLower();
    }

    static bool compareDifficulties(
            const ListItemData &a, const ListItemData &b) {
        return a.m_difficulty > b.m_difficulty;
    }

    static bool compareDurations(const ListItemData &a, const ListItemData &b) {
        return a.m_duration > b.m_duration;
    }

    static bool compareClasses(const ListItemData &a, const ListItemData &b) {
        return a.m_class > b.m_class;
    }

    static bool compareTypes(const ListItemData &a, const ListItemData &b) {
        return a.m_type > b.m_type;
    }

    static bool compareReleaseDates(
            const ListItemData &a, const ListItemData &b) {
        // descending order
        return a.m_releaseDate > b.m_releaseDate;
    }

 private:
    QVector<ListItemData> infoList;
    // QVector<ListOriginalData> originalInfoList;
    QVector<ListItemData*> pictureList;
    bool m_stop = false;
    qint64 m_loadedRows = 0;
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
        QRect imageRect = QRect(cardRect.left() + 10, cardRect.top() + 10, 80, 60);
        painter->setBrush(QColor("#cccccc"));  // Placeholder color
        painter->drawRect(imageRect);
        QPoint coverPoint(cardRect.left() + 10, cardRect.top() + 10);
        QPixmap cover = index.data(Qt::UserRole + 7).value<QPixmap>();
        painter->drawPixmap(coverPoint, cover);

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
        return QSize(300, 80);  // Adjust size as needed
    }
};

#endif  // SRC_LEVELVIEWLIST_HPP_
