#!/bin/bash
cd "$(dirname "$0")" || exit 1
# clean up command
rm -fr CMakeCache.txt CMakeFiles Makefile cmake_install.cmake \
        compile_commands.json TombRaiderLinuxLauncher \
        .cache TombRaiderLinuxLauncher_autogen \
        TombRaiderLinuxLauncherTest_autogen CTestTestfile.cmake
