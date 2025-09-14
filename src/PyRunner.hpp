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

#ifndef SRC_PYRUNNER_HPP_
#define SRC_PYRUNNER_HPP_

#include <QString>
#include <QVector>

class PyRunner {
 public:
    PyRunner() :
        m_status(0),
        m_isRunning(false)
    {}
    ~PyRunner() {}

    void run(const QString& script, const QVector<QString>& args);
    qint64 updateLevel(qint64 lid);
    qint64 syncCards();
    qint64 getStatus() const;

 private:
    qint64 m_status;
    bool m_isRunning;
};

#endif  // SRC_PYRUNNER_HPP_
