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

#include "../src/Runner.hpp"
#include <QDebug>
#include <QTextStream>
#include <QObject>

Runner::Runner() : m_env(QProcessEnvironment::systemEnvironment()) {
    m_status = 0;
    m_setupFlag = false;
    m_isRunning = false;
}

Runner::Runner(const QString& cmd)
    : m_env(QProcessEnvironment::systemEnvironment()), m_command(cmd) {
    m_status = 0;
    m_setupFlag = false;
    m_isRunning = false;
}

void Runner::insertArguments(const QStringList& value) {
    m_arguments << value;
}

void Runner::clearArguments() {
    m_arguments.clear();
}

void Runner::insertProcessEnvironment(const QPair<QString, QString> env) {
    m_env.insert(env.first, env.second);
}

void Runner::clearProcessEnvironment() {
    m_env.clear();
}

void Runner::setWorkingDirectory(const QString& cwd) {
    m_cwd = cwd;
}

void Runner::setupFlag(bool setup) {
    m_setupFlag = setup;
}

void Runner::run() {
    if (m_isRunning) {
        qWarning() << "[Runner] Already running!";
        return;
    }
    m_isRunning = true;
    QProcess process;
    process.setWorkingDirectory(m_cwd);
    process.setProcessEnvironment(m_env);
    process.setProgram(m_command);

    if (m_setupFlag) {
        process.setArguments(QStringList() << m_arguments << "-setup");
    } else {
        process.setArguments(m_arguments);
    }

    process.start();

    process.setProcessChannelMode(QProcess::MergedChannels);

    QObject::connect(&process, &QProcess::readyReadStandardOutput, [&]() {
        QByteArray output = process.readAllStandardOutput();
        qDebug().noquote() << QString::fromUtf8(output);
    });

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
