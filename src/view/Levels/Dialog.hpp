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
#ifndef VIEW_LEVELS_DIALOG_HPP_
#define VIEW_LEVELS_DIALOG_HPP_

#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QRadioButton>
#include <QButtonGroup>
#include <QWidget>

class Dialog : public QWidget {
    Q_OBJECT

 public:
    /*
     * (ui->tabs->levels->stackedWidget)
     * UiDialog
     */
    explicit Dialog(QWidget* parent);
    void setMessage(const QString &text);
    void setOptions(const QStringList &options);
    QString selectedOption() const;

 signals:
    void okClicked();
    void setLevelsState(QString qwidget);
    void cancelClicked();

 private:
    QVBoxLayout* m_layout;
    QLabel* m_label;
    QWidget* m_optionContainer;
    QVBoxLayout* m_optionLayout;
    QButtonGroup* m_buttonGroup;
    QPushButton* m_okButton;
    QString m_oneOptionHolder;
};

#endif  // VIEW_LEVELS_DIALOG_HPP_
