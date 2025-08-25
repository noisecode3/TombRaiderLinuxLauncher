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

#ifndef SRC_COMMANDLINEPARSER_HPP_
#define SRC_COMMANDLINEPARSER_HPP_

#include <QCommandLineParser>

struct StartupSetting {
    quint8 class_id = 0;
    quint8 type_id = 0;
    quint8 difficulty_id = 0;
    quint8 duration_id = 0;
    quint8 sort_id = 0;
    bool installed = false;
    bool original = false;
    bool fullscreen = false;
};

/**
 * CommandLinParsere wraps around QCommandLineParser to keep main simple
 */
class CommandLineParser : public QObject {
    Q_OBJECT

 public:
    CommandLineParser(const QString& type);
    StartupSetting process(const QStringList& arguments);

 private:
    QCommandLineParser m_parser;

    // Constant data ----------------------------------------------------------
    // Don't change the order, index is an id number --------------------------

    const QStringList m_class_options = {
        "AlienSpace", "Atlantis", "BaseLab", "Cambodia", "Castle",
        "CaveCat", "City", "Coastal", "ColdSnowy", "Desert", "Easter",
        "Egypt", "FantasySurreal", "Home", "Ireland", "Joke", "Jungle",
        "Kids", "Library", "MysteryHorror", "nc", "NordicCeltic", "Oriental",
        "Persia", "Pirates", "Remake", "RomeGreece", "Ship", "Shooter",
        "South America", "South Pacific", "Steampunk", "Train", "Venice",
        "WildWest", "Xmas", "YoungLara"
    };

    const QStringList m_type_options = {
        "TR1", "TR2", "TR3", "TR4", "TR5", "TEN"
    };

    const QStringList m_difficulty_options = {
        "easy", "medium", "challenging", "very-challenging"
    };

    const QStringList m_duration_options = {
        "short", "medium", "long", "very-long"
    };
};

#endif  // SRC_COMMANDLINEPARSER_HPP_
