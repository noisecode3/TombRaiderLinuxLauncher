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

#ifndef SRC_GLOBALTYPES_HPP_
#define SRC_GLOBALTYPES_HPP_

#include <QtGlobal>
#include <QPair>
#include <QString>
#include <QStringList>

enum Runners {
    UMU = 0,
    WINE,
    LUTRIS,
    STEAM,
    BASH,
};

/**
 * @struct RunnerOptions
 * @brief Runner options.
 *
 */
struct RunnerOptions {
    quint64 id = 0;
    quint64 steamID = 0;
    quint64 command = 0;
    QList<QPair<QString, QString>> envList;
    QStringList arguments;
    bool setup = false;
};

#endif  // SRC_GLOBALTYPES_HPP_
