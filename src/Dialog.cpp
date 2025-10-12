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
    // Remove or reduce spacing between widgets
    m_layout->setSpacing(4);  // smaller space between widgets
    m_layout->setContentsMargins(10, 10, 10, 10);

    // Add stretch to push content to vertical center
    m_layout->addStretch(1);

    // Label (centered)
    m_label->setAlignment(Qt::AlignCenter);
    m_layout->addWidget(m_label, 0, Qt::AlignHCenter);

    // Option container (centered and compact)
    m_optionLayout->setContentsMargins(0, 0, 0, 0);
    m_optionLayout->setSpacing(4);
    m_optionContainer->setLayout(m_optionLayout);
    m_layout->addWidget(m_optionContainer, 0, Qt::AlignHCenter);

    // OK Button (centered, fixed width)
    m_okButton->setFixedWidth(80);
    m_layout->addWidget(m_okButton, 0, Qt::AlignHCenter);

    m_layout->addStretch(1);

    // Connect OK button
    connect(m_okButton, &QPushButton::clicked, this, [this]() {
        emit okClicked();
    });
}

void Dialog::setMessage(const QString &text) {
    m_label->setText(text);
    this->adjustSize();
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
    } else {
        m_optionContainer->show();
        for (quint64 i = 0; i < options.size(); i++) {
            QRadioButton *rb = new QRadioButton(options.at(i));
            if (i == 0) {
                rb->setChecked(true);
            }
            m_buttonGroup->addButton(rb);
            m_optionLayout->addWidget(rb);
        }
    }

    this->adjustSize();
}

QString Dialog::selectedOption() const {
    auto btn = m_buttonGroup->checkedButton();
    return btn ? btn->text() : QString();
}

