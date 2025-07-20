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
#include <QDebug>
#include <QString>
#include <QVector>
#include <QFile>
#include <cstdio>

#undef slots
#undef signals
#include <Python.h>
PyThreadState *g_mainState = nullptr;

PyRunner::PyRunner() : m_status(0) {
    Py_Initialize();
    g_mainState = PyEval_SaveThread();
}

PyRunner::PyRunner(const QString& cwd) : m_cwd(cwd), m_status(0) {
    Py_Initialize();
    g_mainState = PyEval_SaveThread();
}

PyRunner::~PyRunner() {
    PyEval_RestoreThread(g_mainState);
    Py_FinalizeEx();
}

bool PyRunner::setUpCamp(const QString& level) {
    bool status = true;
    if (!QFile::exists(level)) {
        status = false;
    }
    m_cwd = level;
    return status;
}

void PyRunner::run(const QString& script,
        const QVector<QString>& args) {
    if (!Py_IsInitialized()) {
        qDebug() << "Python not initialized!";
        m_status = 1;
        return;
    }

    PyGILState_STATE gil = PyGILState_Ensure();
    qDebug() << "[PyRunner] Executing script file: " << script << Qt::endl;

    // Add working dir to sys.path
    if (!m_cwd.isEmpty()) {
        PyObject *sysPath = PySys_GetObject("path");
        PyObject *cwdPath = PyUnicode_FromString(m_cwd.toStdString().c_str());
        PyList_Insert(sysPath, 0, cwdPath);
        Py_DECREF(cwdPath);
    }

    // Set sys.argv
    PyObject *argvList = PyList_New(args.size());
    for (size_t i = 0; i < args.size(); ++i) {
        PyObject *arg = PyUnicode_FromString(args[i].toStdString().c_str());
        PyList_SET_ITEM(argvList, i, arg);  // steals ref
    }
    PySys_SetObject("argv", argvList);
    Py_DECREF(argvList);

    QString fullpath =
        m_cwd.isEmpty() ? script : (m_cwd + "/" + script + ".py");
    FILE* fp = fopen(fullpath.toStdString().c_str(), "r");
    if (!fp) {
        qDebug() << "[PyRunner] Failed to open script file: "
            << fullpath << Qt::endl;
        m_status = 1;
        return;
    }

    int result = PyRun_SimpleFile(fp, fullpath.toStdString().c_str());
    fclose(fp);
    PyGILState_Release(gil);

    if (result != 0) {
        qDebug() << "[PyRunner] Python script returned error.\n";
        m_status = 1;
    } else {
        qDebug() << "[PyRunner] Script executed successfully.\n";
        m_status = 0;
    }
}

qint64 PyRunner::updateLevel(qint64 lid) {
    run("tombll_manage_data",
            {"tombll_manage_data.py", "-u", QString("%1").arg(lid)});
    return m_status;
}

qint64 PyRunner::syncCards() {
    run("tombll_manage_data", {"tombll_manage_data.py", "-sc"});
    return m_status;
}

qint64 PyRunner::getStatus() const {
    return m_status;
}

