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
    m_status(0),
    m_process(nullptr),
    m_isRunning(false)
{}

Runner::~Runner() {
    delete m_process;
}

const quint64 Runner::getStatus() {
    return m_status;
}

inline bool killCondition(qint64 command) {
    bool status = false;
    if (command != 3) {
        status = true;
    }
    return status;
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

void Runner::stop()
{
    if (!m_process || m_process->state() == QProcess::NotRunning)
        return;

    m_process->terminate();
    if (!m_process->waitForFinished(2000)) {
        m_process->kill();
    }
}

void Runner::run() {
    if (m_isRunning) {
        qWarning() << "[Runner] Already running!";
        return;
    }
    Q_ASSERT_WITH_TRACE(!m_cwd.isEmpty());

    m_isRunning = true;
    delete m_process;
    m_process = new QProcess();

    m_process->setWorkingDirectory(m_cwd);
    m_process->setProcessEnvironment(m_env);
    if (m_winePath.isEmpty() == true) {
        m_process->setProgram(getCommandString(m_command));
    } else {
        m_process->setProgram(m_winePath);
    }
    m_process->setArguments(m_arguments);
    m_process->setProcessChannelMode(QProcess::MergedChannels);

    connect(m_process, &QProcess::readyReadStandardOutput, [&]() {
        QByteArray output = m_process->readAllStandardOutput();
        qDebug().noquote() << QString::fromUtf8(output);
    });

    m_process->setChildProcessModifier([]() {
        ::setsid();
    });

    connect(m_process,
            QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, [this](int code, QProcess::ExitStatus) {
                m_status = (code == 0) ? 0 : 1;
                m_isRunning = false;
                emit runningDone();
            });

    m_process->start();
}
