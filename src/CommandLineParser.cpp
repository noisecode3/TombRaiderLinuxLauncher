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
#include <QDebug>

CommandLineParser::CommandLineParser(const QString& type) {
    if (type == "APP") {
        m_parser.setApplicationDescription("Tomb Raider Linux Launcher");
        m_parser.addHelpOption();

        m_parser.addOption(QCommandLineOption(
            QStringList{"so", "showOriginal"}, "Show old Core Design list"));

        m_parser.addOption(QCommandLineOption(
            QStringList{"si", "showInstalled"}, "Show only installed list"));

        // Class
        const QString classDesc =
            "Filter levels by class. Allowed values:\n    "
            + m_class_options.join(", ");

        m_parser.addOption(QCommandLineOption(
            QStringList{"fc", "filterByClass"}, classDesc, "class"));


        // Type
        const QString typeDesc =
            "Filter levels by TR game type. Allowed values:    "
            + m_type_options.join(", ");

        m_parser.addOption(QCommandLineOption(
            QStringList{"ft", "filterByType"}, typeDesc, "type"));


        // Difficulty
        const QString difficultyDesc =
            "Filter levels by difficulty. Allowed values:    "
            + m_type_options.join(", ");

        m_parser.addOption(QCommandLineOption(
            QStringList{"fd", "filterByDifficulty"}, difficultyDesc, "difficulty"));


        // Duration
        const QString durationDesc =
            "Filter levels by duration. Allowed values:    "
            + m_type_options.join(", ");

        m_parser.addOption(QCommandLineOption(
            QStringList{"fr", "filterByDuration"}, durationDesc, "duration"));


        // Sort
        m_parser.addOption(QCommandLineOption(
            QStringList{"sortByTitle"}, "Sort the list by title"));

        m_parser.addOption(QCommandLineOption(
            QStringList{"sortByDifficulty"}, "Sort the list by difficulty"));

        m_parser.addOption(QCommandLineOption(
            QStringList{"sortByDuration"}, "Sort the list by duration"));

        m_parser.addOption(QCommandLineOption(
            QStringList{"sortByClass"}, "Sort the list by class"));

        m_parser.addOption(QCommandLineOption(
            QStringList{"sortByType"}, "Sort the list by type"));

        m_parser.addOption(QCommandLineOption(
            QStringList{"sortByReleaseDate"}, "Sort the list by release date"));

    } else if (type == "TEST") {
        m_parser.setApplicationDescription("Tomb Raider Linux Launcher Test Suite");
        m_parser.addHelpOption();

        m_parser.addOption(QCommandLineOption(
            QStringList {"w", "widescreen"},
            "Set widescreen bit on original games, probably not useful for TRLE",
            "PATH"));

        m_parser.addOption(QCommandLineOption(
            QStringList {"b", "binary"},
            "Print PE Header Information, to record Tomb Raider and TRLE binaries",
            "PATH"));
    } else {
        qDebug() << "CommandLineParser: Internal object error";
    }
}

StartupSetting CommandLineParser::process(const QStringList& arguments) {
    m_parser.process(arguments);

    StartupSetting settings;

    if (m_parser.isSet("showInstalled")) {
        settings.installed = true;
    }

    if (m_parser.isSet("showOriginal")) {
        settings.original = true;
    }

    // --- Class ---
    if (m_parser.isSet("filterByClass")) {
        QString value = m_parser.value("filterByClass");
        int idx = m_class_options.indexOf(value, Qt::CaseInsensitive);
        if (idx != -1) {
            settings.class_id = static_cast<quint8>(idx+1);
        } else {
            qCritical().noquote() << "Invalid class value:" << value
                                  << "\nAllowed:" << m_class_options.join(", ");
            ::exit(1);
        }
    }

    // --- Type ---
    if (m_parser.isSet("filterByType")) {
        QString value = m_parser.value("filterByType");
        int idx = m_type_options.indexOf(value, Qt::CaseInsensitive);
        if (idx != -1) {
            settings.type_id = static_cast<quint8>(idx);
        } else {
            qCritical().noquote() << "Invalid type value:" << value
                                  << "\nAllowed:" << m_type_options.join(", ");
            ::exit(1);
        }
    }

    // --- Difficulty ---
    if (m_parser.isSet("filterByDifficulty")) {
        QString value = m_parser.value("filterByDifficulty");
        int idx = m_difficulty_options.indexOf(value, Qt::CaseInsensitive);
        if (idx != -1) {
            settings.difficulty_id = static_cast<quint8>(idx);
        } else {
            qCritical().noquote() << "Invalid difficulty value:" << value
                                  << "\nAllowed:" << m_difficulty_options.join(", ");
            ::exit(1);
        }
    }

    // --- Duration ---
    if (m_parser.isSet("filterByDuration")) {
        QString value = m_parser.value("filterByDuration");
        int idx = m_duration_options.indexOf(value, Qt::CaseInsensitive);
        if (idx != -1) {
            settings.duration_id = static_cast<quint8>(idx);
        } else {
            qCritical().noquote() << "Invalid duration value:" << value
                                  << "\nAllowed:" << m_duration_options.join(", ");
            ::exit(1);
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
        if (m_parser.isSet(opt)) {
            setSorts << opt;
        }
    }

    if (setSorts.size() > 1) {
        qCritical().noquote()
            << "Only one sort option may be specified at a time.\n"
            << "You provided:" << setSorts.join(", ");
        ::exit(1);
    }

    if (setSorts.size() == 1) {
        qDebug() << "Sorting by:" << setSorts.first();
        settings.sort_id =
            static_cast<quint8>(sortOptions.indexOf(setSorts.first()));

    }

    return settings;
}
