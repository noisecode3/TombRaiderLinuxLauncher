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
#ifndef VIEW_LEVELS_LOADINGINDICATOR_HPP_
#define VIEW_LEVELS_LOADINGINDICATOR_HPP_

#include <QWidget>
#include <QPropertyAnimation>

class LoadingIndicator : public QWidget {
    Q_OBJECT
    Q_PROPERTY(qreal angle READ angle WRITE setAngle)

 public:
    explicit LoadingIndicator(QWidget* parent = nullptr);
    QSize sizeHint() const override { return QSize(64, 64); }

    qreal angle() const { return m_angle; }
    void setAngle(qreal a) { m_angle = a; update(); }

 protected:
    void paintEvent(QPaintEvent* event) override;
    void showEvent(QShowEvent* event) override;
    void hideEvent(QHideEvent* event) override;

 private:
    qreal m_angle = 0;
    QPropertyAnimation* m_anim;
};

#endif // VIEW_LEVELS_LOADINGINDICATOR_HPP_
