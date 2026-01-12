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

#include "view/Levels/Select/StackedWidgetBar.hpp"

NavigateWidgetBar::NavigateWidgetBar(QWidget *parent)
    : QWidget(parent),
    pushButtonRun(new QPushButton(("Run"), this)),
    checkBoxSetup(new QCheckBox(("Setup"), this)),
    pushButtonFilter(new QPushButton(("Filter/Sort"), this)),
    pushButtonInfo(new QPushButton(("Info"), this)),
    pushButtonDownload(new QPushButton(("Download and install"), this)),
    layout(new QHBoxLayout(this))

{
    setObjectName("navigateWidgetBar");
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(8);
    //setMaximumHeight(20);

    // Set init state
    pushButtonRun->setEnabled(false);
    pushButtonRun->setFixedSize(242, 32);
    layout->addWidget(pushButtonRun);

    layout->addWidget(checkBoxSetup);

    pushButtonFilter->setFixedSize(242, 32);
    layout->addWidget(pushButtonFilter);

    pushButtonInfo->setEnabled(false);
    pushButtonInfo->setFixedSize(242, 32);
    layout->addWidget(pushButtonInfo);

    pushButtonDownload->setEnabled(false);
    pushButtonDownload->setFixedSize(242, 32);
    layout->addWidget(pushButtonDownload);
}

ProgressWidgetBar::ProgressWidgetBar(QWidget *parent)
    : QWidget(parent),
    progressBar(new QProgressBar(this)),
    layout(new QHBoxLayout(this))
{
    setObjectName("progressWidgetBar");
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(8);

    layout->addWidget(progressBar);
}

StackedWidgetBar::StackedWidgetBar(QWidget *parent)
    : QStackedWidget(parent),
    navigateWidgetBar(new NavigateWidgetBar(this)),
    progressWidgetBar(new ProgressWidgetBar(this))
    
{
    setObjectName("stackedWidgetBar");
    this->addWidget(navigateWidgetBar);
    this->setCurrentWidget(navigateWidgetBar);
    this->addWidget(progressWidgetBar);
}

