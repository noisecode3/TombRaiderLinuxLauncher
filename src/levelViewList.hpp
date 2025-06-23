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

class LevelListBuffer {
 public:
    LevelListBuffer() {
        m_coversLoaded = false;
        m_original = false;
        m_a = 0;
        m_b = 0;
        m_sortList = nullptr;
    }

    void setLevelList(const QVector<ListItemData> &levelList) {
        m_levelList = levelList;
    }

    void setGameList(const QVector<ListItemData> &gameList) {
        m_gameList = gameList;
    }

    QVector<ListItemData>* getLevelList() {
        return &m_levelList;
    }

    QVector<ListItemData>* getGameList() {
        return &m_gameList;
    }

    void setInstalledListOriginal(const QHash<int, bool> installed) {
        for (ListItemData& item : m_gameList) {
            if (installed.contains(item.m_game_id)) {
                if (installed.value(item.m_game_id) == true) {
                    item.m_installed = true;
                }
            }
        }
    }

    void setInstalledList(const QHash<int, bool> installed) {
        for (ListItemData& item : m_levelList) {
            if (installed.contains(item.m_trle_id)) {
                if (installed.value(item.m_trle_id) == true) {
                    item.m_installed = true;
                }
            }
        }
    }

    void toogelList() {
        if (m_original) {
            m_original =  false;
        } else {
            m_original =  true;
        }
    }

    bool getCoversLoaded() {
        return m_coversLoaded;
    }

    bool getOriginal() {
        return m_original;
    }

    qint64 getRowCount() const {
        int rows = 0;
        if (m_original == true) {
            rows = m_gameList.count();
        } else {
            rows = m_levelList.count();
        }
        return rows;
    }

    qint64 getA() const {return m_a;}
    qint64 getB() const {return m_b;}

    ListItemData* getItem(qint64 index) {
        ListItemData* card;
        if (m_original == true) {
            card = &m_gameList[index];
        } else {
            card = &m_levelList[index];
        }
        return card;
    }

    QVector<ListItemData*>* getNextBuffer() {
        m_cache.clear();
        if (!m_coversLoaded) {
            m_a = m_b;
            const qint64 count = m_levelList.count();
            if (count > m_b+100) {
                m_b += 100;
            } else {
                m_b = count;
                m_coversLoaded = true;
            }
            for (qint64 i = m_a ; i < m_b; i++) {
                m_cache.append(&m_levelList[i]);
            }
        }
    return &m_cache;
    }

    void setSortFilter(QVector<ListItemData*>* sortList) {
        m_sortList = sortList;
        reloadSorted();
    }

    void reloadSorted() {
        m_sortList->clear();
        if (m_original == true) {
            for (ListItemData& item : m_gameList) {
                m_sortList->append(&item);
            }
        } else {
            for (ListItemData& item : m_levelList) {
                m_sortList->append(&item);
            }
        }
    }

 private:
    QVector<ListItemData> m_levelList;
    QVector<ListItemData> m_gameList;
    QVector<ListItemData*>* m_sortList;
    QVector<ListItemData*> m_cache;
    bool m_coversLoaded;
    bool m_original;
    qint64 m_a;
    qint64 m_b;
};

class LevelListFilter {
 public:
    LevelListFilter() {
        m_installed = false;
        m_author = false;
        m_sortList = nullptr;
    }

    void setSortFilter(QVector<ListItemData*>* sortList) {
        m_sortList = sortList;
    }

    int getRowCount() const {
        return m_filterList.count();
    }

    ListItemData* getAtIndex(qint64 id) const {
        return m_filterList[id];
    }

    void filterClass(const QString& class_) {
        qDebug() << "filterClass " << class_;
        if (class_ == " - All -") {
            m_class.clear();
            qDebug() << "reset class";
        } else {
            m_class = class_;
        }
        reFilter();
    }

    void filterType(const QString& type) {
        qDebug() << "filterType " << type;
        if (type == " - All -") {
            m_type.clear();
            qDebug() << "reset type";
        } else {
            m_type = type;
        }
        reFilter();
    }

    void filterDifficulty(const QString &difficulty) {
        qDebug() << "filterDifficulty " << difficulty;
        if (difficulty == " - All -") {
            m_difficulty.clear();
            qDebug() << "reset difficulty";
        } else {
            m_difficulty = difficulty;
        }
        reFilter();
    }

    void filterDuration(const QString& duration) {
        qDebug() << "filterDuration " << duration;
        if (duration == " - All -") {
            m_duration.clear();
            qDebug() << "reset duration";
        } else {
            m_duration = duration;
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

    void filterInstalled() {
        qDebug() << "filterInstalled";
        if (m_installed == true) {
            m_installed = false;
            qDebug() << "reset installed";
        } else {
            m_installed = true;
        }
        reFilter();
    }

    void reFilter() {
        qDebug() << "reFilter";
        if (m_sortList != nullptr) {
            m_filterList.clear();

            for (ListItemData* item : *m_sortList) {
                bool keep = true;

                if (!m_class.isEmpty()) {
                    const QString itemString =
                        StaticData().getClass()[item->m_class];
                    if (itemString != m_class) {
                        keep = false;
                    }
                }

                if (!m_type.isEmpty()) {
                    const QString itemString =
                        StaticData().getType()[item->m_type];
                    if (itemString != m_type) {
                        keep = false;
                    }
                }

                if (!m_difficulty.isEmpty()) {
                    const QString itemString =
                        StaticData().getDifficulty()[item->m_difficulty];
                    if (itemString != m_difficulty) {
                        keep = false;
                    }
                }

                if (!m_duration.isEmpty()) {
                    const QString itemString =
                        StaticData().getDuration()[item->m_duration];
                    if (itemString != m_duration) {
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

                if (m_installed) {
                    if (!item->m_installed) {
                        keep = false;
                    }
                }

                if (keep)
                    m_filterList.append(item);
            }
        }
    }

 private:
    QVector<ListItemData*>* m_sortList;
    QVector<ListItemData*> m_filterList;
    QString m_class;
    QString m_type;
    QString m_difficulty;
    QString m_duration;
    bool m_installed;
    QString m_search;
    bool m_author;
};


class LevelListModel : public QAbstractListModel {
    Q_OBJECT

 public:
    explicit LevelListModel(QObject *parent): QAbstractListModel(parent) {}

    void setLevels(const QList<int>& availableGames) {
        beginResetModel();
        QVector<ListItemData> list;
        controller.getList(&list);
        m_buffer.setLevelList(list);
        setOriginalGames(availableGames);
        m_buffer.setSortFilter(&m_sortList);
        m_filter.setSortFilter(&m_sortList);
        m_filter.reFilter();
        endResetModel();
        getMoreCovers();
    }

    void setOriginalGames(const QList<int>& availableGames) {
        const QString pictures = ":/pictures/";
        OriginalGameStaticData data;
        QVector<ListItemData> list;

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

            ListItemData item;
            item.setGameId(IdPositive);
            item.setTitle(title);
            item.setShortBody(shortBody);
            item.setType(type);
            item.setReleaseDate(release);
            item.setPicture(QPixmap(coverPath));

            list.append(ListItemData(item));
        }

        m_buffer.setGameList(list);
    }

    int rowCount(const QModelIndex&) const override {
        return m_filter.getRowCount();
    }

    QVariant data(const QModelIndex &index, int role) const override {
        QVariant result;
        int row = index.row();

        if ((index.isValid()) && (row < m_filter.getRowCount())) {
            const ListItemData* item = m_filter.getAtIndex(row);
            switch (role) {
                case Qt::DisplayRole:
                    result = item->m_title;
                    break;
                case Qt::UserRole + 1:
                    result = item->m_game_id;
                    break;
                case Qt::UserRole + 2:
                    result = item->m_type;
                    break;
                case Qt::UserRole + 3:
                    result = item->m_releaseDate;
                    break;
                case Qt::UserRole + 4:
                    result = item->m_cover;
                    break;
                case Qt::UserRole + 5:
                    result = item->m_shortBody;
                    break;
                case Qt::UserRole + 6:
                    result = item->m_authors;
                    break;
                case Qt::UserRole + 7:
                    result = item->m_class;
                    break;
                case Qt::UserRole + 8:
                    result = item->m_difficulty;
                    break;
                case Qt::UserRole + 9:
                    result = item->m_duration;
                    break;
                case Qt::UserRole + 10:
                    result = item->m_trle_id;
                    break;
            }
        }

        return result;
    }

    void getMoreCovers() {
        QVector<ListItemData*>* cache = m_buffer.getNextBuffer();
        if (!cache->isEmpty()) {
            controller.getCoverList(cache);
        }
    }

    void loadMoreCovers() {
        QModelIndex indexA = createIndex(m_buffer.getA(), 0);
        QModelIndex indexB = createIndex(m_buffer.getB(), 0);
        emit dataChanged(indexA, indexB, {Qt::DecorationRole});
        getMoreCovers();
    }

    void sortItems(
            std::function<bool(ListItemData*, ListItemData*)> compare) {
        beginResetModel();
        std::sort(m_sortList.begin(), m_sortList.end(), compare);
        m_filter.reFilter();
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

    void filterClass(const QString& class_) {
        beginResetModel();
        m_filter.filterClass(class_);
        endResetModel();
    }

    void filterType(const QString& type) {
        beginResetModel();
        m_filter.filterType(type);
        endResetModel();
    }

    void filterDifficulty(const QString& difficulty) {
        beginResetModel();
        m_filter.filterDifficulty(difficulty);
        endResetModel();
    }

    void filterDuration(const QString& duration) {
        beginResetModel();
        m_filter.filterDuration(duration);
        endResetModel();
    }

    void filterSearch(const QString& search) {
        beginResetModel();
        m_filter.filterSearch(search);
        endResetModel();
    }

    void filterInstalled() {
        beginResetModel();
        m_filter.filterInstalled();
        endResetModel();
    }

    int getLid(const QModelIndex &index) const {
        const ListItemData* item = m_filter.getAtIndex(index.row());
        return item->m_trle_id;
    }

    void setInstalled(const QModelIndex &index) {
        ListItemData* item = m_filter.getAtIndex(index.row());
        item->m_installed = true;
    }

    void setInstalledListOriginal(const QHash<int, bool> installed) {
        m_buffer.setInstalledListOriginal(installed);
    }

    void setInstalledList(const QHash<int, bool> installed) {
        m_buffer.setInstalledList(installed);
    }

    void showOriginal() {
        beginResetModel();
        m_buffer.toogelList();
        m_buffer.reloadSorted();
        m_filter.reFilter();
        endResetModel();
    }

    bool getListType() {
        return m_buffer.getOriginal();
    }

    bool getInstalled(const QModelIndex &index) {
        return m_filter.getAtIndex(index.row())->m_installed;
    }

 private:
    LevelListBuffer m_buffer;
    QVector<ListItemData*> m_sortList;
    LevelListFilter m_filter;
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
        QPixmap cover = index.data(Qt::UserRole + 4).value<QPixmap>();

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

        if (index.data(Qt::UserRole + 1).toInt() == 0) {
            QStringList authorsl = index.data(Qt::UserRole + 6).toStringList();
            QString authors = authorsl.join(", ");
            if (authors.size() > 100) {
                authors = "Various";
            }
            point.setY(textY + 35);
            QString authorsText = QString("By: %1").arg(authors);
            painter->drawText(point, authorsText);

            qint64 classId = index.data(Qt::UserRole + 7).toInt();
            QString class_ = StaticData().getClass()[classId];
            point.setY(textY + 80);
            QString classText = QString("class: %1").arg(class_);
            painter->drawText(point, classText);

            qint64 difficultyId = index.data(Qt::UserRole + 8).toInt();
            QString difficulty = StaticData().getDifficulty()[difficultyId];
            point.setY(textY + 110);
            QString difficultyText = QString("Difficulty: %1").arg(difficulty);
            painter->drawText(point, difficultyText);

            qint64 durationId = index.data(Qt::UserRole + 9).toInt();
            QString duration = StaticData().getDuration()[durationId];
            point.setY(textY + 95);
            QString durationText = QString("Duration: %1").arg(duration);
            painter->drawText(point, durationText);
        } else {
            QString shortBody = index.data(Qt::UserRole + 5).toString();
            point.setY(textY + 80);
            painter->drawText(point, shortBody);
        }
        QString release = index.data(Qt::UserRole + 3).toString();
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
