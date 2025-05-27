"""Manage data for the tombll database."""
import os
import sys
import re
import sqlite3
import json
import time
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
      -ac   [lid] Add a level card record without info and walkthrough
      -acr  [lid lid] Add a range of level card records
      -rm   [lid] Remove one level
      -u    [lid] Update a level record

      -lz   [Level.LevelID] List zip file records
      -az   [Level.LevelID Zip.name Zip.size Zip.md5sum]
                Optional and in order [Zip.url Zip.version Zip.release] Add a zip file
"""
    print(help_text.strip())


def database_make_connection():
    """
    Get a connection to the database.

    Returns:
        (sqlite3.Connection): An open SQLite database connection.

    """
    return sqlite3.connect(os.path.dirname(os.path.abspath(__file__)) + '/tombll.db')


def database_begin_write(con):
    """
    Run this before you call commit() when you write data as a starting point.

    If there is an data base error it will role back and exit the program.
    If it comes back sane you should run con.commit()

    Args:
        con (sqlite3.Connection): An open SQLite database connection.

    """
    cur = con.cursor()
    cur.execute("BEGIN;")


def database_commit_and_close(con):
    """
    Commit and close in one line or just run close if you only read data.

    Args:
        con (sqlite3.Connection): An open SQLite database connection.

    """
    con.commit()
    con.close()


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


def trle_level_url(lid):
    """
    Get level TRLE url from lid number.

    Args:
        con (sqlite3.Connection): An open SQLite database connection.
    """
    return f"https://www.trle.net/sc/levelfeatures.php?lid={lid}"


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


def parse_position(s):
    """Pares the letter to int from an jpg name."""
    match = re.match(r'^(\d+)([a-z]?)$', s)
    if not match:
        raise ValueError(f"Invalid format: {s}")
    suffix = match.group(2)
    return ord(suffix) - ord('a') + 1 if suffix else 0


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
        picture = data_factory.make_picture()
        # Fetch the .webp image data for the screen
        picture['data'] = \
            scrape_trle.get_trle_cover(screen.replace("https://www.trle.net/screens/", ""))
        picture['position'] = parse_position(scrape_trle.scrape_common.url_basename_prefix(screen))
        picture['md5sum'] = scrape_trle.scrape_common.calculate_data_md5(picture['data'])
        tombll_create.database_screen(picture, level_id, con)

    # Add large screens if they are provided
    large_screens = data.get('large_screens')
    if large_screens:
        webp_imgage_array = []
        for screen in large_screens:
            if screen.startswith("https://www.trle.net/screens/"):
                picture = data_factory.make_picture()
                picture['data'] = \
                    scrape_trle.get_trle_cover(screen.replace("https://www.trle.net/screens/", ""))
                picture['position'] = \
                    parse_position(scrape_trle.scrape_common.url_basename_prefix(screen))
                picture['md5sum'] = scrape_trle.scrape_common.calculate_data_md5(picture['data'])
                webp_imgage_array.append(picture)

        if webp_imgage_array:
            tombll_create.database_screens(webp_imgage_array, level_id, con)


def update_tombll_authors_to_database(authors, level_id, con):
    """Update authors data by adding and deleting.

    Args:
        authors (list of str): A list containing author names.
        level_id (int): Level.LevelID number.
        con (sqlite3.Connection): SQLite database connection.
    """
    database_authors = tombll_read.database_author_ids(level_id, con)
    new_set = set()
    for name in authors:
        author_id = tombll_create.database_author(name, level_id, con)
        new_set.add(author_id)

    if database_authors:
        current_set = set(a[0] for a in database_authors)
        to_remove = current_set - new_set
        for author_id in to_remove:
            tombll_delete.database_author(author_id, level_id, con)


def update_tombll_genres_to_database(genres, level_id, con):
    """Update genres data by adding and deleting.

    Args:
        genres (list of str): A list containing genres.
        level_id (int): Level.LevelID number.
        con (sqlite3.Connection): SQLite database connection.
    """
    database_genres = tombll_read.database_genre_ids(level_id, con)
    new_set = set()
    for genre in genres:
        genre_id = tombll_create.database_genre(genre, level_id, con)
        new_set.add(genre_id)

    if database_genres:
        current_set = set(a[0] for a in database_genres)
        to_remove = current_set - new_set
        for genre_id in to_remove:
            tombll_delete.database_genre(genre_id, level_id, con)


def update_tombll_tags_to_database(tags, level_id, con):
    """Update tags data by adding and deleting.

    Args:
        tags (list of str): A list containing tags.
        level_id (int): Level.LevelID number.
        con (sqlite3.Connection): SQLite database connection.
    """
    database_tags = tombll_read.database_tag_ids(level_id, con)
    new_set = set()
    for tag in tags:
        tag_id = tombll_create.database_tag(tag, level_id, con)
        new_set.add(tag_id)

    if database_tags:
        current_set = set(a[0] for a in database_tags)
        to_remove = current_set - new_set
        for tag_id in to_remove:
            tombll_delete.database_tag(tag_id, level_id, con)


def update_tombll_zip_files_to_database(zip_files, level_id, con):
    """Update zip_files data by adding and deleting.

    Args:
        zip_files (list of dict): A list containing zip_files.
        level_id (int): Level.LevelID number.
        con (sqlite3.Connection): SQLite database connection.
    """
    database_zip_files = tombll_read.database_zip_list(level_id, con)

    # Build sets for fast comparison
    current_set = set((z[1], z[4]) for z in database_zip_files)  # (name, url)
    new_set = set((z['name'], z['url']) for z in zip_files)

    # Find new files to add (using set difference)
    to_add = new_set - current_set
    for name, url in to_add:
        zip_file = next(z for z in zip_files if (z['name'], z['url']) == (name, url))
        tombll_create.database_zip_file(zip_file, level_id, con)

    # Find files to remove (using set difference)
    to_remove = current_set - new_set
    for name, url in to_remove:
        for d in database_zip_files:
            if (d[1], d[4]) == (name, url):
                tombll_delete.database_zip_file(d[0], level_id, con)
                break


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

    authors = data.get('authors')
    if isinstance(authors, list):
        update_tombll_authors_to_database(authors, level_id, con)

    genres = data.get('genres')
    if isinstance(genres, list):
        update_tombll_genres_to_database(genres, level_id, con)

    tags = data.get('tags')
    if isinstance(tags, list):
        update_tombll_tags_to_database(tags, level_id, con)

    zip_files = data.get('zip_files')
    if zip_files:
        update_tombll_zip_files_to_database(zip_files, level_id, con)

    new_set = set()
    # Single screen image, checked for None in the add_screen_to_database function
    screen = data.get('screen')
    if screen.startswith("https://www.trle.net/screens/"):
        picture = data_factory.make_picture()
        # Fetch the .webp image data for the screen
        picture['data'] = \
            scrape_trle.get_trle_cover(screen.replace("https://www.trle.net/screens/", ""))
        picture['position'] = parse_position(scrape_trle.scrape_common.url_basename_prefix(screen))
        picture['md5sum'] = scrape_trle.scrape_common.calculate_data_md5(picture['data'])
        picture_id = tombll_create.database_screen(picture, level_id, con)
        new_set.add(picture_id)

    # Add large screens if they are provided
    large_screens = data.get('large_screens')
    if large_screens:
        webp_imgage_array = []
        for screen in large_screens:
            if screen.startswith("https://www.trle.net/screens/"):
                picture = data_factory.make_picture()
                picture['data'] = \
                    scrape_trle.get_trle_cover(screen.replace("https://www.trle.net/screens/", ""))
                picture['position'] = \
                    parse_position(scrape_trle.scrape_common.url_basename_prefix(screen))
                picture['md5sum'] = scrape_trle.scrape_common.calculate_data_md5(picture['data'])
                webp_imgage_array.append(picture)

        if webp_imgage_array:
            new_set.update(tombll_create.database_screens(webp_imgage_array, level_id, con))

    database_pictures = tombll_read.database_picture_ids(level_id, con)
    if database_pictures:
        current_set = set(a[0] for a in database_pictures)
        to_remove = current_set - new_set
        for picture_id in to_remove:
            tombll_delete.database_picture(picture_id, level_id, con)


if __name__ == "__main__":
    number_of_argument = len(sys.argv)
    if (number_of_argument == 1 or number_of_argument >= 10):
        print_info()
        sys.exit(1)

    if (sys.argv[1] == "-h" and number_of_argument == 2):
        print_info()
        sys.exit(1)

    if (sys.argv[1] == "-l" and number_of_argument == 2):
        main_con = database_make_connection()
        print_list(main_con)
        main_con.close()

    elif (sys.argv[1] == "-a" and number_of_argument == 3):
        main_lid = sys.argv[2]
        main_data = data_factory.make_trle_tombll_data()

        main_soup = scrape_trle.scrape_common.get_soup(trle_level_url(main_lid))
        scrape_trle.get_trle_level(main_soup, main_data)

        if main_data['title']:
            main_con = database_make_connection()
            database_begin_write(main_con)
            add_tombll_json_to_database(main_data, main_con)
            database_commit_and_close(main_con)
            print(f"lid {main_lid} with title \"{main_data['title']}\" added successfully.")
        else:
            print(f"lid {main_lid} was an empty page.")

    elif (sys.argv[1] == "-aj" and number_of_argument == 4):
        main_lid = sys.argv[2]
        main_path = sys.argv[3]
        main_data = data_factory.make_trle_tombll_data()

        main_soup = scrape_trle.scrape_common.get_soup(trle_level_url(main_lid))
        scrape_trle.get_trle_level(main_soup, main_data)

        if main_data['title']:
            with open(main_path, mode='w', encoding='utf-8') as json_file:
                json.dump(main_data, json_file)
            print(
                f"lid {main_lid} with title \"{main_data['title']}\" "
                f"saved to {main_path} successfully."
            )
        else:
            print(f"lid {main_lid} was an empty page.")

    elif (sys.argv[1] == "-af" and number_of_argument == 3):
        main_path = sys.argv[2]
        main_data = tombll_common.get_tombll_json(main_path)

        main_con = database_make_connection()
        database_begin_write(main_con)
        add_tombll_json_to_database(main_data, main_con)
        database_commit_and_close(main_con)
        print(f"File {main_path} with title \"{main_data['title']}\" added successfully.")

    elif (sys.argv[1] == "-ac" and number_of_argument == 3):
        main_lid = sys.argv[2]
        main_data = data_factory.make_trle_tombll_data()

        main_soup = scrape_trle.scrape_common.get_soup(trle_level_url(main_lid))
        scrape_trle.get_trle_level_card(main_soup, main_data)

        if main_data['title']:
            main_con = database_make_connection()
            database_begin_write(main_con)
            add_tombll_json_to_database(main_data, main_con)
            database_commit_and_close(main_con)
            print(f"lid {main_lid} card with title \"{main_data['title']}\" added successfully.")
        else:
            print(f"lid {main_lid} was an empty page.")

    elif (sys.argv[1] == "-acr" and number_of_argument == 4):
        main_range_a = int(sys.argv[2])
        main_range_b = int(sys.argv[3])

        for i in range(min(main_range_a, main_range_b), max(main_range_a, main_range_b) + 1):
            main_data = data_factory.make_trle_tombll_data()
            main_soup = scrape_trle.scrape_common.get_soup(trle_level_url(i))
            scrape_trle.get_trle_level_card(main_soup, main_data)

            if main_data['title']:
                main_con = database_make_connection()
                database_begin_write(main_con)
                add_tombll_json_to_database(main_data, main_con)
                database_commit_and_close(main_con)
                print(f"lid {i} card with title \"{main_data['title']}\" added successfully.")
            else:
                print(f"lid {i} was an empty page.")

            time.sleep(5)

    elif (sys.argv[1] == "-rm" and number_of_argument == 3):
        main_lid = sys.argv[2]
        main_con = database_make_connection()
        main_database_level_id = tombll_read.database_level_id(main_lid, main_con)

        database_begin_write(main_con)
        tombll_delete.database_level(main_database_level_id, main_con)
        database_commit_and_close(main_con)
        print(f"lid {main_lid} removed successfully.")

    elif (sys.argv[1] == "-u" and number_of_argument == 3):
        main_lid = sys.argv[2]
        main_data = data_factory.make_trle_tombll_data()

        main_soup = scrape_trle.scrape_common.get_soup(trle_level_url(main_lid))
        scrape_trle.get_trle_level(main_soup, main_data)

        if main_data['title']:
            main_con = database_make_connection()
            main_level_id = tombll_read.database_level_id(main_lid, main_con)

            database_begin_write(main_con)
            update_tombll_json_to_database(main_data, main_level_id, main_con)
            database_commit_and_close(main_con)
            print(f"lid {main_lid} with title \"{main_data['title']}\" updated successfully.")
        else:
            print(f"lid {main_lid} was an empty page.")

    elif (sys.argv[1] == "-lz" and number_of_argument == 3):
        main_con = database_make_connection()
        print_download_list(sys.argv[2], main_con)
        main_con.close()

    elif (sys.argv[1] == "-az" and number_of_argument >= 5):
        main_data = data_factory.make_zip_file()
        main_data["name"] = sys.argv[3]
        main_data["size"] = sys.argv[4]
        main_data["md5"] = sys.argv[5]
        main_data["url"] = sys.argv[6] if number_of_argument >= 7 else None
        main_data["release"] = sys.argv[7] if number_of_argument >= 8 else None
        main_data["version"] = sys.argv[8] if number_of_argument >= 9 else None

        main_con = database_make_connection()
        database_begin_write(main_con)
        tombll_create.database_zip_file(main_data, sys.argv[2], main_con)
        database_commit_and_close(main_con)

    else:
        print_info()
