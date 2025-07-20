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

#include "../src/LoadingIndicator.hpp"
#include <QPainter>
#include <QtMath>

LoadingIndicator::LoadingIndicator(QWidget* parent) : QWidget(parent) {
    setAttribute(Qt::WA_TransparentForMouseEvents);
    setAttribute(Qt::WA_OpaquePaintEvent, false);

    m_anim = new QPropertyAnimation(this, "angle");
    m_anim->setStartValue(0);
    m_anim->setEndValue(360);
    m_anim->setDuration(1000);
    m_anim->setLoopCount(-1);
}

void LoadingIndicator::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    int count = 12;
    qreal radius = qMin(width(), height()) / 2.5;
    QPointF center = rect().center();

    for (int i = 0; i < count; ++i) {
        qreal theta = (360.0 / count) * i + m_angle;
        qreal alpha = 255.0 * (i + 1) / count;

        QPointF dotPos = center + QPointF(
            radius * std::cos(qDegreesToRadians(theta)),
            radius * std::sin(qDegreesToRadians(theta)));

        QColor color = Qt::gray;
        color.setAlphaF(alpha / 255.0);

        p.setBrush(color);
        p.setPen(Qt::NoPen);
        p.drawEllipse(dotPos, 4, 4);
    }
}
void LoadingIndicator::showEvent(QShowEvent* event) {
    QWidget::showEvent(event);
    m_anim->start();
}

void LoadingIndicator::hideEvent(QHideEvent* event) {
    QWidget::hideEvent(event);
    m_anim->stop();
}
