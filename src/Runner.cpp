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
}

Runner::Runner(const QString& cmd)
    : m_env(QProcessEnvironment::systemEnvironment()) {
    m_status = 0;
    m_command = cmd;
}

void Runner::insertArguments(const QStringList& value) {
    m_arguments << value;
}

void Runner::clearArguments() {
    m_arguments.clear();
}

void Runner::insertProcessEnvironment(
        const QString& name, const QString& value) {
    m_env.insert(name, value);
}

void Runner::clearProcessEnvironment() {
    m_env.clear();
}

void Runner::setWorkingDirectory(const QString& cwd) {
    m_process.setWorkingDirectory(cwd);
}

void Runner::run() {
    // Start Wine with the application as an argument
    m_process.setProcessEnvironment(m_env);
    m_process.start(m_command, m_arguments);
    QObject::connect(&m_process, &QProcess::readyReadStandardOutput, [&]() {
        // Read and print the output to standard output
        QTextStream(stdout) << m_process.readAllStandardOutput();
    });

    if (m_process.waitForStarted() == true) {
        m_process.waitForFinished();
        m_status = 0;
    } else {
        m_status = 1;
        qWarning() << "Failed to start Wine process!";
    }
}
