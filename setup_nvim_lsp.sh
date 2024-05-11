#!/bin/bash
cd $(dirname $0)
# don't forget that you need to build it one time
# and look at .neovim how to use debuger
cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=1 -DCMAKE_BUILD_TYPE=Debug .
