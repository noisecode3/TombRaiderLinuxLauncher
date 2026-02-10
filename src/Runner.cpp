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
#include <sys/prctl.h>
#include <signal.h>
#include <unistd.h>
#include <QDebug>
#include <QTextStream>
#include <QObject>

Runner::Runner()
    : m_env(QProcessEnvironment::systemEnvironment()),
    m_command(0),
    m_status(0),
    m_process(nullptr),
    m_pgid(-1)
{}

Runner::~Runner()
{
    if (m_process &&
        (m_process->state() != QProcess::NotRunning) &&
        (m_pgid > 0))
    {
        ::kill(-m_pgid, SIGKILL);
        m_process->waitForFinished(1000);
    }
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

void Runner::stop()
{
    if (!m_process || m_process->state() == QProcess::NotRunning) {
        return;
    }
    if (m_pgid <= 0) {
        return;
    }
    ::kill(-m_pgid, SIGTERM);
    if (!m_process->waitForFinished(2000)) {
        ::kill(-m_pgid, SIGKILL);
        m_process->waitForFinished(1000);
    }

    m_pgid = -1;
}

void Runner::run()
{
    if ((m_process != nullptr) && (m_process->state() != QProcess::NotRunning)) {
        qWarning() << "[Runner] Already running!";
        return;
    }

    Q_ASSERT_WITH_TRACE(!m_cwd.isEmpty());

    m_pgid = -1;

    if (m_process) {
        m_process->deleteLater();
    }

    m_process = new QProcess(this);

    m_process->setWorkingDirectory(m_cwd);
    m_process->setProcessEnvironment(m_env);

    if (m_winePath.isEmpty())
        m_process->setProgram(getCommandString(m_command));
    else
        m_process->setProgram(m_winePath);

    m_process->setArguments(m_arguments);
    m_process->setProcessChannelMode(QProcess::MergedChannels);

    connect(m_process, &QProcess::readyReadStandardOutput, this, [this]() {
        qDebug().noquote() << QString::fromUtf8(m_process->readAllStandardOutput());
    });

    m_process->setChildProcessModifier([]() {
        ::setsid();
        ::prctl(PR_SET_PDEATHSIG, SIGTERM);
        if (::getppid() == 1) {
            ::kill(::getpid(), SIGTERM);
        }
        ::signal(SIGPIPE, SIG_DFL);
    });

    connect(m_process,
            QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this,
            [this](int code, QProcess::ExitStatus) {
                m_status = (code == 0) ? 0 : 1;
                m_pgid = -1;
                emit runningDone();
            });

    connect(m_process, &QProcess::errorOccurred,
            this, [this](QProcess::ProcessError err) {
                qWarning() << "Process error:" << err;
                m_pgid = -1;
            });

    m_process->start();

    if (!m_process->waitForStarted()) {
        m_pgid = -1;
        return;
    }

    m_pgid = m_process->processId();
}
