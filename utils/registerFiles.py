"""
My mamma bakes em potatis
"""
import os
import hashlib
import json
DIRECTORY = os.path.expanduser("~") + "/.local/share/TombRaiderLinuxLauncher/Original.TR3"
file_info = {}

for root, dirs, files in os.walk(DIRECTORY):
    for file in files:
        file_path = os.path.join(root, file)

        if file_path.startswith(DIRECTORY):
            relative_path = file_path[len(DIRECTORY):]

        with open(file_path, "rb") as current_file:
            file_content = current_file.read()
            md5sum = hashlib.md5(file_content).hexdigest()

        file_info[relative_path] = md5sum

# Write the dictionary to a JSON file
with open('file_info.json', 'w') as json_file:
    json.dump(file_info, json_file, indent=4)
