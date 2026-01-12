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
#ifndef VIEW_LEVELS_INFO_HPP_
#define VIEW_LEVELS_INFO_HPP_

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QListWidget>
#include <QWebEngineView>

class InfoBar : public QWidget
{
    Q_OBJECT
public:
    /*
     * (ui->tabs->levels->stackedWidget->info)
     * InfoBar
     * ├── pushButtonWalkthrough
     * └── pushButtonBack
     */
    explicit InfoBar(QWidget *parent);
    QPushButton *pushButtonWalkthrough{nullptr};
    QPushButton *pushButtonBack{nullptr};
private:
    QHBoxLayout *layout{nullptr};
};

class InfoContent : public QWidget
{
    Q_OBJECT
public:
    /*
     * (ui->tabs->levels->stackedWidget->info)
     * InfoContent
     * ├── coverListWidget
     * └── infoWebEngineView
     */
    explicit InfoContent(QWidget *parent);
    QListWidget *coverListWidget{nullptr};
    QWebEngineView *infoWebEngineView{nullptr};

    void setWebEngineTheme();
private:
    QHBoxLayout *layout{nullptr};
};

class Info : public QWidget
{
    Q_OBJECT
public:
    /*
     * (ui->tabs->levels->stackedWidget)
     * Info
     * ├── infoContent ->
     * └── infoBar ->
     */
    explicit Info(QWidget *parent);
    InfoContent *infoContent{nullptr};
    InfoBar *infoBar{nullptr};
private:
    QVBoxLayout *layout{nullptr};
};

#endif  // VIEW_LEVELS_INFO_HPP_
