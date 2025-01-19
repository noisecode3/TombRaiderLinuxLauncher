/* TombRaiderLinuxLauncher
 * Martin Bångens Copyright (C) 2024
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef SRC_RUNNER_HPP_
#define SRC_RUNNER_HPP_

#include <QProcess>
#include <QProcessEnvironment>
#include <QDebug>


class Runner : public QObject {
    Q_OBJECT

 public:
    Runner();
    explicit Runner(const QString& cmd);
    void run();
    int getStatus();
    int getCommand();
    bool setCommand(const QString& cmd);

 signals:
    void started();
    void stopped();

 private:
    QProcessEnvironment m_env;
    QProcess m_process;
    QString m_command;
    qint64 m_status;
};

#endif  // SRC_RUNNER_HPP_
