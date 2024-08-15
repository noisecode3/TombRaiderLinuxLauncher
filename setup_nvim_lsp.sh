#!/bin/bash
cd "$(dirname "$0")"
# don't forget that you need to build it one time
# and look at .neovim how to use debuger
mkdir -p .gdb/qt5prettyprinters/

gdbinit=$(cat <<'EOINS'
python
import sys, os

sys.path.insert(0, "./.gdb/qt5prettyprinters")

from qt import register_qt_printers
register_qt_printers (None)

end
EOINS
)
echo "$gdbinit" > .gdbinit
chmod 0644 .gdbinit

wget -O .gdb/qt5prettyprinters/helper.py \
    https://invent.kde.org/kdevelop/kdevelop/-/raw/master/plugins/gdb/printers/helper.py
wget -O .gdb/qt5prettyprinters/qt.py \
    https://invent.kde.org/kdevelop/kdevelop/-/raw/master/plugins/gdb/printers/qt.py

cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=1 -DCMAKE_BUILD_TYPE=Debug .
