"""Manage data for the tombll database."""
import os
import sys
import sqlite3
import json
import logging

import scrape_trle
import data_factory

import tombll_common
import tombll_create
import tombll_delete
import tombll_read
import tombll_update

# Set up logging
logging.basicConfig(level=logging.DEBUG, format='%(asctime)s %(levelname)s:%(message)s')
logging.getLogger("requests").setLevel(logging.DEBUG)


def print_info():
    """Print help info."""
    help_text = """
Usage: python3 tombll_manage_data.py [options]
  Options:
      -l    List level records
      -a    [lid] Add a level record
      -aj   [lid path] Download a level record to json file
      -af   [path] Add from the json file
      -rm   [Level.LevelID] Remove one level
      -u    [lid] Update a level record

      -lz   [Level.LevelID] List zip file records
      -az   [Level.LevelID Zip.name Zip.size Zip.md5sum]
                Optional and in order [Zip.url Zip.version Zip.release] Add a zip file
"""
    print(help_text.strip())


def print_list(con):
    """
    Retrieve and prints a list of level information.

    Args:
        con (sqlite3.Connection): An open SQLite database connection.

    Returns:
        None: This function does not return any value.
        It prints the results directly to the console.
    """
    # Fetch all rows
    results = tombll_read.database_level_list(con)

    # Iterate over the results and print each row
    for row in results:
        print(row)


def print_download_list(level_id, con):
    """
    Print a list of ZIP file entries associated with a specific level.

    Args:
        level_id (int): The ID of the level for which to retrieve ZIP files.
        con (sqlite3.Connection): An active SQLite database connection.

    Returns:
        None: This function prints the query results to the console.

    """
    # Fetch all rows
    results = tombll_read.database_zip_list(level_id, con)

    # Print each row in the result
    for row in results:
        print(row)


def add_tombll_json_to_database(data, con):
    """Insert level data and related details into the database.

    This function inserts a level record into the database and subsequently
    adds associated authors, genres, tags, zip files, screen, and large screens.

    Args:
        data (dict): A dictionary containing level data and related details.
        con (sqlite3.Connection): SQLite database connection.
    """
    # Insert level information and obtain the generated level ID
    level_id = tombll_create.database_level(data, con)

    # Add related information only if corresponding data is present
    if data.get('authors'):
        tombll_create.database_authors(data['authors'], level_id, con)
    if data.get('genres'):
        tombll_create.database_genres(data['genres'], level_id, con)
    if data.get('tags'):
        tombll_create.database_tags(data['tags'], level_id, con)
    if data.get('zip_files'):
        tombll_create.database_zip_files(data['zip_files'], level_id, con)

    # Single screen image, checked for None in the add_screen_to_database function
    screen = data.get('screen')
    if screen.startswith("https://www.trle.net/screens/"):
        # Fetch the .webp image data for the screen
        webp_image_data = \
                scrape_trle.get_trle_cover(screen.replace("https://www.trle.net/screens/", ""))
        tombll_create.database_screen(webp_image_data, level_id, con)

    # Add large screens if they are provided
    large_screens = data.get('large_screens')
    if large_screens:
        webp_imgage_array = []
        for screen in large_screens:
            webp_image_data = \
                scrape_trle.get_trle_cover(screen.replace("https://www.trle.net/screens/", ""))
            webp_imgage_array.append(webp_image_data)

        if webp_imgage_array:
            tombll_create.database_screens(webp_imgage_array, level_id, con)


def update_tombll_json_to_database(data, level_id, con):
    """Update level data and related attributes into the database.

    This function update a level record into the database and subsequently
    updates associated authors, genres, tags, zip files, screen, and large screens.

    Args:
        data (dict): A dictionary containing level data and related details.
        level_id (int): Level.LevelID number.
        con (sqlite3.Connection): SQLite database connection.
    """
    info_id = tombll_update.database_level(data, level_id, con)
    tombll_update.database_info(data, info_id, con)

    if data.get('authors'):
        database_authors = tombll_read.database_author_list(level_id, con)
        current_set = set(a[0] for a in database_authors)
        new_set = set(data['authors'])
        to_add = new_set - current_set
        to_remove = current_set - new_set
        for author in to_add:
            tombll_create.database_author(author, level_id, con)
        for author in to_remove:
            tombll_delete.database_author(author, level_id, con)

    # TODO: update more


if __name__ == "__main__":
    number_of_argument = len(sys.argv)
    if (number_of_argument == 1 or number_of_argument >= 10):
        print("here")
        print_info()
        sys.exit(1)
    if (sys.argv[1] == "-h" and number_of_argument == 2):
        print_info()
        sys.exit(1)

    main_con = sqlite3.connect(os.path.dirname(os.path.abspath(__file__)) + '/tombll.db')
    if (sys.argv[1] == "-l" and number_of_argument == 2):
        print_list(main_con)
    elif (sys.argv[1] == "-a" and number_of_argument == 3):
        main_cur = main_con.cursor()
        main_cur.execute("BEGIN;")
        main_lid = sys.argv[2]
        main_data = data_factory.make_trle_tombll_data()
        main_soup = scrape_trle.scrape_common.get_soup(
            f"https://www.trle.net/sc/levelfeatures.php?lid={main_lid}")
        scrape_trle.get_trle_level(main_soup, main_data)
        add_tombll_json_to_database(main_data, main_con)
        main_con.commit()

        print(f"Level {sys.argv[2]} and related authors removed successfully.")
    elif (sys.argv[1] == "-aj" and number_of_argument == 4):
        main_lid = sys.argv[2]
        main_data = data_factory.make_trle_tombll_data()
        main_soup = scrape_trle.scrape_common.get_soup(
            f"https://www.trle.net/sc/levelfeatures.php?lid={main_lid}")
        scrape_trle.get_trle_level(main_soup, main_data)
        with open(sys.argv[3], mode='w', encoding='utf-8') as json_file:
            json.dump(main_data, json_file)
        print(f"Level {sys.argv[2]} saved to {sys.argv[3]} successfully.")
    elif (sys.argv[1] == "-af" and number_of_argument == 3):
        main_data = tombll_common.get_tombll_json(sys.argv[2])
        main_cur = main_con.cursor()
        main_cur.execute("BEGIN;")
        add_tombll_json_to_database(main_data, main_con)
        main_con.commit()
        print(f"File {sys.argv[2]} added successfully.")
    elif (sys.argv[1] == "-rm" and number_of_argument == 3):
        cur = main_con.cursor()
        cur.execute("BEGIN;")
        tombll_delete.database_level(sys.argv[2], main_con)
        main_con.commit()
        print(f"Level {sys.argv[2]} removed successfully.")
    elif (sys.argv[1] == "-u" and number_of_argument == 3):
        main_cur = main_con.cursor()
        main_cur.execute("BEGIN;")
        main_lid = sys.argv[2]
        main_data = data_factory.make_trle_tombll_data()
        main_soup = scrape_trle.scrape_common.get_soup(
            f"https://www.trle.net/sc/levelfeatures.php?lid={main_lid}")
        scrape_trle.get_trle_level(main_soup, main_data)
        main_level_id = tombll_read.database_level_id(main_lid, main_con)
        update_tombll_json_to_database(main_data, main_level_id, main_con)
        main_con.commit()
        print(f"Level {sys.argv[2]} updated successfully.")
    elif (sys.argv[1] == "-lz" and number_of_argument == 3):
        print_download_list(sys.argv[2], main_con)
    elif (sys.argv[1] == "-az" and number_of_argument >= 5):
        main_data = data_factory.make_zip_file()
        main_data["name"] = sys.argv[3]
        main_data["size"] = sys.argv[4]
        main_data["md5"] = sys.argv[5]
        main_data["url"] = sys.argv[6] if number_of_argument == 7 else None
        main_data["release"] = sys.argv[7] if number_of_argument == 8 else None
        main_data["version"] = sys.argv[8] if number_of_argument == 9 else None
        tombll_create.database_zip_file(main_data, sys.argv[2], main_con)
        main_con.commit()
    else:
        print_info()
    main_con.close()
