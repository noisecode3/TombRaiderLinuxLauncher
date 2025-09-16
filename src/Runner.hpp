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

#ifndef SRC_RUNNER_HPP_
#define SRC_RUNNER_HPP_

#include <QProcess>
#include <QProcessEnvironment>

#include "../src/Path.hpp"

class Runner : public QObject {
    Q_OBJECT

 public:
    Runner();
    void run();
    const quint64 getStatus();
    void clear();
    void setProgram(const quint64 command);
    void setWorkingDirectory(const Path& cwd);
    void setProcessEnvironment(const QPair<QString, QString> env);
    void setArguments(const QStringList& value);
    void setSetupFlag(bool setup);

 private:
    QString getCommandString(const quint64 cmd);
    QProcessEnvironment m_env;
    QStringList m_arguments;
    Path m_cwd;
    quint64 m_command;
    quint64 m_status;
    bool m_setupFlag;
    bool m_isRunning;
};

#endif  // SRC_RUNNER_HPP_
