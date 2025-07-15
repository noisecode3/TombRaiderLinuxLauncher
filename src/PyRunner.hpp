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

#ifndef SRC_PYRUNNER_HPP_
#define SRC_PYRUNNER_HPP_

#include <string>
#include <vector>

class PyRunner {
 public:
    PyRunner();
    explicit PyRunner(const std::string& cwd);
    ~PyRunner();

    bool setUpCamp(const std::string& level);
    void run(const std::string& script, const std::vector<std::string>& args);
    int64_t updateLevel(int64_t lid);
    int64_t syncCards(int64_t lid);
    int64_t getStatus() const;

 private:
    std::string m_cwd;
    int64_t m_status;
};

#endif  // SRC_PYRUNNER_HPP_
