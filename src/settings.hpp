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

#ifndef SRC_SETTINGS_HPP_
#define SRC_SETTINGS_HPP_

#include <QSettings>


static QSettings& getSettingsInstance() {
    // cppcheck-suppress threadsafety-threadsafety
    static QSettings instance;
    return instance;
}

#endif  // SRC_SETTINGS_HPP_
