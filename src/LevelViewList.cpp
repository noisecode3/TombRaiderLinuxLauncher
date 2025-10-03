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

#include "../src/LevelViewList.hpp"

#include <QDateTime>
#include "../src/staticViewData.hpp"
#include "../src/assert.hpp"


LevelViewList::LevelViewList(QWidget *parent)
    : QListView(parent)
{
    m_coversLoaded = false;
    m_proxyCoversFirst = false;
}

void LevelViewList::setProxyCoversFirst() {
    m_proxyCoversFirst = true;
}

void LevelViewList::scrollContentsBy(int dx, int dy)
{
    QListView::scrollContentsBy(dx, dy);
    if (!m_coversLoaded) {
        updateVisibleItems();
    }
}

void LevelViewList::paintEvent(QPaintEvent *event)
{
    QListView::paintEvent(event);
    if (m_proxyCoversFirst == true) {
        updateVisibleItems();
    }
}

QModelIndexList LevelViewList::visibleIndexes(QAbstractProxyModel* proxy) const
{
    QModelIndexList result;
    QRect visRect = viewport()->rect();

    for (int row = 0; row < model()->rowCount(); ++row) {
        QModelIndex idx = model()->index(row, 0);
        QRect itemRect = visualRect(idx);
        if (itemRect.isValid() && visRect.intersects(itemRect)) {
            result << proxy->mapToSource(idx);
        }
    }
    return result;
}

void LevelViewList::updateVisibleItems()
{
    QAbstractProxyModel* proxy = qobject_cast<QAbstractProxyModel *>(model());
    Q_ASSERT_WITH_TRACE(proxy != nullptr);
    LevelListModel* model = qobject_cast<LevelListModel*>(proxy->sourceModel());
    Q_ASSERT_WITH_TRACE( model != nullptr);
    if (model->stop()) {
        m_coversLoaded = true;
    } else {
        model->setScrollChanged(visibleIndexes(proxy));
    }
}

// The Model
void LevelListModel::setLevels(
        const QVector<QSharedPointer<ListItemData>>& levels) {
    beginResetModel();
    m_levels = levels;
    endResetModel();
}

int LevelListModel::rowCount(const QModelIndex &parent) const {
    return parent.isValid() ? 0 : m_levels.size();
}

void LevelListModel::setInstalled(const QModelIndex &index) {
    ListItemData &item = *m_levels[index.row()];
    item.m_installed = true;
}

QVariant LevelListModel::data(const QModelIndex &index, int role) const {
    QVariant result;

    if (index.isValid() && index.row() < m_levels.size()) {
        const ListItemData &item = *m_levels[index.row()];
        const auto it = m_roleTable.find(role);
        if (it != m_roleTable.end()) {
            result = it.value()(item);
        }
    }

    return result;
}

inline quint64 LevelListModel::indexInBounds(const quint64 index) const {
    return qMin(index, quint64(m_levels.size()));
}

void LevelListModel::setScrollChanged(QModelIndexList list) {
    if (!m_levels.empty()) {
        m_viewItems << list;
    }
}

QVector<QSharedPointer<ListItemData>>
        LevelListModel::getChunk(QModelIndexList list) {
    QVector<QSharedPointer<ListItemData>> result;
    for (QModelIndex& item : list) {
        result << m_levels.at(item.row());
    }
    return result;
}

QVector<QSharedPointer<ListItemData>>
        LevelListModel::getChunk(const quint64 cursor, const quint64 items) {
    const quint64 len = indexInBounds(cursor + items) - cursor;
    return m_levels.mid(cursor, len);
}

QVector<QSharedPointer<ListItemData>>
        LevelListModel::getDataBuffer(quint64 items) {
    QVector<QSharedPointer<ListItemData>> chunk;
    if (!m_levels.isEmpty()) {
        if (!m_viewItems.isEmpty()) {
            chunk = getChunk(m_viewItems);
        } else {
            m_cursor_b = indexInBounds(m_cursor_a + items);
            chunk = getChunk(m_cursor_a, items);
        }
    }

    return chunk;
}

void LevelListModel::updateCovers(QModelIndexList list) {
    for (QModelIndex& item : list) {
        emit dataChanged(item, item);
    }
}

void LevelListModel::updateCovers(quint64 a, quint64 b) {
    QModelIndex index_a(index(a, 0));
    QModelIndex index_b(index(b, 0));
    if (index_a.isValid() && index_b.isValid()) {
        emit dataChanged(index_a, index_b);
    }
}

void LevelListModel::reset() {
    if (!m_viewItems.isEmpty()) {
        updateCovers(m_viewItems);
        m_viewItems.clear();
    } else {
        updateCovers(m_cursor_a, m_cursor_b);
        m_cursor_a = m_cursor_b;
    }
}

bool LevelListModel::stop() const {
    return m_viewItems.isEmpty() && (m_cursor_b >= m_levels.size());
}

quint64 LevelListProxy::getLid(const QModelIndex &i) const {
    quint64 id = sourceModel()->data(i, Qt::UserRole+1).toInt();
    if (id == 0) {
        id = sourceModel()->data(i, Qt::UserRole+10).toInt();
    }
    Q_ASSERT_WITH_TRACE(id != 0);
    return id;
}

bool LevelListProxy::getItemType(const QModelIndex &i) const {
    return sourceModel()->data(i, Qt::UserRole+1).toBool();
}

bool LevelListProxy::getInstalled(const QModelIndex &i) const {
    return sourceModel()->data(i, Qt::UserRole+11).toBool();
}

void LevelListProxy::setClassFilter(const QString &c) {
    if (c == m_all) {
        m_class = 0;
    } else {
        m_class = StaticData::getClassID().at(c);
    }
    invalidateFilter();
}

void LevelListProxy::setTypeFilter(const QString &t) {
    if (t == m_all) {
        m_type = 0;
    } else {
        m_type = StaticData::getTypeID().at(t);
    }
    invalidateFilter();
}

void LevelListProxy::setDifficultyFilter(const QString &d) {
    if (d == m_all) {
        m_difficulty = 0;
    } else {
        m_difficulty = StaticData::getDifficultyID().at(d);
    }
    invalidateFilter();
}

void LevelListProxy::setDurationFilter(const QString &d) {
    if(d == m_all) {
        m_duration = 0;
    } else {
        m_duration = StaticData::getDurationID().at(d);
    }
    invalidateFilter();
}

void LevelListProxy::setSearchFilter(const QString &s) {
    m_search = s;
    invalidateFilter();
}

void LevelListProxy::setInstalledFilter(bool on) {
    m_installed = on;
    invalidateFilter();
}

void LevelListProxy::setSortMode(SortMode mode) {
    qDebug() << "LevelListProxy::setSortMode";
    Qt::SortOrder order;
    if (m_sortMode == mode){
        order = Qt::AscendingOrder;
    } else {
        m_sortMode = mode;
        order = Qt::DescendingOrder;
    }
    invalidate();
    this->sort(0, order);
}

bool LevelListProxy::lessThan(const QModelIndex &left,
                              const QModelIndex &right) const {
    const quint64 role = roleForMode();
    QVariant l = sourceModel()->data(left, role);
    QVariant r = sourceModel()->data(right, role);

    bool less = false;
    switch (m_sortMode) {
    case Title:
        less = l.toString().localeAwareCompare(r.toString().toUpper()) < 0;
        break;
    case ReleaseDate:
        less = l.toDateTime() < r.toDateTime();
        break;
    case Difficulty:
    case Duration:
    case Class:
    case Type:
        less = l.toUInt() < r.toUInt();
        break;
    default:
        less = false;
        break;
    }
    return less;
}

bool LevelListProxy::filterAcceptsRow(int sourceRow,
        const QModelIndex &parent) const {
    QModelIndex idx = sourceModel()->index(sourceRow, 0, parent);
    bool status = true;

    auto gameClass = sourceModel()->data(idx, Qt::UserRole+7).toUInt();
    if ((m_class != 0)  && (m_class != gameClass)) {
        status = false;
    }

    if (status == true) {
        auto type = sourceModel()->data(idx, Qt::UserRole+2).toUInt();
        if ((m_type != 0) && (m_type != type)) {
            status = false;
        }
    }

    if (status == true) {
        auto difficulty = sourceModel()->data(idx, Qt::UserRole+8).toUInt();
        if ((m_difficulty != 0) && (m_difficulty != difficulty)) {
            status = false;
        }
    }

    if (status == true) {
        auto duration = sourceModel()->data(idx, Qt::UserRole+9).toUInt();
        if ((m_duration != 0) && (m_duration != duration)) {
            status = false;
        }
    }

    if (status == true) {
        auto installed = sourceModel()->data(idx, Qt::UserRole+11).toBool();
        if (m_installed && !installed) {
            status = false;
        }
    }

    if (status == true) {
        auto title = sourceModel()->data(idx, Qt::DisplayRole).toString();
        if (!m_search.isEmpty() &&
                !title.contains(m_search, Qt::CaseInsensitive)) {
            status = false;
        }
    }

    return status;
}

int LevelListProxy::roleForMode() const {
    int index = static_cast<int>(m_sortMode);
    int result = Qt::UserRole+3; // default ReleaseDate
    if (index >= 0 && index < static_cast<int>(std::size(m_roleForModeTable))) {
        result = m_roleForModeTable[index];
    }
    return result;
}

void CardItemDelegate::paint(QPainter *painter,
        const QStyleOptionViewItem &option, const QModelIndex &index) const {
    painter->save();
    const bool selected = option.state & QStyle::State_Selected;
    const bool hovered = option.state & QStyle::State_MouseOver;
    const QPalette &palette = option.palette;

    // Card background
    painter->setRenderHint(QPainter::Antialiasing);
    QRectF cardRect = option.rect.adjusted(4, 4, -4, -4);
    QColor bgColor;
    if (selected) {
        bgColor = palette.color(QPalette::Highlight);
    } else if (hovered) {
        bgColor = palette.color(QPalette::AlternateBase);
    } else {
        bgColor = palette.color(QPalette::Base);
    }
    painter->setBrush(bgColor);
    painter->setPen(Qt::NoPen);
    painter->drawRoundedRect(cardRect, 0, 0);

    // Picture space (left side)
    qint64 x = cardRect.left() + 10;
    qint64 y = cardRect.top() + 40;
    QRect imageRect = QRect(x, y, 160, 120);
    painter->setBrush(QColor(0xFFCCCCCC));
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
    point.setX(imageRect.left());
    point.setY(textY + 15);
    painter->drawText(point, title);

    QFont normalFont = option.font;
    normalFont.setPointSizeF(option.font.pointSizeF() - 1);
    painter->setFont(normalFont);

    qint64 typeId = index.data(Qt::UserRole + 2).toInt();
    QString type = StaticData::getType().at(typeId);
    point.setX(textX);
    point.setY(textY + 60);
    QString typeText = QString("Type: %1").arg(type);
    painter->drawText(point, typeText);

    if (index.data(Qt::UserRole + 1).toInt() == 0) {
        QStringList authorsl = index.data(Qt::UserRole + 6).toStringList();
        QString authors = authorsl.join(", ");
        if (authors.size() > 100) {
            authors = "Various";
        }
        point.setY(textY + 45);
        QString authorsText = QString("By: %1").arg(authors);
        painter->drawText(point, authorsText);

        qint64 classId = index.data(Qt::UserRole + 7).toInt();
        QString class_ = StaticData::getClass().at(classId);
        point.setY(textY + 90);
        QString classText = QString("class: %1").arg(class_);
        painter->drawText(point, classText);

        qint64 difficultyId = index.data(Qt::UserRole + 8).toInt();
        QString difficulty = StaticData::getDifficulty().at(difficultyId);
        point.setY(textY + 120);
        QString difficultyText = QString("Difficulty: %1").arg(difficulty);
        painter->drawText(point, difficultyText);

        qint64 durationId = index.data(Qt::UserRole + 9).toInt();
        QString duration = StaticData::getDuration().at(durationId);
        point.setY(textY + 105);
        QString durationText = QString("Duration: %1").arg(duration);
        painter->drawText(point, durationText);
    } else {
        QString shortBody = index.data(Qt::UserRole + 5).toString();
        point.setY(textY + 90);
        painter->drawText(point, shortBody);
    }
    QString release = index.data(Qt::UserRole + 3).toString();
    point.setY(textY + 75);
    QString releaseText = QString("Released: %1").arg(release);
    painter->drawText(point, releaseText);

    painter->restore();
}

QSize CardItemDelegate::sizeHint(
        const QStyleOptionViewItem&, const QModelIndex&) const  {
    return QSize(600, 180);
}
