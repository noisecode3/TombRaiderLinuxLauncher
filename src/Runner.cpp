/* TombRaiderLinuxLauncher
 * Martin Bångens Copyright (C) 2024
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include "Runner.hpp"

Runner::Runner() : m_env(QProcessEnvironment::systemEnvironment()) {
    m_status = 0;
    // You can modify the environment of the process if needed
    // m_env.insert("WINEPREFIX", "/path/to/wineprefix");
    // m_process.setProcessEnvironment(m_env);
}

Runner::Runner(const QString& cmd)
    : m_env(QProcessEnvironment::systemEnvironment()) {
    m_env.insert("WINEDLLOVERRIDES", "winmm=n,b;ddraw=n,b");
    m_env.insert("WINEFSYNC", "1");
    m_process.setProcessEnvironment(m_env);
    m_status = 0;
    m_command = cmd;
}

void Runner::setWorkingDirectory(const QString& cwd) {
    m_process.setWorkingDirectory(cwd);
}

void Runner::run() {
    // Start Wine with the application as an argument
    m_process.start(m_command, QStringList() << "tomb4.exe");
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
