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

    // Merge stderr into stdout
    m_process.setProcessChannelMode(QProcess::MergedChannels);

    // Connect single output handler
    QObject::connect(&m_process, &QProcess::readyReadStandardOutput,
                     this,       &Runner::handleOutput);


    // Connect to clean up and singnal View function
    QObject::connect(
            &m_process,
            QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this,
            &Runner::onProcessFinished);
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
    m_process.setProcessEnvironment(m_env);
    m_process.setProgram(m_command);

    if (m_setupFlag) {
        m_arguments << "-setup";
    }
    m_process.setArguments(m_arguments);

    m_process.start();
    if (!m_process.waitForStarted()) {
        qWarning() << "Process failed to start!";
        qWarning() << "Error: " << m_process.errorString();
        m_status = 1;
    }
}

void Runner::onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus) {
    if (exitStatus == QProcess::NormalExit) {
        qDebug() << "Process exited normally with code:" << exitCode;
        m_status = exitCode;
    } else {
        qWarning() << "Process crashed or was killed!";
        m_status = 1;
    }

    m_env.clear();
    m_env.insert(QProcessEnvironment::systemEnvironment());

    m_arguments.clear();
    m_setupFlag = false;
}

void Runner::handleOutput() {
    QByteArray output = m_process.readAllStandardOutput();
    QTextStream outStream(stdout);
    outStream << output;
}
