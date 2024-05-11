python
#https://invent.kde.org/kdevelop/kdevelop/-/tree/master/plugins/gdb/printers
#set auto-load safe-path /
# put that into ~/.config/gdb/gdbinit or the path to this src
# if a thread die it will ask what thread you want to run
# and the python script built into gdb will crash.
# its still better to debug qt threads with qtcreator
import sys, os

sys.path.insert(0, "./.gdb/qt5prettyprinters")

from qt import register_qt_printers
register_qt_printers (None)

end
