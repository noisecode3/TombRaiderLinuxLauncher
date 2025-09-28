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

#include "../src/CommandLineParser.hpp"
#include <QCommandLineOption>
#include <QDebug>

CommandLineParser::CommandLineParser(const QString& type) : m_processStatus(0) {
    if (type == "APP") {
        m_parser.setApplicationDescription("Tomb Raider Linux Launcher");
        m_parser.addHelpOption();

        // Fullscreen ------------
        QStringList fullscreenName;
        fullscreenName << "f" << "fullscreen";
        QCommandLineOption fullscreenOption(fullscreenName);
        fullscreenOption.setDescription("Start in fullscreen");
        m_parser.addOption(fullscreenOption);

        // Original ------------
        QStringList showOriginalName;
        showOriginalName << "so" << "showOriginal";
        QCommandLineOption showOriginalOption(showOriginalName);
        showOriginalOption.setDescription("Show old Core Design list");
        m_parser.addOption(showOriginalOption);

        // Installed -----------
        QStringList showInstalledName;
        showInstalledName << "si" << "showInstalled";
        QCommandLineOption showInstalledOption(showInstalledName);
        showInstalledOption.setDescription("Show only installed list");
        m_parser.addOption(showInstalledOption);

        // Class ---------------
        QStringList filterByClassName;
        filterByClassName << "fc" << "filterByClass";
        QCommandLineOption filterByClassOption(filterByClassName);
        const QString filterByClassDesc = QString("%1%2%3").arg(
            "\nFilter levels by class. Allowed values:\n",
            m_class_options.join(", "),
            "\n");
        filterByClassOption.setDescription(filterByClassDesc);
        filterByClassOption.setValueName("class");
        m_parser.addOption(filterByClassOption);

        // Type ----------------
        QStringList filterByTypeName;
        filterByTypeName << "ft" << "filterByType";
        QCommandLineOption filterByTypeOption(filterByTypeName);
        const QString filterByTypeDesc = QString("%1%2%3").arg(
            "Filter levels by TR game type. Allowed values:\n",
            m_type_options.join(", "),
            "\n");
        filterByTypeOption.setDescription(filterByTypeDesc);
        filterByTypeOption.setValueName("type");
        m_parser.addOption(filterByTypeOption);

        // Difficulty ----------
        QStringList filterByDifficultyName;
        filterByDifficultyName << "fd" << "filterByDifficulty";
        QCommandLineOption filterByDifficultyOption(filterByDifficultyName);
        const QString filterByDifficultyDesc = QString("%1%2%3").arg(
            "Filter levels by difficulty. Allowed values:\n",
            m_difficulty_options.join(", "),
            "\n");
        filterByDifficultyOption.setDescription(filterByDifficultyDesc);
        filterByDifficultyOption.setValueName("difficulty");
        m_parser.addOption(filterByDifficultyOption);

        // Duration -----------
        QStringList filterByDurationName;
        filterByDurationName << "fr" << "filterByDuration";
        QCommandLineOption filterByDurationOption(filterByDurationName);

        const QString filterByDurationDesc = QString("%1%2%3").arg(
            "Filter levels by duration. Allowed values:\n",
            m_duration_options.join(", "),
            "\n");
        filterByDurationOption.setDescription(filterByDurationDesc);
        filterByDurationOption.setValueName("duration");
        m_parser.addOption(filterByDurationOption);

        // Sort
        QCommandLineOption sortByTitleOption("sortByTitle");
        sortByTitleOption.setDescription("Sort the list by title");
        m_parser.addOption(sortByTitleOption);

        QCommandLineOption sortByDifficultyOption("sortByDifficulty");
        sortByDifficultyOption.setDescription("Sort the list by difficulty");
        m_parser.addOption(sortByDifficultyOption);

        QCommandLineOption sortByDurationOption("sortByDuration");
        sortByDurationOption.setDescription("Sort the list by duration");
        m_parser.addOption(sortByDurationOption);

        QCommandLineOption sortByClassOption("sortByClass");
        sortByClassOption.setDescription("Sort the list by class");
        m_parser.addOption(sortByClassOption);

        QCommandLineOption sortByTypeOption("sortByType");
        sortByTypeOption.setDescription("Sort the list by type");
        m_parser.addOption(sortByTypeOption);

        QCommandLineOption sortByReleaseDateOption("sortByReleaseDate");
        sortByReleaseDateOption.setDescription("Sort the list by release date");
        m_parser.addOption(sortByReleaseDateOption);

    } else if (type == "TEST") {
        m_parser.setApplicationDescription("Tomb Raider Linux Launcher Test Suite");
        m_parser.addHelpOption();

        QStringList widescreenName;
        widescreenName << "w" << "widescreen";
        QCommandLineOption widescreenOption(widescreenName);
        const QString widescreenDesc = QString("%1%2").arg(
            "Set widescreen bit on original games,",
            " probably not useful for TRLE");
        widescreenOption.setDescription(widescreenDesc);
        widescreenOption.setDefaultValue("PATH");
        m_parser.addOption(widescreenOption);

        QStringList binaryName;
        binaryName << "b" << "binary";
        QCommandLineOption binaryOption(binaryName);
        const QString binaryDesc = QString("%1%2").arg(
            "Print PE Header Information,",
            " to record Tomb Raider and TRLE binaries");
        binaryOption.setDescription(binaryDesc);
        binaryOption.setDefaultValue("PATH");
        m_parser.addOption(binaryOption);

    } else {
        qDebug() << "CommandLineParser: Internal object error";
    }
}

StartupSetting CommandLineParser::process(const QStringList& arguments) {
    m_parser.process(arguments);

    StartupSetting settings;
    
    if (m_parser.isSet("fullscreen") == true) {
        settings.fullscreen = true;
    }

    if (m_parser.isSet("showInstalled") == true) {
        settings.installed = true;
    }

    if (m_parser.isSet("showOriginal") == true) {
        settings.original = true;
    }

    // --- Class ---
    if (m_parser.isSet("filterByClass") == true) {
        QString value = m_parser.value("filterByClass");
        int idx = m_class_options.indexOf(value, Qt::CaseInsensitive);
        if (idx != -1) {
            settings.class_id = static_cast<quint8>(idx + 1);
        } else {
            qCritical().noquote() << "Invalid class value:" << value
                                  << "\nAllowed:" << m_class_options.join(", ");
            m_processStatus = 1;
        }
    }

    // --- Type ---
    if (m_parser.isSet("filterByType") == true) {
        QString value = m_parser.value("filterByType");
        int idx = m_type_options.indexOf(value, Qt::CaseInsensitive);
        if (idx != -1) {
            settings.type_id = static_cast<quint8>(idx + 1);
        } else {
            qCritical().noquote() << "Invalid type value:" << value
                                  << "\nAllowed:" << m_type_options.join(", ");
            m_processStatus = 1;
        }
    }

    // --- Difficulty ---
    if (m_parser.isSet("filterByDifficulty") == true) {
        QString value = m_parser.value("filterByDifficulty");
        int idx = m_difficulty_options.indexOf(value, Qt::CaseInsensitive);
        if (idx != -1) {
            settings.difficulty_id = static_cast<quint8>(idx + 1);
        } else {
            qCritical().noquote() << "Invalid difficulty value:" << value
                                  << "\nAllowed:" << m_difficulty_options.join(", ");
            m_processStatus = 1;
        }
    }

    // --- Duration ---
    if (m_parser.isSet("filterByDuration") == true) {
        QString value = m_parser.value("filterByDuration");
        int idx = m_duration_options.indexOf(value, Qt::CaseInsensitive);
        if (idx != -1) {
            settings.duration_id = static_cast<quint8>(idx + 1);
        } else {
            qCritical().noquote() << "Invalid duration value:" << value
                                  << "\nAllowed:" << m_duration_options.join(", ");
            m_processStatus = 1;
        }
    }

    // Collect all mutually exclusive sort options
    QStringList sortOptions = {
        "sortByReleaseDate",
        "sortByTitle",
        "sortByDifficulty",
        "sortByDuration",
        "sortByClass",
        "sortByType"
    };

    QStringList setSorts;
    for (const QString &opt : sortOptions) {
        if (m_parser.isSet(opt) == true) {
            setSorts << opt;
        }
    }

    if (setSorts.size() > 1) {
        qCritical().noquote()
            << "Only one sort option may be specified at a time.\n"
            << "You provided:" << setSorts.join(", ");
        m_processStatus = 1;
    }

    if (setSorts.size() == 1) {
        qDebug() << "Sorting by:" << setSorts.first();
        settings.sort_id =
            static_cast<quint8>(sortOptions.indexOf(setSorts.first()));

    }

    return settings;
}

quint64 CommandLineParser::getProcessStatus() {
    return m_processStatus;
}
