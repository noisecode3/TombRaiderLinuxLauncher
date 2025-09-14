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
    explicit Runner(const QString& cmd);
    void run();
    int getStatus();
    int getCommand();
    bool setCommand(const QString& cmd);
    void setWorkingDirectory(const QString& cwd);
    void insertProcessEnvironment(const QPair<QString, QString> env);
    void clearProcessEnvironment();
    void insertArguments(const QStringList& value);
    void clearArguments();
    void setupFlag(bool setup);

 private:
    QProcessEnvironment m_env;
    QString m_command;
    QStringList m_arguments;
    qint64 m_status;
    bool m_setupFlag;
    bool m_isRunning;
    QString m_cwd;
};

#endif  // SRC_RUNNER_HPP_
