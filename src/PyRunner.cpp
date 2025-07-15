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
#include <Python.h>
#include <vector>
#include <string>
#include <iostream>
#include <filesystem>
#include <cstdio>

namespace fs = std::filesystem;

PyRunner::PyRunner() : m_status(0) {
    Py_Initialize();
}

PyRunner::PyRunner(const std::string& cwd) : m_cwd(cwd), m_status(0) {
    Py_Initialize();
}

PyRunner::~PyRunner() {
    Py_Finalize();
}

bool PyRunner::setUpCamp(const std::string& level) {
    if (!fs::exists(level)) return false;
    m_cwd = level;
    return true;
}

void PyRunner::run(const std::string& script,
        const std::vector<std::string>& args) {
    if (!Py_IsInitialized()) {
        std::cerr << "Python not initialized!" << std::endl;
        m_status = 1;
        return;
    }

    std::cerr << "[PyRunner] Executing script file: " << script << std::endl;

    // Add working dir to sys.path
    if (!m_cwd.empty()) {
        PyObject *sysPath = PySys_GetObject("path");
        PyObject *cwdPath = PyUnicode_FromString(m_cwd.c_str());
        PyList_Insert(sysPath, 0, cwdPath);
        Py_DECREF(cwdPath);
    }

    // Set sys.argv
    PyObject *argvList = PyList_New(args.size());
    for (size_t i = 0; i < args.size(); ++i) {
        PyObject *arg = PyUnicode_FromString(args[i].c_str());
        PyList_SET_ITEM(argvList, i, arg);  // steals ref
    }
    PySys_SetObject("argv", argvList);
    Py_DECREF(argvList);

    std::string fullpath =
        m_cwd.empty() ? script : (m_cwd + "/" + script + ".py");
    FILE* fp = fopen(fullpath.c_str(), "r");
    if (!fp) {
        std::cerr << "[PyRunner] Failed to open script file: "
            << fullpath << std::endl;
        m_status = 1;
        return;
    }

    int result = PyRun_SimpleFile(fp, fullpath.c_str());
    fclose(fp);

    if (result != 0) {
        std::cerr << "[PyRunner] Python script returned error.\n";
        m_status = 1;
    } else {
        std::cerr << "[PyRunner] Script executed successfully.\n";
        m_status = 0;
    }
}

int64_t PyRunner::updateLevel(int64_t lid) {
    run("tombll_manage_data",
            {"tombll_manage_data.py", "-u", std::to_string(lid)});
    return m_status;
}

int64_t PyRunner::syncCards(int64_t lid) {
    run("tombll_manage_data", {"tombll_manage_data.py", "-sc"});
    return m_status;
}

int64_t PyRunner::getStatus() const {
    return m_status;
}

