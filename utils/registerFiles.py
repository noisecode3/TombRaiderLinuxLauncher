import os
import hashlib

home_directory = os.path.expanduser("~")
directory   = home_directory + "/.steam/steam/steamapps/common/TombRaider (III)/"  # Replace with your directory path
output_file = "output.h"
path_to_cut = "~/.steam/steam/steamapps/common/TombRaider (III)/"

with open(output_file, "w") as f:
    f.write('#pragma once\n')
    f.write('#include "files.h"\n\n')
    f.write('gameFileList TR3FileList("TR3.Original");\n')
    f.write('void work()\n{\n')

    for root, dirs, files in os.walk(directory):
        for file in files:
            file_path = os.path.join(root, file)

            if file_path.startswith(directory):
                # Remove the specified path prefix
                relative_path = file_path[len(directory):]
            else:
                pass

            with open(file_path, "rb") as current_file:
                file_content = current_file.read()
                md5sum = hashlib.md5(file_content).hexdigest()

            f.write(f'   TR3FileList.addFile("{md5sum}", "{relative_path}");\n')

    f.write('}')
