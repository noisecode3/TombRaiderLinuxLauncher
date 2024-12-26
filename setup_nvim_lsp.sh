#!/bin/bash
cd "$(dirname "$0")" || exit 1
# don't forget that you need to build it one time

rm -fr build
mkdir build
cd build || exit 1

cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=1 -DCMAKE_BUILD_TYPE=Debug "$@" ..
cd ..
ln -s build/compile_commands.json compile_commands.json

git config submodule.libs/miniz.ignore all
