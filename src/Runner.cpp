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

#include "../src/assert.hpp"
#include "../src/Runner.hpp"
#include <QDebug>
#include <QTextStream>
#include <QObject>

Runner::Runner()
    : m_env(QProcessEnvironment::systemEnvironment()),
    m_command(0),
    m_status(0)
{
    m_isRunning = false;

}

const quint64 Runner::getStatus() {
    return m_status;
}

void Runner::clear() {
    m_arguments.clear();
    m_env.clear();
    m_winePath.clear();
    m_env.insert(QProcessEnvironment::systemEnvironment());
    m_cwd.clear();
    m_command = 0;
    m_status = 0;
}

void Runner::setProgram(const quint64 command) {
    m_command = command;
}

void Runner::setCurrentWorkingDirectory(const QString& dir) {
    m_cwd = dir;
}

void Runner::addEnvironmentVariable(const QPair<QString, QString> env) {
    m_env.insert(env.first, env.second);
}

void Runner::addArguments(const QStringList& value) {
    m_arguments << value;
}

QString Runner::getCommandString(const quint64 cmd) {
    QString result;
    if (cmd == 0) {
        result = "umu-run";
    } else if (cmd == 1) {
        result = "wine";
    } else if (cmd == 2) {
        result = "lutris";
    } else if (cmd == 3) {
        result = "steam";
    } else if (cmd == 4) {
        result = "bash";
    } else {
        Q_ASSERT_WITH_TRACE(false);
    }
    return result;
}

void Runner::setWinePath(const QString& path) {
    m_winePath = path;
}

void Runner::run() {
    if (m_isRunning) {
        qWarning() << "[Runner] Already running!";
        return;
    }
    Q_ASSERT_WITH_TRACE(!m_cwd.isEmpty());

    m_isRunning = true;
    QProcess process;

    process.setWorkingDirectory(m_cwd);
    process.setProcessEnvironment(m_env);
    if (m_winePath.isEmpty() == true) {
        process.setProgram(getCommandString(m_command));
    } else {
        process.setProgram(m_winePath);
    }
    process.setArguments(m_arguments);
    process.setProcessChannelMode(QProcess::MergedChannels);

    QObject::connect(&process, &QProcess::readyReadStandardOutput, [&]() {
        QByteArray output = process.readAllStandardOutput();
        qDebug().noquote() << QString::fromUtf8(output);
    });

    process.start();

    if (!process.waitForStarted()) {
        qWarning() << "Process failed to start!";
        qWarning() << "Error: " << process.errorString();
        m_status = 1;
        return;
    }

    while (process.state() != QProcess::NotRunning) {
        process.waitForReadyRead(50);
    }

    int exitCode = process.exitCode();
    m_status = (exitCode == 0) ? 0 : 1;

    qDebug() << "[Runner] Finished with code:" << exitCode;

    m_isRunning = false;
}
