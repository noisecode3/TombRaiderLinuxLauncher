"""
My mamma bakes em potatis
"""
import os
import hashlib

#DIRECTORY = os.path.expanduser("~") + "/.steam/steam/steamapps/common/TombRaider (III)/"
DIRECTORY = os.path.expanduser("~") + "/test-TR3"
OUTPUT = "output.h"

with open(OUTPUT, "w") as f:
    f.write('#pragma once\n')
    f.write('#include "files.h"\n\n')
    f.write('gameFileList TRLE3573FileList("TR3.3573");\n')
    f.write('void work()\n{\n')

    for root, dirs, files in os.walk(DIRECTORY):
        for file in files:
            file_path = os.path.join(root, file)

            if file_path.startswith(DIRECTORY):
                relative_path = file_path[len(DIRECTORY):]

            with open(file_path, "rb") as current_file:
                file_content = current_file.read()
                md5sum = hashlib.md5(file_content).hexdigest()

            f.write(f'   TRLE3573FileList.addFile("{md5sum}", "{relative_path}");\n')

    f.write('}')
