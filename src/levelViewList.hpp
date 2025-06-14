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

    void setLevels(const QList<int>& availableGames) {
        beginResetModel();
        m_mainList.clear();
        controller.getList(&m_mainList);
        for (ListItemData& item : m_mainList) {
            m_filterList.append(&item);
            m_sortList.append(&item);
        }
        setOriginalGames(availableGames);
        endResetModel();
        m_a = createIndex(0, 0);
        m_b = createIndex(99, 0);
        getMoreCovers();
    }

    void setOriginalGames(const QList<int>& availableGames) {
        const QString pictures = ":/pictures/";
        OriginalGameStaticData data;

        foreach(const int &id, availableGames) {
            int IdPositive;
            if (id < 0) {
                IdPositive = id*(-1);
            } else {
                IdPositive = id;
            }

            // Picture and title
            QString fileName = data.getCoverJPEG(IdPositive);
            QString coverPath = QString("%1%2").arg(pictures, fileName);
            QString title =
                QString("Tomb Raider %1 Original")
                    .arg(data.romanNumerals[IdPositive]);
            QString release = data.getRelease(IdPositive);
            qint64 type = data.getType(IdPositive);
            QString shortBody = data.getShortBody(IdPositive);

            qDebug() << "coverPath :" << coverPath;
            qDebug() << "title :" << title;

            m_originalMainList.append(OriginalGameData(
                IdPositive, title, shortBody,
                type, release, QPixmap(coverPath)));
        }
    }

    int rowCount(const QModelIndex&) const override {
        int rows = 0;
        if (m_original == true) {
            rows = m_originalMainList.count();
        } else {
            rows = m_filterList.count();
        }
        return rows;
    }

    QVariant data(const QModelIndex &index, int role) const override {
        QVariant result;
        int row = index.row();

        if (m_original == true) {
            if ((index.isValid()) && (row < m_originalMainList.count())) {
                const OriginalGameData* item = &m_originalMainList.at(row);
                switch (role) {
                    case Qt::DisplayRole:
                        result = item->m_title;
                        break;
                    case Qt::UserRole + 1:
                        result = item->m_shortBody;
                        break;
                    case Qt::UserRole + 2:
                        result = item->m_type;
                        break;
                    case Qt::UserRole + 5:
                        result = item->m_releaseDate;
                        break;
                    case Qt::UserRole + 7:
                        result = item->m_cover;
                        break;
                    case Qt::UserRole + 10:
                        result = true;
                        break;
                }
            }
        } else {
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
                    case Qt::UserRole + 10:
                        result = false;
                        break;
                }
            }
        }

        return result;
    }

    bool getListType() const {
        return m_original;
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
        m_filterList.clear();
        for (ListItemData* item : m_sortList) {
            m_filterList.append(item);
        }
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

    void filterClass(const QString& class_) {
        qDebug() << "filterClass " << class_;
        if (class_ == " - All -") {
            m_filter.class_.clear();
            qDebug() << "reset class";
        } else {
            m_filter.class_ = class_;
        }
        reFilter();
    }

    void filterType(const QString& type) {
        qDebug() << "filterType " << type;
        if (type == " - All -") {
            m_filter.type.clear();
            qDebug() << "reset type";
        } else {
            m_filter.type = type;
        }
        reFilter();
    }

    void filterDifficulty(const QString &difficulty) {
        qDebug() << "filterDifficulty " << difficulty;
        if (difficulty == " - All -") {
            m_filter.difficulty.clear();
            qDebug() << "reset difficulty";
        } else {
            m_filter.difficulty = difficulty;
        }
        reFilter();
    }

    void filterDuration(const QString& duration) {
        qDebug() << "filterDuration " << duration;
        if (duration == " - All -") {
            m_filter.duration.clear();
            qDebug() << "reset duration";
        } else {
            m_filter.duration = duration;
        }
        reFilter();
    }

    void filterSearch(const QString& search) {
        qDebug() << "filterSearch " << search;
        if (search == "") {
            m_search.clear();
            qDebug() << "reset search";
        } else {
            m_search.clear();
            m_search = search;
        }
        reFilter();
    }

    void reFilter() {
        qDebug() << "reFilter";
        beginResetModel();
        m_original = false;
        m_filterList.clear();

        for (ListItemData* item : m_sortList) {
            bool keep = true;

            if (!m_filter.class_.isEmpty()) {
                const QString itemString =
                    StaticData().getClass()[item->m_class];
                if (itemString != m_filter.class_) {
                    keep = false;
                }
            }

            if (!m_filter.type.isEmpty()) {
                const QString itemString =
                    StaticData().getType()[item->m_type];
                if (itemString != m_filter.type) {
                    keep = false;
                }
            }

            if (!m_filter.difficulty.isEmpty()) {
                const QString itemString =
                    StaticData().getDifficulty()[item->m_difficulty];
                if (itemString != m_filter.difficulty) {
                    keep = false;
                }
            }

            if (!m_filter.duration.isEmpty()) {
                const QString itemString =
                    StaticData().getDuration()[item->m_duration];
                if (itemString != m_filter.duration) {
                    keep = false;
                }
            }

            if (!m_search.isEmpty()) {
                QRegularExpression regex(
                        QRegularExpression::escape(m_search),
                        QRegularExpression::CaseInsensitiveOption);
                if (!regex.match(item->m_title).hasMatch()) {
                    keep = false;
                }
            }

            if (keep)
                m_filterList.append(item);
        }

        endResetModel();
    }

    void showOriginal() {
        beginResetModel();
        m_original = true;
        endResetModel();
    }

    void setInstalledListOriginal(const QHash<int, bool> installed) {
        m_installedOriginal = installed;
    }
    void setInstalledList(const QHash<int, bool> installed) {
        m_installed = installed;
    }

    int getLid(const QModelIndex &index) const {
        qint64 id = 0;
        if (m_original) {
            const OriginalGameData* item = &m_originalMainList.at(index.row());
            id =  item->m_game_id;
        } else {
            const ListItemData* item = m_filterList.at(index.row());
            id =  item->m_trle_id;
        }
        return id;
    }

    void setInstalled(const QModelIndex &index) {
        int id = 0;
        if (m_original) {
            const OriginalGameData* item = &m_originalMainList.at(index.row());
            id =  item->m_game_id;
            m_installedOriginal.insert(id, true);
        } else {
            const ListItemData* item = m_filterList.at(index.row());
            id =  item->m_trle_id;
            m_installed.insert(id, true);
        }
    }

    bool getInstalled(const qint64 id) {
        bool result;
        if (m_original) {
            if (m_installedOriginal.contains(id)) {
                result = m_installedOriginal.value(id);
            } else {
                result = false;
            }
        } else {
            if (m_installed.contains(id)) {
                result = m_installed.value(id);
            } else {
                result = false;
            }
        }
        return result;
    }

 private:
    QVector<ListItemData> m_mainList;
    QVector<ListItemData*> m_sortList;
    QVector<ListItemData*> m_filterList;
    struct Filter {
        QString class_;
        QString type;
        QString difficulty;
        QString duration;
    } m_filter;
    QHash<int, bool> m_installedOriginal;
    QHash<int, bool> m_installed;
    QString m_search;
    QVector<ListItemData*> m_cache;
    bool m_covers_loaded = false;
    bool m_original = false;
    QModelIndex m_a;
    QModelIndex m_b;
    QVector<OriginalGameData> m_originalMainList;
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
        QPoint point;

        QFont boldFont = option.font;
        boldFont.setBold(true);
        painter->setFont(boldFont);
        painter->setPen(Qt::black);

        QString title = index.data(Qt::DisplayRole).toString();
        point.setX(textX);
        point.setY(textY + 15);
        painter->drawText(point, title);

        QFont normalFont = option.font;
        normalFont.setPointSizeF(option.font.pointSizeF() - 1);
        painter->setFont(normalFont);

        qint64 typeId = index.data(Qt::UserRole + 2).toInt();
        QString type = StaticData().getType()[typeId];
        point.setY(textY + 50);
        QString typeText = QString("Type: %1").arg(type);
        painter->drawText(point, typeText);

        if (index.data(Qt::DisplayRole + 10).toBool()) {
            QStringList authorsl = index.data(Qt::UserRole + 1).toStringList();
            QString authors = authorsl.join(", ");
            if (authors.size() > 100) {
                authors = "Various";
            }
            point.setY(textY + 35);
            QString authorsText = QString("By: %1").arg(authors);
            painter->drawText(point, authorsText);

            qint64 classId = index.data(Qt::UserRole + 3).toInt();
            QString class_ = StaticData().getClass()[classId];
            point.setY(textY + 80);
            QString classText = QString("class: %1").arg(class_);
            painter->drawText(point, classText);

            qint64 durationId = index.data(Qt::UserRole + 6).toInt();
            QString duration = StaticData().getDuration()[durationId];
            point.setY(textY + 95);
            QString durationText = QString("Duration: %1").arg(duration);
            painter->drawText(point, durationText);

            qint64 difficultyId = index.data(Qt::UserRole + 4).toInt();
            QString difficulty = StaticData().getDifficulty()[difficultyId];
            point.setY(textY + 110);
            QString difficultyText = QString("Difficulty: %1").arg(difficulty);
            painter->drawText(point, difficultyText);
        } else {
            QString shortBody = index.data(Qt::UserRole + 1).toString();
            point.setY(textY + 80);
            painter->drawText(point, shortBody);
        }

        QString release = index.data(Qt::UserRole + 5).toString();
        point.setY(textY + 65);
        QString releaseText = QString("Released: %1").arg(release);
        painter->drawText(point, releaseText);

        painter->restore();
    }

    QSize sizeHint(
            const QStyleOptionViewItem&, const QModelIndex&) const override {
        return QSize(600, 300);
    }
};

#endif  // SRC_LEVELVIEWLIST_HPP_
