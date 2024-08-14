import os
import sys
import json
import hashlib

if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("Usage: python3 makeGameFileList.py TAG GAMEDIR")
        sys.exit(1)

    tag = sys.argv[1]
    if not tag:
        print("Error: TAG cannot be an empty string.")
        sys.exit(1)

    game_directory = sys.argv[2]
    if not game_directory:
        print("Error: GAMEDIR cannot be an empty string.")
        sys.exit(1)

    working_dir = os.getcwd()

    # Construct the directory path
    if game_directory.startswith('/'):
        directory = game_directory
    else:
        directory = os.path.join(working_dir, game_directory)

    # Change to the script directory
    script_dir = os.path.dirname(os.path.abspath(__file__))
    os.chdir(script_dir)

    file_info = {}

    # Walk through the directory and collect file information
    for root, dirs, files in os.walk(directory):
        for file in files:
            file_path = os.path.join(root, file)

            # Get the relative path
            if file_path.startswith(directory):
                relative_path = file_path[len(directory):]
            else:
                # Skip files not within the specified directory
                continue

            # Calculate the MD5 hash of the file
            with open(file_path, "rb") as current_file:
                file_content = current_file.read()
                md5sum = hashlib.md5(file_content).hexdigest()

            # Store file info
            file_info[relative_path] = md5sum

    # Create the JSON data structure
    data_list = [
        {"filename": filename, "md5sum": md5sum}
        for filename, md5sum in file_info.items()
    ]

    new_json_object = {
        "name": tag,
        "file_data_list": data_list
    }

    # Write the JSON object to a file
    output_filename = f'fileList-{tag}.json'
    with open(output_filename, 'w') as json_file:
        json.dump(new_json_object, json_file, indent=2)

    print(f"File list saved to {output_filename}")
