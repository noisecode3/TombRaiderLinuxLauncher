import os
import sys
import json
import hashlib
if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("Usage: python3 makeGameFileList.py TAG GAMEDIR")
        sys.exit(1)
    else:
        tag = sys.argv[1]
        file = sys.argv[2]

working_dir = os.getcwd()

if file[0] == '/':
    DIRECTORY = file;
else:
    DIRECTORY = working_dir + "/" + file;

script_dir = os.path.dirname(os.path.abspath(__file__))
os.chdir(script_dir)
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

data_list = [
    {"filename": filename, "md5sum": md5sum}
    for filename, md5sum in file_info.items()
]

new_json_object = {
    "name": tag,
    "file_data_list": data_list
}

with open('fileList-'+ tag +'.json', 'w') as json_file:
    json.dump(new_json_object, json_file, indent=2)
