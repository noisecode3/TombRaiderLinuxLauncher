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
#include "../src/Path.hpp"
#include <QDebug>
#include <QFile>
#include <QProcess>

void PyRunner::run(const QString& script, const QVector<QString>& args) {
    if (m_isRunning) {
        qWarning() << "[PyRunner] Already running!";
        return;
    }
    m_isRunning = true;
    QProcess process;
    QString program = "python3";

    // Build full path to script
    Path path = Path(Path::resource);
    process.setWorkingDirectory(path.get());
    path << QString("%1.py").arg(script);

    QStringList arguments;
    arguments << path.get();
    for (const QString& arg : args) {
        arguments << arg;
    }

    process.setProcessChannelMode(QProcess::MergedChannels);

    QObject::connect(&process, &QProcess::readyReadStandardOutput, [&]() {
        QByteArray output = process.readAllStandardOutput();
        qDebug().noquote() << QString::fromUtf8(output);
    });

    qDebug() << "[PyRunner] Starting Python script:" << path.get();
    process.start(program, arguments);

    if (!process.waitForStarted()) {
        qWarning() << "[PyRunner] Failed to start Python process.";
        m_status = 1;
        return;
    }

    while (process.state() != QProcess::NotRunning) {
        process.waitForReadyRead(50);
    }

    int exitCode = process.exitCode();
    m_status = (exitCode == 0) ? 0 : 1;

    qDebug() << "[PyRunner] Script finished with code:" << exitCode;

    m_isRunning = false;
}

qint64 PyRunner::updateLevel(qint64 lid) {
    run("tombll_manage_data", {"-u", QString::number(lid)});
    return m_status;
}

qint64 PyRunner::syncCards() {
    run("tombll_manage_data", {"-sc"});
    return m_status;
}

qint64 PyRunner::getStatus() const {
    return m_status;
}
