#!/bin/bash
cd "$(dirname "$0")" || exit 1

rm -fr build
mkdir build
cd build || exit 1

cmake \
    -DCMAKE_INSTALL_PREFIX=/home/"$USER"/.local \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=1 \
    -DNO_DATABASE=ON \
    -DCMAKE_BUILD_TYPE=Debug \
    "$@" ..

cd ..

git config submodule.libs/miniz.ignore all

./build.sh
