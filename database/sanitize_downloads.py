"""
This script gose thure all TRLE levels by a specified ID range and sanitize the downloads.
Usage: python3 sanitize_downloads.py FROM_ID TO_ID
"""
import os
import sys
import json
import re


def new_input(data, file_path):
    """Take new input"""
    zip_file = data['zip_files'][0]
    print(zip_file)
    if input("Do you want to remove the file? y/n: ") == 'y':
        os.remove(file_path)
        print(f"{file_path} has been removed.")
        return

    zip_file['name'] = input("New name: ")
    zip_file['size'] = float(input("New size: "))
    zip_file['md5'] = input("New md5: ")
    zip_file['url'] = input("New url: ")
    with open(file_path, mode='w', encoding='utf-8') as json_file:
        json.dump(data, json_file)


def sanitize(data, file_path):
    """
    Validates the 'zip_file' data from the given dictionary.

    This function checks:
    1. That the 'name' attribute exists and ends with ".zip".
    2. That the 'size' attribute is a float, greater than 2, and has exactly two decimal places.
    3. That the 'md5' attribute is a valid 32-character hexadecimal MD5 hash.
    4. That the 'url' attribute matches one of the allowed URL patterns.

    Args:
        data (dict): The input dictionary containing 'zip_files' data.

    Exits:
        Exits the program with status 1 if any validation fails.
    """
    zip_file = data['zip_files'][0]
    errors = []

    # Validate name
    name = zip_file.get('name')
    if not name:
        errors.append("The 'name' attribute is missing.")
    elif not name.endswith(".zip") or "$" in name:
        errors.append(f"The file '{name}' is not a valid .zip file.")

    # Validate size
    size = zip_file.get('size')
    if not isinstance(size, float):
        errors.append("The 'size' attribute is missing or not a float.")
    elif size <= 2:
        errors.append("The 'size' attribute is smaller than 2 MiB.")

    # Validate md5
    md5 = zip_file.get('md5')
    if not md5:
        errors.append("The 'md5' attribute is missing.")
    elif not re.fullmatch(r"^[a-fA-F0-9]{32}$", md5):
        errors.append("The 'md5' attribute is not a valid 32-character hexadecimal MD5 hash.")

    # Validate url
    url = zip_file.get('url')
    pattern1 = r"^https://trcustoms\.org/api/level_files/\d+/download$"
    pattern2 = r"^https://www\.trle\.net/levels/levels/\d{4}/\d{4}/[a-zA-Z0-9%-_\.$]+\.zip$"
    if not url:
        errors.append("The 'url' attribute is missing.")
    elif not re.match(pattern1, url) and not re.match(pattern2, url):
        errors.append("The 'url' attribute does not match any of the expected patterns.")

    # Handle errors
    if errors:
        for error in errors:
            print(error)
        new_input(data, file_path)


def safe_string_to_int(id_str):
    """Make sure the input is an int"""
    try:
        return int(id_str)
    except ValueError:
        print("Error: The provided string is not a valid integer.")
        sys.exit(1)


def trle_by_id(trle_id):
    """Open File by id"""
    file_path = f"trle/{trle_id}.json"

    if os.path.exists(file_path):
        print(f"File {file_path} exists.")
    else:
        print(f"File {file_path} does not exist.")
        return

    with open(file_path, "r", encoding='utf-8') as file:
        data = json.load(file)

    sanitize(data, file_path)


if __name__ == '__main__':
    if len(sys.argv) != 3:
        print("Usage: python3 get_trle_by_id_range.py FROM_ID TO_ID")
        sys.exit(1)

    # Convert arguments to integers with validation
    from_id = safe_string_to_int(sys.argv[1])
    to_id = safe_string_to_int(sys.argv[2])

    if from_id == to_id:
        trle_by_id(from_id)
        sys.exit(0)

    # Ensure from_id is less than to_id by swapping if necessary
    if from_id > to_id:
        from_id, to_id = to_id, from_id

    # Fetch and save data for each level ID in the specified range
    for level_id in range(from_id, to_id + 1):
        print(f"Getting TRLE level by ID: {level_id}")
        trle_by_id(level_id)
