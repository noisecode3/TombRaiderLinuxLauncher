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

#include "view/Levels/StackedWidgetBar.hpp"

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
    layout->setContentsMargins(6, 6, 6, 6);
    layout->setSpacing(8);
    //setMaximumHeight(20);

    // Set init state
    pushButtonRun->setEnabled(false);
    layout->addWidget(pushButtonRun);
    layout->addWidget(checkBoxSetup);

    layout->addWidget(pushButtonFilter);

    pushButtonInfo->setEnabled(false);
    layout->addWidget(pushButtonInfo);

    pushButtonDownload->setEnabled(false);
    layout->addWidget(pushButtonDownload);
}

ProgressWidgetBar::ProgressWidgetBar(QWidget *parent)
    : QWidget(parent),
    progressBar(new QProgressBar(this)),
    layout(new QHBoxLayout(this))
{
    setObjectName("progressWidgetBar");
    layout->setContentsMargins(6, 6, 6, 6);
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


void StackedWidgetBar::downloadError(int status) {
    this->progressWidgetBar->progressBar->setValue(0);
    this->navigateWidgetBar->pushButtonRun->setEnabled(true);
    this->navigateWidgetBar->pushButtonInfo->setEnabled(true);
    this->navigateWidgetBar->pushButtonDownload->setEnabled(true);
    this->setCurrentWidget(
            this->findChild<QWidget*>("navigate"));
    // ui->levels->select->levelViewList->setEnabled(true);
    // QMessageBox msgBox;
    // msgBox.setWindowTitle("Error");
    if (status == 1) {
        qDebug() << "No internet";
    //     msgBox.setText("No internet");
    } else if (status == 2) {
        qDebug() << "You seem to be missing ssl keys";
    //     msgBox.setText("You seem to be missing ssl keys");
    } else {
        qDebug() << "Could not connect";
    //     msgBox.setText("Could not connect");
    }
    // msgBox.setStandardButtons(QMessageBox::Ok);
    // msgBox.setDefaultButton(QMessageBox::Ok);
    // msgBox.exec();
}


void StackedWidgetBar::backClicked() {
    if (this->currentWidget() ==
        this->findChild<QWidget*>("info")) {
        this->setCurrentWidget(
                this->findChild<QWidget*>("select"));
    } else if (this->currentWidget() ==
        this->findChild<QWidget*>("walkthrough")) {
        this->setCurrentWidget(
                this->findChild<QWidget*>("select"));
    }
}

