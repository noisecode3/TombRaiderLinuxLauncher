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

#include "view/Levels/Dialog.hpp"

Dialog::Dialog(QWidget *parent)
    : QWidget(parent),
      m_layout(new QVBoxLayout(this)),
      m_label(new QLabel(this)),
      m_optionContainer(new QWidget(this)),
      m_optionLayout(new QVBoxLayout(m_optionContainer)),
      m_buttonGroup(new QButtonGroup(this)),
      m_okButton(new QPushButton("OK", this))
{
    setObjectName("dialog");

    // ========== Frame to hold content ==========
    QFrame *frame = new QFrame(this);
    frame->setFrameShape(QFrame::StyledPanel);
    frame->setFrameShadow(QFrame::Plain);
    frame->setStyleSheet("QFrame { border: 1px solid #999; border-radius: 6px; padding: 10px; }");

    // Layout inside the frame
    QVBoxLayout *frameLayout = new QVBoxLayout(frame);
    frameLayout->setSpacing(6);
    frameLayout->setContentsMargins(10, 10, 10, 10);

    // Label
    m_label->setAlignment(Qt::AlignCenter);
    frameLayout->addWidget(m_label, 0, Qt::AlignHCenter);

    // Options
    m_optionLayout->setSpacing(4);
    m_optionLayout->setContentsMargins(0, 0, 0, 0);
    m_optionContainer->setLayout(m_optionLayout);
    frameLayout->addWidget(m_optionContainer, 0, Qt::AlignHCenter);

    // ========== OK + Cancel buttons ==========
    QHBoxLayout *buttonLayout = new QHBoxLayout;
    m_okButton->setFixedWidth(80);
    QPushButton *m_cancelButton = new QPushButton("Cancel", this);
    m_cancelButton->setFixedWidth(80);

    buttonLayout->addStretch();
    buttonLayout->addWidget(m_okButton);
    buttonLayout->addWidget(m_cancelButton);
    buttonLayout->addStretch();

    frameLayout->addLayout(buttonLayout);

    // ========== Main layout ==========
    m_layout->setSpacing(4);
    m_layout->setContentsMargins(10, 10, 10, 10);
    m_layout->addStretch(1);
    m_layout->addWidget(frame, 0, Qt::AlignHCenter);
    m_layout->addStretch(1);

    // ========== Connections ==========
    connect(m_okButton, &QPushButton::clicked, this, [this]() {
        const QString selected = this->selectedOption();
        qDebug() << "OK clicked, selected:" << selected;
        emit setLevelsState(selected);
    });

    connect(m_cancelButton, &QPushButton::clicked, this, [this]() {
        qDebug() << "Cancel clicked";
        emit cancelClicked();
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
    } else if (options.size() == 1) {
        m_optionContainer->hide();
        m_oneOptionHolder = options.at(0);
    } else {
        m_optionContainer->show();
        m_oneOptionHolder.clear();
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
    this->updateGeometry();
}

QString Dialog::selectedOption() const {
    QString result;
    if (m_oneOptionHolder.isEmpty()) {
        auto btn = m_buttonGroup->checkedButton();
        result = btn ? btn->text() : QString();
    } else {
        result = m_oneOptionHolder;
    }
    return result;
}

