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
#include <qnamespace.h>
#include <QWebEngineScript>
#include <QWebEngineProfile>
#include <QWebEngineScriptCollection>

Info::Info(QWidget *parent)
    : QWidget(parent),
    infoContent(new InfoContent(this)),
    infoBar(new InfoBar(this))
{
    setObjectName("info");
    layout = new QVBoxLayout(this);
    layout->setContentsMargins(6, 6, 6, 6);
    layout->setSpacing(8);

    layout->addWidget(infoContent);
    infoBar->setMaximumHeight(44);
    layout->addWidget(infoBar);
}

void InfoContent::setWebEngineTheme()
{
    QPalette pal = infoWebEngineView->palette();
    QColor bg   = pal.color(QPalette::Base);
    QColor text = pal.color(QPalette::Text);

    QWebEngineScript script;
    script.setName("qt-theme");
    script.setInjectionPoint(QWebEngineScript::DocumentCreation);
    script.setWorldId(QWebEngineScript::MainWorld);
    script.setRunsOnSubFrames(true);

    script.setSourceCode(QString(R"(
    (function applyQtTheme() {

        // Dont run without document
        if (!document.documentElement) {
            requestAnimationFrame(applyQtTheme);
            return;
        }

        // make sure there is a <head>
        let head = document.head;
        if (!head) {
            head = document.createElement('head');
            document.documentElement.appendChild(head);
        }

        let style = document.getElementById('qt-theme-style');
        if (!style) {
            style = document.createElement('style');
            style.id = 'qt-theme-style';
            head.appendChild(style);
         }

        // Trusted Types-safe: use textContent
        style.textContent = `
            html, body {
                background-color: %1 !important;
                color: %2 !important;
            }

            html {
                scrollbar-color: %2 %1;
            }

            /* fallback old Chromium */
            ::-webkit-scrollbar-track {
                background: %1;
            }
            ::-webkit-scrollbar-thumb {
                background-color: %2;
            }

        `;
    })();)").arg(bg.name(), text.name()));


    QWebEngineProfile::defaultProfile()
        ->scripts()->insert(script);
}

InfoContent::InfoContent(QWidget *parent)
    : QWidget(parent)
{
    layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(8);

    infoWebEngineView = new QWebEngineView(this);
    infoWebEngineView->page()->setBackgroundColor(
        palette().color(QPalette::Base)
    );
    setWebEngineTheme();

    infoWebEngineView->setHtml(
        "<!doctype html><html><body></body></html>",
        QUrl("qrc:/")
        );

    layout->addWidget(infoWebEngineView);

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
}

InfoBar::InfoBar(QWidget *parent)
    : QWidget(parent)
{
    layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(8);

    pushButtonBack = new QPushButton(tr("Back"), this);
    pushButtonBack->setFixedSize(242, 32);
    layout->addWidget(pushButtonBack);

    pushButtonWalkthrough = new QPushButton(tr("Walkthrough"), this);
    pushButtonWalkthrough->setFixedSize(242, 32);
    layout->addWidget(pushButtonWalkthrough);
    layout->addSpacerItem(
    new QSpacerItem(0, 0,
        QSizePolicy::Expanding,
        QSizePolicy::Minimum)
    );
}

