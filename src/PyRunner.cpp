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


#include "../src/PyRunner.hpp"
#include <QFileInfo>

PyRunner::PyRunner(const QString& cwd)
        : m_env(QProcessEnvironment::systemEnvironment()) {
    m_status = 0;
    if (QFileInfo("/usr/bin/python3").exists()) {
        m_command = "/usr/bin/python3";
    } else if (QFileInfo("/usr/bin/python").exists()) {
        m_command = "/usr/bin/python";
    }
    m_process.setWorkingDirectory(cwd);
    m_process.setProcessEnvironment(m_env);
}

void PyRunner::run(const QStringList& arguments) {
    m_process.setProgram(m_command);
    m_process.setArguments(arguments);
    qDebug() << m_command << arguments;
    m_process.start();

    if (m_process.waitForStarted() == true) {
        m_process.waitForFinished();
        qDebug().noquote() << "\n"
            << m_process.readAllStandardOutput();
        qDebug().noquote()
            << m_process.readAllStandardError();
        qDebug().noquote() << "\nExit status :" << m_process.exitStatus();
        m_status = m_process.exitCode();
    } else {
        qWarning() << "Failed to start python process!";
    }
}

qint64 PyRunner::updateLevel(qint64 lid) {
    QStringList arguments;
    arguments << "tombll_manage_data.py" << "-u" << QString::number(lid);
    run(arguments);
    return getStatus();
}

qint64 PyRunner::syncCards(qint64 lid ) {
    QStringList arguments;
    arguments << "tombll_manage_data.py" << "-sc";
    run(arguments);
    return getStatus();
}

qint64 PyRunner::getStatus() {
    return m_status;
}
