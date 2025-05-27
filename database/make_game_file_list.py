"""Record original game files."""
import re
import os
import sys
import json
import hashlib


def _get_game_dir():
    """Get basic user input."""
    path = input("Path to directory: ")

    # Check if the provided path is a valid directory
    if not os.path.isdir(path):
        print("Error: Invalid directory path.")
        sys.exit(1)

    tag = input("Specify one of the following tags TR1, TR2, TR3, TR4 or TR5: ")

    if tag not in ["TR1", "TR2", "TR3", "TR4", "TR5"]:
        print("Error: TAG input error.")
        sys.exit(1)

    return path, tag


def _game_file_loop(game_dir):
    """Loop through the files."""
    file_info = {}
    savegame_pattern = re.compile(r'^savegame\.\d+$', re.IGNORECASE)
    dxvk_caches = re.compile(r'^.*\.dxvk-cache$')

    for root, _, files in os.walk(game_dir):
        for file in files:
            # Skip some files we don't need to track
            if savegame_pattern.match(file):
                continue
            if dxvk_caches.match(file):
                continue
            if file == 'installscript.vdf':
                continue

            file_path = os.path.join(root, file)

            # Get the relative path using os.path.relpath()
            relative_path = os.path.relpath(file_path, game_dir)

            # Calculate the MD5 hash of the file
            with open(file_path, "rb") as current_file:
                file_content = current_file.read()
                md5sum = hashlib.md5(file_content, usedforsecurity=False).hexdigest()

            # Store file info
            file_info[relative_path] = md5sum

    return file_info


def make_game_file_list():
    """Get the game file list."""
    game_dir, tag = _get_game_dir()
    file_info = _game_file_loop(game_dir)

    if not isinstance(file_info, dict):
        print("File type return from game_file_loop error")
        sys.exit(1)

    # Sort the file_info by filename (relative path)
    sorted_file_info = dict(sorted(file_info.items()))

    # Create the JSON data structure
    data_list = [
        {"filename": filename, "md5sum": md5sum}
        for filename, md5sum in sorted_file_info.items()
    ]

    new_json_object = {
        "name": tag,
        "file_data_list": data_list
    }

    # Write the JSON object to a file
    output_filename = f'fileList-{tag}.json'
    with open(output_filename, 'w', encoding="utf-8") as json_file:
        json.dump(new_json_object, json_file, indent=2)

    print(f"File list saved to {output_filename}")
