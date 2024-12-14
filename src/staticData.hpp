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

#ifndef SRC_STATICDATA_HPP_
#define SRC_STATICDATA_HPP_
#include <QString>
#include <unordered_map>

class StaticClass {
 public:
    static const std::unordered_map<qint64, QString>& getMap() {
        static const std::unordered_map<qint64, QString> idToString = {
            {1, "Alien/Space"},
            {2, "Atlantis"},
            {3, "Base/Lab"},
            {4, "Cambodia"},
            {5, "Castle"},
            {6, "Cave/Cat"},
            {7, "City"},
            {8, "Coastal"},
            {9, "Cold/Snowy"},
            {10, "Desert"},
            {11, "Easter"},
            {12, "Egypt"},
            {13, "Fantasy/Surreal"},
            {14, "Home"},
            {15, "Ireland"},
            {16, "Joke"},
            {17, "Jungle"},
            {18, "Kids"},
            {19, "Library"},
            {20, "Mystery/Horror"},
            {21, "nc"},
            {22, "Nordic/Celtic"},
            {23, "Oriental"},
            {24, "Persia"},
            {25, "Pirates"},
            {26, "Remake"},
            {27, "Rome/Greece"},
            {28, "Ship"},
            {29, "Shooter"},
            {30, "South America"},
            {31, "South Pacific"},
            {32, "Steampunk"},
            {33, "Train"},
            {34, "Venice"},
            {35, "Wild West"},
            {36, "Xmas"},
            {36, "Young Lara"}
        };
        return idToString;
    }
};

class StaticDifficulty {
 public:
    static const std::unordered_map<qint64, QString>& getMap() {
        static const std::unordered_map<qint64, QString> idToString = {
            {0, "null"},
            {1, "easy"},
            {2, "medium"},
            {3, "challenging"},
            {4, "very challenging"}
        };
        return idToString;
    }
};

class StaticType {
 public:
    static const std::unordered_map<qint64, QString>& getMap() {
        static const std::unordered_map<qint64, QString> idToString = {
            {1, "TR1"},
            {2, "TR2"},
            {3, "TR3"},
            {4, "TR4"},
            {5, "TR5"},
            {6, "TEN"}
        };
        return idToString;
    }
};

class StaticDuration {
 public:
    static const std::unordered_map<qint64, QString>& getMap() {
        static const std::unordered_map<qint64, QString> idToString = {
            {0, "null"},
            {1, "short"},
            {2, "medium"},
            {3, "long"},
            {4, "very long"}
        };
        return idToString;
    }
};

#endif  // SRC_STATICDATA_HPP_
