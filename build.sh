#!/bin/bash
cd "$(dirname "$0")" || exit 1
rm -fr build
mkdir build
cd build || exit 1
cmake -DCMAKE_BUILD_TYPE=Debug ..
make
