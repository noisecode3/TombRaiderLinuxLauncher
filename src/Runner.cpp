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
}

Runner::Runner(const QString& cmd)
    : m_env(QProcessEnvironment::systemEnvironment()), m_command(cmd) {
    m_status = 0;
    m_setupFlag = false;
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
    m_process.setWorkingDirectory(cwd);
}


void Runner::toggleSetupFlag() {
    m_setupFlag = !m_setupFlag;
}

void Runner::run() {
    int status = 1;

    m_process.setProcessEnvironment(m_env);
    m_process.setProgram(m_command);
    if (m_setupFlag) {
        m_arguments << "-setup";
    }
    m_process.setArguments(m_arguments);

    // Merge stderr into stdout
    m_process.setProcessChannelMode(QProcess::MergedChannels);

    // Connect single output handler
    QObject::connect(&m_process, &QProcess::readyReadStandardOutput,
                     this,       &Runner::handleOutput);

    m_process.start();
    bool hasStarted = m_process.waitForStarted();

    if (hasStarted) {
        bool finished = false;
        finished = m_process.waitForFinished();
        if (finished) {
            status = 0;
        }
    }

    if (status != 0) {
        qWarning() << "Failed to start or finish process!";
    }

    m_env.clear();
    m_arguments.clear();
    m_setupFlag = false;
    m_status = status;
}

void Runner::handleOutput() {
    QByteArray output = m_process.readAllStandardOutput();
    QTextStream outStream(stdout);
    outStream << output;
}
