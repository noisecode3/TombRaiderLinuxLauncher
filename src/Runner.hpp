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

class Runner : public QObject {
    Q_OBJECT

 public:
    Runner();
    void run();
    const quint64 getStatus();
    void clear();
    void setProgram(const quint64 command);
    void setCurrentWorkingDirectory(const QString& dir);
    void addEnvironmentVariable(const QPair<QString, QString> env);
    void addArguments(const QStringList& value);

 private:
    QString getCommandString(const quint64 cmd);
    QProcessEnvironment m_env;
    QStringList m_arguments;
    QString m_cwd;
    quint64 m_command;
    quint64 m_status;
    bool m_isRunning;
};

#endif  // SRC_RUNNER_HPP_
