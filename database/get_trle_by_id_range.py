"""
This script retrieves all TRLE levels by a specified ID range and saves the data as JSON files.
Usage: python3 get_trle_by_id_range.py FROM_ID TO_ID
"""
import sys
import time
import json

import scrape
import data_factory


def safe_string_to_int(id_str):
    """Converts a string to an integer with error checking.

    Args:
        s (str): The string to convert.

    Returns:
        int: The converted integer if valid.

    Exits:
        Exits with status code 1 if the input string is not a valid integer.
    """
    try:
        return int(id_str)
    except ValueError:
        print("Error: The provided string is not a valid integer.")
        sys.exit(1)


def trle_by_id(trle_id):
    """Fetches TRLE level data by ID and saves it as a JSON file.

    Args:
        trle_id (int): The ID of the TRLE level to fetch.
    """
    data = data_factory.make_trle_tombll_data()
    soup = scrape.get_soup(f"https://www.trle.net/sc/levelfeatures.php?lid={trle_id}")
    scrape.get_trle_level(soup, data)
    if data['title']:
        with open(f'trle/{trle_id}.json', mode='w', encoding='utf-8') as json_file:
            json.dump(data, json_file)


if __name__ == '__main__':

    if len(sys.argv) != 3:
        print("Usage: python3 get_trle_by_id_range.py FROM_ID TO_ID")
        sys.exit(1)

    print("Please use get_trle.sh; I provide this as part of the open source license.")
    if input("Continue? (y/n): ").lower() != 'y':
        sys.exit(1)

    # Convert arguments to integers with validation
    from_id = safe_string_to_int(sys.argv[1])
    to_id = safe_string_to_int(sys.argv[2])

    if from_id == to_id:
        trle_by_id(from_id)
        sys.exit(0)

    # Ensure from_id is less than to_id by swapping if necessary
    if from_id > to_id:
        from_id, to_id = to_id, from_id  # Tuple for cleaner swapping... python...

    # Fetch and save data for each level ID in the specified range
    for level_id in range(from_id, to_id + 1):  # Include to_id in range
        print(f"Getting TRLE level by ID: {level_id}")
        trle_by_id(level_id)
        time.sleep(5)  # To avoid rate-limiting by adding delay between requests
