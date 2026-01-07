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

#include "view/Levels/Info.hpp"

Info::Info(QWidget *parent)
    : QWidget(parent)
{
    setObjectName("info");
    layout = new QVBoxLayout(this);
    layout->setContentsMargins(6, 6, 6, 6);
    layout->setSpacing(8);

    infoContent = new InfoContent(this);
    layout->addWidget(infoContent);

    infoBar = new InfoBar(this);
    layout->addWidget(infoBar);

}

InfoContent::InfoContent(QWidget *parent)
    : QWidget(parent)
{
    layout = new QHBoxLayout(this);
    layout->setContentsMargins(6, 6, 6, 6);
    layout->setSpacing(8);

    coverListWidget = new QListWidget(this);

    // Get the vertical scrollbar size
    // to center the images for all themes
    qint64 scrollbarWidth = coverListWidget->
             style()->pixelMetric(QStyle::PM_ScrollBarExtent);
    QMargins margins = coverListWidget->contentsMargins();
    qint64 left = margins.left();
    qint64 right = margins.right();

    coverListWidget->setMinimumWidth(left+502+scrollbarWidth+right);
    coverListWidget->setMaximumWidth(left+502+scrollbarWidth+right);

    coverListWidget->setViewMode(QListView::IconMode);
    coverListWidget->setIconSize(QSize(502, 377));
    coverListWidget->setDragEnabled(false);
    coverListWidget->setAcceptDrops(false);
    coverListWidget->setDragDropMode(QAbstractItemView::NoDragDrop);
    coverListWidget->setDefaultDropAction(Qt::IgnoreAction);
    coverListWidget->setSelectionMode(QAbstractItemView::NoSelection);

    layout->addWidget(coverListWidget);

    infoWebEngineView = new QWebEngineView(this);
    layout->addWidget(infoWebEngineView);
}

InfoBar::InfoBar(QWidget *parent)
    : QWidget(parent)
{
    layout = new QHBoxLayout(this);
    layout->setContentsMargins(6, 6, 6, 6);
    layout->setSpacing(8);

    pushButtonWalkthrough = new QPushButton(this);
    layout->addWidget(pushButtonWalkthrough);

    pushButtonBack = new QPushButton(this);
    layout->addWidget(pushButtonBack);
}

Walkthrough::Walkthrough(QWidget *parent)
    : QWidget(parent)
{
    setObjectName("walkthrough");
    layout = new QVBoxLayout(this);
    layout->setContentsMargins(6, 6, 6, 6);
    layout->setSpacing(8);

    walkthroughWebEngineView = new QWebEngineView(this);
    layout->addWidget(walkthroughWebEngineView);

    walkthroughBar = new WalkthroughBar(this);
    layout->addWidget(walkthroughBar);
}

WalkthroughBar::WalkthroughBar(QWidget *parent)
    : QWidget(parent)
{
    layout = new QHBoxLayout(this);
    layout->setContentsMargins(6, 6, 6, 6);
    layout->setSpacing(8);

    walkthroughBackButton = new QPushButton(this);
    layout->addWidget(walkthroughBackButton);
}
