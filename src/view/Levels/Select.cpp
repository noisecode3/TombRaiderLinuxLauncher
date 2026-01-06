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

#include "view/Levels/Select.hpp"
#include <qlineedit.h>

Select::Select(QWidget *parent)
    : QWidget(parent)
{
    setObjectName("select");
    layout = new QVBoxLayout(this);
    layout->setContentsMargins(6, 6, 6, 6);
    layout->setSpacing(8);

    filter = new Filter(this);
    layout->addWidget(filter, 0);

    levelViewList = new LevelViewList(this);
    levelViewList->setViewMode(QListView::ListMode);
    levelViewList->setFlow(QListView::LeftToRight);
    levelViewList->setWrapping(true);

    levelViewList->setResizeMode(QListView::ResizeMode::Fixed);
    levelViewList->setUniformItemSizes(true);

    levelViewList->setVerticalScrollMode(QAbstractItemView::ScrollPerItem);
    levelViewList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    levelViewList->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    levelListModel = new LevelListModel(this);
    levelListProxy = new LevelListProxy(this);
    levelListProxy->setSourceModel(levelListModel);
    levelViewList->setModel(levelListProxy);

    CardItemDelegate* delegate = new CardItemDelegate(levelViewList);
    levelViewList->setItemDelegate(delegate);
    levelViewList->setSpacing(8);
    levelViewList->setMouseTracking(true);
    layout->addWidget(levelViewList, 1);

    stackedWidgetBar = new StackedWidgetBar(this);
    layout->addWidget(stackedWidgetBar, 0);


    FilterGroupBoxSort *sortBox =
        filter->filterSecondInputRow->filterGroupBoxSort;
    FilterGroupBoxToggle *toggleBox =
        filter->filterSecondInputRow->filterGroupBoxToggle;

    connect(sortBox->radioButtonLevelName,
            &QRadioButton::clicked, this, [this]() {
        levelListProxy->setSortMode(LevelListProxy::Title);
        qDebug() << "QRadioButton LevelName clicked";
    });

    connect(sortBox->radioButtonDifficulty,
            &QRadioButton::clicked, this, [this]() {
        qDebug() << "QRadioButton Difficulty clicked";
        levelListProxy->setSortMode(LevelListProxy::Difficulty);
    });

    connect(sortBox->radioButtonDuration,
            &QRadioButton::clicked, this, [this]() {
        qDebug() << "QRadioButton Duration clicked";
        levelListProxy->setSortMode(LevelListProxy::Duration);
    });

    connect(sortBox->radioButtonClass,
            &QRadioButton::clicked, this, [this]() {
        qDebug() << "QRadioButton Class clicked";
        levelListProxy->setSortMode(LevelListProxy::Class);
    });

    connect(sortBox->radioButtonType,
            &QRadioButton::clicked, this, [this]() {
        qDebug() << "QRadioButton Type clicked";
        levelListProxy->setSortMode(LevelListProxy::Type);
    });

    connect(sortBox->radioButtonReleaseDate,
            &QRadioButton::clicked, this, [this]() {
        qDebug() << "QRadioButton ReleaseDate clicked";
        levelListProxy->setSortMode(LevelListProxy::ReleaseDate);
    });

    connect(toggleBox->checkBoxInstalled,
            &QCheckBox::clicked, this, [this]() {
        qDebug() << "QCheckBox Installed clicked";
        levelListProxy->setInstalledFilter(true);
    });

    connect(toggleBox->checkBoxOriginal,
            &QCheckBox::clicked, this, [this]() {
        qDebug() << "QCheckBox Original clicked";
    });

    FilterGroupBoxFilter *filterGroupBoxFilter =
        filter->filterFirstInputRow->filterGroupBoxFilter;

    connect(filterGroupBoxFilter->comboBoxClass,
            &QComboBox::currentTextChanged,
            this,
            [this](const QString &class_) {
        levelListProxy->setClassFilter(class_);
        qDebug() << "QComboBox Class clicked";
    });

    connect(filterGroupBoxFilter->comboBoxType,
            &QComboBox::currentTextChanged,
            this,
            [this](const QString &type) {
        levelListProxy->setTypeFilter(type);
        qDebug() << "QComboBox Class clicked";
    });

    connect(filterGroupBoxFilter->comboBoxDifficulty,
            &QComboBox::currentTextChanged,
            this,
            [this](const QString &difficulty) {
        levelListProxy->setDifficultyFilter(difficulty);
        qDebug() << "QComboBox Class clicked";
    });

    connect(filterGroupBoxFilter->comboBoxDuration,
            &QComboBox::currentTextChanged,
            this,
            [this](const QString &duration) {
        levelListProxy->setDurationFilter(duration);
        qDebug() << "QComboBox Class clicked";
    });

    FilterGroupBoxToggle * filterGroupBoxToggle =
        filter->filterSecondInputRow->filterGroupBoxToggle;
    connect(filterGroupBoxToggle->checkBoxInstalled,
            &QCheckBox::toggled,
            levelListProxy,
            &LevelListProxy::setInstalledFilter);

    FilterGroupBoxSearch* filterGroupBoxSearch =
        filter->filterFirstInputRow->filterGroupBoxSearch;
    connect(filterGroupBoxSearch->comboBoxSearch,
            &QComboBox::currentTextChanged,
            levelListProxy,
            &LevelListProxy::setSearchType);

    connect(filterGroupBoxSearch->lineEditSearch,
            &QLineEdit::textChanged,
            levelListProxy,
            &LevelListProxy::setSearchFilter);
}

void Select::setLevels(
    QVector<QSharedPointer<ListItemData>> &list) {
    levelListModel->setLevels(list);
}

bool Select::stop() {
    return levelListModel->stop();
}

QVector<QSharedPointer<ListItemData>> Select::getDataBuffer(quint64 lenght) {
    return levelListModel->getDataBuffer(20);
}

void  Select::reset() {
    levelListModel->reset();
}

void Select::setItemChanged(const QModelIndex &current) {
    if (current.isValid()) {
        m_current = levelListProxy->mapToSource(current);
    } else {
        qDebug() << "Current QModelIndex from LevelListProxy not valid";
    }
}

void Select::setInstalledLevel() {
    levelListModel->setInstalled(m_current);
}

void Select::setRemovedLevel() {
    levelListModel->clearInstalled(m_current);
    FilterSecondInputRow *filterSecondInputRow =
        filter->filterSecondInputRow;
    FilterGroupBoxToggle *filterGroupBoxToggle =
        filterSecondInputRow->filterGroupBoxToggle;
    if (filterGroupBoxToggle->checkBoxInstalled->isChecked()) {
        levelListProxy->setInstalledFilter(true);
    } else {
        NavigateWidgetBar *navigateWidgetBar =
            stackedWidgetBar->navigateWidgetBar;
        navigateWidgetBar->
            pushButtonDownload->setText("Download and install");
    }
}

quint64 Select::getLid() {
    return levelListProxy->getLid(m_current);
}

bool Select::getType() {
    return levelListProxy->getItemType(m_current);
}

void Select::setSortMode(LevelListProxy::SortMode mode) {
    levelListProxy->setSortMode(mode);
}

