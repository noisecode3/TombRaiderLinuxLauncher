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

#ifndef SRC_STATICDATA_HPP_
#define SRC_STATICDATA_HPP_
#include <QString>
#include <QMap>
#include <unordered_map>

struct OriginalGameStaticData {
    QMap<qint64, QString> romanNumerals = {
            {0, "null"},
            {1, "I"},
            {2, "II"},
            {3, "III"},
            {4, "IV"},
            {5, "V"},
            {6, "VI"},
            {7, "I_UB"},
            {8, "II_GM"},
            {9, "III_LM"},
            {10, "IV_TT"},
    };

    const QString getCoverJPEG(qint64 id) {
        return QString ("Tomb_Raider_%1.jpg").arg(romanNumerals[id]);
    }

    QMap<qint64, qint64> type = {
            {0, 0},
            {1, 1},
            {2, 2},
            {3, 3},
            {4, 4},
            {5, 5},
            {6, 6},
            {7, 1},
            {8, 2},
            {9, 3},
            {10, 4},
    };

    const qint64 getType(qint64 id) {
        return type[id];
    }

    QMap<qint64, QString> release = {
            {0, "null"},
            {1, "1996-11-14"},
            {2, "1997-11-21"},
            {3, "1998-11-20"},
            {4, "1999-11-24"},
            {5, "2000-11-17"},
            {6, "2003-07-01"},
            {7, "1998-12-31"},
            {8, "1999-06-04"},
            {9, "2000-03-00"},
            {10, "1999-12-00"},
    };

    const QString getRelease(qint64 id) {
        return release[id];
    }

    QMap<qint64, QString> shortBody = {
            {0, QString("%1%2%3").arg(
                "",
                "",
                "")
            },
            {1, ""},
            {2, ""},
            {3, ""},
            {4, ""},
            {5, ""},
            {6, ""},
            {7, ""},
            {8, ""},
            {9, ""},
            {10, ""},
    };

    const QString getShortBody(qint64 id) {
        return shortBody[id];
    }
};

struct StaticData {
 private:
    // Difficulty
    static constexpr std::pair<qint64, const char*> difficultyList[] = {
        {0, "null"}, {1, "easy"}, {2, "medium"}, {3, "challenging"}, {4, "very challenging"}
    };

    // Class
    static constexpr std::pair<qint64, const char*> classList[] = {
        {0, "null"}, {1, "Alien/Space"}, {2, "Atlantis"}, {3, "Base/Lab"}, {4, "Cambodia"},
        {5, "Castle"}, {6, "Cave/Cat"}, {7, "City"}, {8, "Coastal"}, {9, "Cold/Snowy"},
        {10, "Desert"}, {11, "Easter"}, {12, "Egypt"}, {13, "Fantasy/Surreal"}, {14, "Home"},
        {15, "Ireland"}, {16, "Joke"}, {17, "Jungle"}, {18, "Kids"}, {19, "Library"},
        {20, "Mystery/Horror"}, {21, "nc"}, {22, "Nordic/Celtic"}, {23, "Oriental"},
        {24, "Persia"}, {25, "Pirates"}, {26, "Remake"}, {27, "Rome/Greece"}, {28, "Ship"},
        {29, "Shooter"}, {30, "South America"}, {31, "South Pacific"}, {32, "Steampunk"},
        {33, "Train"}, {34, "Venice"}, {35, "Wild West"}, {36, "Xmas"}, {37, "Young Lara"}
    };

    // Duration
    static constexpr std::pair<qint64, const char*> durationList[] = {
        {0, "null"}, {1, "short"}, {2, "medium"}, {3, "long"}, {4, "very long"}
    };

    // Type
    static constexpr std::pair<qint64, const char*> typeList[] = {
        {0, "null"}, {1, "TR1"}, {2, "TR2"}, {3, "TR3"}, {4, "TR4"}, {5, "TR5"}, {6, "TEN"}
    };

    template <size_t N>
    static std::unordered_map<qint64, QString> buildMap(const std::pair<qint64, const char*> (&arr)[N]) {
        std::unordered_map<qint64, QString> m;
        for (const auto &p : arr) m[p.first] = QString::fromUtf8(p.second);
        return m;
    }

    template <size_t N>
    static std::unordered_map<QString, qint64> buildReverseMap(const std::pair<qint64, const char*> (&arr)[N]) {
        std::unordered_map<QString, qint64> m;
        for (const auto &p : arr) m[QString::fromUtf8(p.second)] = p.first;
        return m;
    }

 public:
    // Difficulty
    static const std::unordered_map<qint64, QString>& getDifficulty() {
        static const auto m = buildMap(difficultyList);
        return m;
    }
    static const std::unordered_map<QString, qint64>& getDifficultyID() {
        static const auto m = buildReverseMap(difficultyList);
        return m;
    }

    // Class
    static const std::unordered_map<qint64, QString>& getClass() {
        static const auto m = buildMap(classList);
        return m;
    }
    static const std::unordered_map<QString, qint64>& getClassID() {
        static const auto m = buildReverseMap(classList);
        return m;
    }

    // Duration
    static const std::unordered_map<qint64, QString>& getDuration() {
        static const auto m = buildMap(durationList);
        return m;
    }
    static const std::unordered_map<QString, qint64>& getDurationID() {
        static const auto m = buildReverseMap(durationList);
        return m;
    }

    // Type
    static const std::unordered_map<qint64, QString>& getType() {
        static const auto m = buildMap(typeList);
        return m;
    }
    static const std::unordered_map<QString, qint64>& getTypeID() {
        static const auto m = buildReverseMap(typeList);
        return m;
    }
};

#endif  // SRC_STATICDATA_HPP_
