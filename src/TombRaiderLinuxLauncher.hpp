/* TombRaiderLinuxLauncher
 * Martin Bångens Copyright (C) 2024
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef SRC_TOMBRAIDERLINUXLAUNCHER_HPP_
#define SRC_TOMBRAIDERLINUXLAUNCHER_HPP_

#include <QMainWindow>
#include <QSettings>
#include <QMessageBox>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QStringList>
#include <QListWidgetItem>
#include <QStyledItemDelegate>
#include <QSet>
#include <QDebug>
#include <QVector>
#include <QString>

#include "../src/Controller.hpp"

QT_BEGIN_NAMESPACE
namespace Ui { class TombRaiderLinuxLauncher; }
QT_END_NAMESPACE

class LevelListModel : public QAbstractListModel {
    Q_OBJECT

 public:
    explicit LevelListModel(QObject *parent): QAbstractListModel(parent) {}

    void setLevels() {
        beginResetModel();
        infoList.clear();
        controller.getList(&infoList);
        endResetModel();
    }

    int rowCount(const QModelIndex &parent = QModelIndex()) const override {
        Q_UNUSED(parent);
        return infoList.count();
    }

    QVariant data(const QModelIndex &index, int role) const override {
        QVariant result;
        int row = index.row();

        if (index.isValid() && row < infoList.size()) {
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

 private:
    Controller& controller = Controller::getInstance();
    QVector<ListItemData> infoList;
    QVector<ListItemPicture> pictureList;
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


/**
 * View component in the MVC pattern; Main UI class for the launcher.
 */
class TombRaiderLinuxLauncher : public QMainWindow {
    Q_OBJECT

 public:
    /**
     * Constructs the main window.
     */
    explicit TombRaiderLinuxLauncher(QWidget *parent = nullptr);
    ~TombRaiderLinuxLauncher();

 public slots:
    /**
     * Triggered by the "Link" button.
     */
    void linkClicked();
    /**
     * Triggered by the "Download" button.
     */
    void downloadClicked();
    /**
     * Opens the Info thru the navigation bar.
     */
    void infoClicked();
    /**
     * Opens the Walkthrough thru the navigation bar.
     */
    void walkthroughClicked();
    /**
     * Returns to the first navigation state, the list.
     */
    void backClicked();
    /**
     * Opens the first-time setup options.
     */
    void setOptionsClicked();
    /**
     * Updates button states based on selected menu level.
     */
    void onCurrentItemChanged(const QModelIndex &current, const QModelIndex &previous);
    /**
     * Updates progress by 1% of total work steps.
     */
    void workTick();
    /**
     * Displays an error dialog for a curl download error.
     */
    void downloadError(int status);
    /**
     * Generates the initial level list after file analysis.
     */
    void generateList(const QList<int>& availableGames);
    /**
     * Sorts the list by author.
     */
    void sortByAuthor();
    /**
     * Sorts the list by title.
     */
    void sortByTitle();
    /**
     * Sorts the list by difficulty.
     */
    void sortByDifficulty();
    /**
     * Sorts the list by duration.
     */
    void sortByDuration();
    /**
     * Sorts the list by level class.
     */
    void sortByClass();
    /**
     * Sorts the list by type.
     */
    void sortByType();
    /**
     * Sorts the list by release date.
     */
    void sortByReleaseDate();

    void GlobalSaveClicked();
    void GlobalResetClicked();
    void LevelSaveClicked();
    void LevelResetClicked();

 private:
    /**
     * 
     */
     void originalSelected(qint64 id);
    /**
     * 
     */
    void levelDirSelected(qint64 id);
    /**
     * Configures game and level directories.
     */
    void setup();
    /**
     * Loads saved settings.
     */
    void readSavedSettings();
    /**
     * Executes the sorting algorithm.
     */
    void sortItems(std::function<bool(
        QListWidgetItem*,
        QListWidgetItem*)> compare);

    QSet<QListWidgetItem*> originalGamesSet_m;
    QList<QListWidgetItem*> originalGamesList_m;
    LevelListModel *model;
    Controller& controller = Controller::getInstance();
    QSettings m_settings;
    Ui::TombRaiderLinuxLauncher *ui;
};

struct OriginalGameData {
    QMap<int, QString> romanNumerals = {
            {0, "null"},
            {1, "I"},
            {2, "II"},
            {3, "III"},
            {4, "IV"},
            {5, "V"},
            {6, "VI"},
            {7, "IUB"},
            {8, "IIGM"},
            {9, "IIILM"},
    };
    const QString getPicture(int id) {
        return QString ("Tomb_Raider_%1.jpg").arg(romanNumerals[id]);
    }
};

#endif  // SRC_TOMBRAIDERLINUXLAUNCHER_HPP_
