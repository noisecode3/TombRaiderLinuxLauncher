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

#include "../src/Dialog.hpp"


Dialog::Dialog(QWidget *parent)
    : QWidget(parent),
      m_layout(new QVBoxLayout(this)),
      m_label(new QLabel(this)),
      m_optionContainer(new QWidget(this)),
      m_optionLayout(new QVBoxLayout(m_optionContainer)),
      m_buttonGroup(new QButtonGroup(this)),
      m_okButton(new QPushButton("OK", this))
{
    m_layout->addWidget(m_label);
    m_layout->addWidget(m_optionContainer);
    m_layout->addWidget(m_okButton);

    connect(m_okButton, &QPushButton::clicked, this, [this]() {
        emit okClicked();
    });
}

void Dialog::setMessage(const QString &text) {
    m_label->setText(text);
}

void Dialog::setOptions(const QStringList &options) {
    // Clear old options
    qDeleteAll(m_buttonGroup->buttons());
    QLayoutItem *child;
    while ((child = m_optionLayout->takeAt(0)) != nullptr) {
        delete child->widget();
        delete child;
    }

    if (options.isEmpty()) {
        m_optionContainer->hide();
        return;
    }

    m_optionContainer->show();
    for (const QString &opt : options) {
        QRadioButton *rb = new QRadioButton(opt);
        m_buttonGroup->addButton(rb);
        m_optionLayout->addWidget(rb);
    }
}

QString Dialog::selectedOption() const {
    auto btn = m_buttonGroup->checkedButton();
    return btn ? btn->text() : QString();
}

