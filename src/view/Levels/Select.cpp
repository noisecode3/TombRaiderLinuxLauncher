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


    CardItemDelegate* delegate = new CardItemDelegate(levelViewList);
    levelViewList->setItemDelegate(delegate);
    levelViewList->setSpacing(8);
    levelViewList->setMouseTracking(true);
    layout->addWidget(levelViewList, 1);

    stackedWidgetBar = new StackedWidgetBar(this);
    layout->addWidget(stackedWidgetBar, 0);
}

