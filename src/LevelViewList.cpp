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
#include "../src/staticData.hpp"

void LevelListModel::setLevels(const QVector<QSharedPointer<ListItemData>>& levels) {
    beginResetModel();
    m_levels = levels;
    endResetModel();
}

int LevelListModel::rowCount(const QModelIndex &parent) const {
    return parent.isValid() ? 0 : m_levels.size();
}

QVariant LevelListModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || index.row() >= m_levels.size())
        return {};
    const auto &item = m_levels[index.row()];
    switch (role) {
        case Qt::DisplayRole: return item->m_title;
        case Qt::UserRole+1:  return item->m_game_id;
        case Qt::UserRole+2:  return item->m_type;
        case Qt::UserRole+3:  return item->m_releaseDate;
        case Qt::UserRole+4:  return item->m_cover;
        case Qt::UserRole+5:  return item->m_shortBody;
        case Qt::UserRole+6:  return item->m_authors;
        case Qt::UserRole+7:  return item->m_class;
        case Qt::UserRole+8:  return item->m_difficulty;
        case Qt::UserRole+9:  return item->m_duration;
        case Qt::UserRole+10: return item->m_trle_id;
        case Qt::UserRole+11: return item->m_installed;
    }
    return {};
}

void LevelListModel::setScrollChange(const quint64 index) {
    m_scrollCoversCursorChanged = true;
    m_scrollCoversCursor = index;
}

QVector<QSharedPointer<ListItemData>> LevelListModel::getDataBuffer(quint64 items) {
    QVector<QSharedPointer<ListItemData>> buffer;

    if (items > 0 && !m_levels.isEmpty()) {
        quint64 size = m_levels.size();
        quint64 index_a, index_b;

        if (m_scrollCoversCursorChanged) {
            m_scrollCoversCursorChanged = false;
            index_a = m_scrollCoversCursor;
            index_b = qMin(m_scrollCoversCursor + items, size);
            m_scrollCoversCursor = index_b;
        } else {
            index_a = m_sequentialCoversCursor;
            index_b = qMin(m_sequentialCoversCursor + items, size);
            m_sequentialCoversCursor = index_b;
        }

        buffer.reserve(index_b - index_a);
        for (quint64 i = index_a; i < index_b; ++i) {
            buffer.append(m_levels[i]);
        }
    }

    beginResetModel();
    return buffer;
}

void LevelListModel::reset() {
    endResetModel();
}

bool LevelListModel::stop() {
    return (m_sequentialCoversCursor == m_levels.size());
}

quint64 LevelListProxy::getLid(const QModelIndex &i) {
    quint64 id = sourceModel()->data(i, Qt::UserRole+1).toInt();
    if (id == 0) {
        id = sourceModel()->data(i, Qt::UserRole+10).toInt();
    }
    return id;
}

bool LevelListProxy::getItemType(const QModelIndex &i) {
    return sourceModel()->data(i, Qt::UserRole+10).toBool();
}

bool LevelListProxy::getInstalled(const QModelIndex &i) {
    return sourceModel()->data(i, Qt::UserRole+11).toBool();
}

void LevelListProxy::setClassFilter(const QString &c) {
    m_class = c;
    invalidateFilter();
}

void LevelListProxy::setTypeFilter(const QString &t) {
    m_type = t;
    invalidateFilter();
}

void LevelListProxy::setDifficultyFilter(const QString &d) {
    m_difficulty = d;
    invalidateFilter();
}

void LevelListProxy::setDurationFilter(const QString &d) {
    m_duration = d;
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
    m_sortMode = mode;
    invalidate();
    this->sort(0);
}

bool LevelListProxy::lessThan(const QModelIndex &left,
                              const QModelIndex &right) const {
    int role = roleForMode();
    QVariant l = sourceModel()->data(left, role);
    QVariant r = sourceModel()->data(right, role);

    switch (m_sortMode) {
    case Title:
        return l.toString().localeAwareCompare(r.toString()) < 0;
    case ReleaseDate:
        return l.toDateTime() < r.toDateTime();
    default:
        return l.toInt() < r.toInt();
    }
}

bool LevelListProxy::filterAcceptsRow(int sourceRow,
        const QModelIndex &parent) const {
    QModelIndex idx = sourceModel()->index(sourceRow, 0, parent);

    auto title       = sourceModel()->data(idx, Qt::DisplayRole).toString();
    auto gameClass   = sourceModel()->data(idx, Qt::UserRole+7).toString();
    auto type        = sourceModel()->data(idx, Qt::UserRole+2).toString();
    auto difficulty  = sourceModel()->data(idx, Qt::UserRole+8).toString();
    auto duration    = sourceModel()->data(idx, Qt::UserRole+9).toString();
    auto installed   = sourceModel()->data(idx, Qt::UserRole+11).toBool();

    if (!m_class.isEmpty() && m_class != gameClass) {
        return false;
    }

    if (!m_type.isEmpty() && m_type != type) {
        return false;
    }

    if (!m_difficulty.isEmpty() && m_difficulty != difficulty) {
        return false;
    }

    if (!m_duration.isEmpty() && m_duration != duration) {
        return false;
    }

    if (m_installed && !installed) {
        return false;
    }

    if (!m_search.isEmpty() && !title.contains(m_search, Qt::CaseInsensitive)) {
        return false;
    }

    return true;
}

int LevelListProxy::roleForMode() const {
    switch (m_sortMode) {
    case Title:       return Qt::DisplayRole;
    case Difficulty:  return Qt::UserRole+8;
    case Duration:    return Qt::UserRole+9;
    case Class:       return Qt::UserRole+7;
    case Type:        return Qt::UserRole+2;
    case ReleaseDate: return Qt::UserRole+3;
    default:          return Qt::UserRole+3;
    }
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
    point.setX(imageRect.left());
    point.setY(textY + 15);
    painter->drawText(point, title);

    QFont normalFont = option.font;
    normalFont.setPointSizeF(option.font.pointSizeF() - 1);
    painter->setFont(normalFont);

    qint64 typeId = index.data(Qt::UserRole + 2).toInt();
    QString type = StaticData().getType()[typeId];
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
        QString class_ = StaticData().getClass()[classId];
        point.setY(textY + 90);
        QString classText = QString("class: %1").arg(class_);
        painter->drawText(point, classText);

        qint64 difficultyId = index.data(Qt::UserRole + 8).toInt();
        QString difficulty = StaticData().getDifficulty()[difficultyId];
        point.setY(textY + 120);
        QString difficultyText = QString("Difficulty: %1").arg(difficulty);
        painter->drawText(point, difficultyText);

        qint64 durationId = index.data(Qt::UserRole + 9).toInt();
        QString duration = StaticData().getDuration()[durationId];
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
