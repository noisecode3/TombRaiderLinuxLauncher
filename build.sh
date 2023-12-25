#!/bin/bash
cd $(dirname $0)
rm -fr build
mkdir build
cd build
cmake ..
make
