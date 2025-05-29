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
      -sj   [lid path] Save a level record to json file
      -aj   [path] Add a level record from a json file
      -ac   [lid] Add a level card record without info and walkthrough
      -acr  [lid lid] Add a range of level card records
      -rm   [lid] Remove one level record
      -u    [lid] Update a level record

      -ld   [lid] List download files records
      -ad   [lid Zip.name Zip.size Zip.md5sum]
                Add a local zip file to a level without a file
"""
    print(help_text.strip())


def list_levels():
    """Retrieve and prints a list of level information."""
    # Fetch all rows
    con = database_make_connection()
    results = tombll_read.database_level_list(con)
    con.close()

    # Iterate over the results and print each row
    for row in results:
        print(row)


def add_level(lid):
    """Add a level by taking the lid number."""
    data = data_factory.make_trle_tombll_data()

    soup = scrape_trle.scrape_common.get_soup(trle_level_url(lid))
    scrape_trle.get_trle_level(soup, data)

    if data['title']:
        con = database_make_connection()
        database_begin_write(con)
        add_tombll_json_to_database(data, con)
        database_commit_and_close(con)
        print(f"lid {lid} with title \"{data['title']}\" added successfully.")
    else:
        print(f"lid {lid} was an empty page.")


def save_json_level(lid, path):
    """Save a level to a json file on disk by taking the lid number."""
    data = data_factory.make_trle_tombll_data()

    soup = scrape_trle.scrape_common.get_soup(trle_level_url(lid))
    scrape_trle.get_trle_level(soup, data)

    if data['title']:
        with open(path, mode='w', encoding='utf-8') as json_file:
            json.dump(data, json_file)
        print(
            f"lid {lid} with title \"{data['title']}\" "
            f"saved to {path} successfully."
        )
    else:
        print(f"lid {lid} was an empty page.")


def add_file(path):
    """Add a level from a json file on disk."""
    data = tombll_common.get_tombll_json(path)

    con = database_make_connection()
    database_begin_write(con)
    add_tombll_json_to_database(data, con)
    database_commit_and_close(con)
    print(f"File {path} with title \"{data['title']}\" added successfully.")


def add_level_card(lid):
    """Add a level card by taking the lid number."""
    data = data_factory.make_trle_tombll_data()

    soup = scrape_trle.scrape_common.get_soup(trle_level_url(lid))
    scrape_trle.get_trle_level_card(soup, data)

    if data['title']:
        con = database_make_connection()
        database_begin_write(con)
        add_tombll_json_to_database(data, con)
        database_commit_and_close(con)
        print(f"lid {lid} card with title \"{data['title']}\" added successfully.")
    else:
        print(f"lid {lid} was an empty page.")


def add_level_card_range(range_a, range_b):
    """Add a level card by taking a lid range."""
    for i in range(min(int(range_a), int(range_b)), max(int(range_a), int(range_b)) + 1):
        data = data_factory.make_trle_tombll_data()
        soup = scrape_trle.scrape_common.get_soup(trle_level_url(i))
        scrape_trle.get_trle_level_card(soup, data)

        if data['title']:
            con = database_make_connection()
            database_begin_write(con)
            add_tombll_json_to_database(data, con)
            database_commit_and_close(con)
            print(f"lid {i} card with title \"{data['title']}\" added successfully.")
        else:
            print(f"lid {i} was an empty page.")

        time.sleep(5)


def remove_level(lid):
    """Remove a level from the database by taking the lid number."""
    con = database_make_connection()
    database_level_id = tombll_read.database_level_id(lid, con)

    database_begin_write(con)
    tombll_delete.database_level(database_level_id, con)
    database_commit_and_close(con)
    print(f"lid {lid} removed successfully.")


def update_level(lid):
    """Update a level by taking the lid number."""
    data = data_factory.make_trle_tombll_data()

    soup = scrape_trle.scrape_common.get_soup(trle_level_url(lid))
    scrape_trle.get_trle_level(soup, data)

    if data['title']:
        con = database_make_connection()
        level_id = tombll_read.database_level_id(lid, con)

        database_begin_write(con)
        update_tombll_json_to_database(data, level_id, con)
        database_commit_and_close(con)
        print(f"lid {lid} with title \"{data['title']}\" updated successfully.")
    else:
        print(f"lid {lid} was an empty page.")


def add_download(lid, name, size, md5):
    """Add a download file to a level by taking the lid number."""
    data = data_factory.make_zip_file()
    data["name"] = name
    data["size"] = size
    data["md5"] = md5
    data["url"] = None
    data["release"] = None
    data["version"] = None

    con = database_make_connection()
    database_begin_write(con)
    level_id = tombll_read.database_level_id(lid, con)
    tombll_create.database_zip_file(data, level_id, con)
    database_commit_and_close(con)


def list_downloads(lid):
    """Print a list of ZIP file entries associated with a specific level."""
    # Fetch all rows
    con = database_make_connection()
    level_id = tombll_read.database_level_id(lid, con)
    results = tombll_read.database_zip_list(level_id, con)
    con.close()

    # Print each row in the result
    for row in results:
        print(row)


def get_local_page(offset, con):
    """Pass down API to get a TRLE page from the database."""
    return tombll_read.trle_page(offset, con, sort_latest_first=True)


def get_trle_page(offset):
    """Pass down API to scrape a TRLE page from "trle.net"."""
    return scrape_trle.get_trle_page(offset, sort_created_first=True)


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


def trle_level_url(lid):
    """
    Get level TRLE url from lid number.

    Args:
        con (sqlite3.Connection): An open SQLite database connection.
    """
    return f"https://www.trle.net/sc/levelfeatures.php?lid={lid}"


def parse_position(s):
    """Pares the letter to int from an jpg name."""
    match = re.match(r'^(\d+)([a-z]?)$', s)
    if not match:
        raise ValueError(f"Invalid format: {s}")
    suffix = match.group(2)
    return ord(suffix) - ord('a') + 1 if suffix else 0


def add_screen_from_url(screen, level_id, con):
    """
    Scrape the url picture and add to database.

    Returns:
        INT: Picture ID.

    """
    if screen.startswith("https://www.trle.net/screens/"):
        picture = data_factory.make_picture()
        # Fetch the .webp image data
        relative_path = screen.replace("https://www.trle.net/screens/", "")
        picture['data'] = scrape_trle.get_trle_cover(relative_path)
        picture['position'] = parse_position(scrape_trle.scrape_common.url_basename_prefix(screen))
        picture['md5sum'] = scrape_trle.scrape_common.calculate_data_md5(picture['data'])
        return tombll_create.database_screen(picture, level_id, con)

    print("Cover did not start with https://www.trle.net/screens/ ")
    sys.exit(1)


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
    add_screen_from_url(data.get('screen'), level_id, con)

    # Add large screens if they are provided
    large_screens = data.get('large_screens')
    if large_screens:
        for screen in large_screens:
            add_screen_from_url(screen, level_id, con)


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

    # Single screen image, checked for None in the add_screen_to_database function
    new_set = set()
    new_set.add(add_screen_from_url(data.get('screen'), level_id, con))

    # Add large screens if they are provided
    large_screens = data.get('large_screens')
    if large_screens:
        for screen in large_screens:
            new_set.add(add_screen_from_url(screen, level_id, con))

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
        list_levels()

    elif (sys.argv[1] == "-a" and number_of_argument == 3):
        add_level(sys.argv[2])

    elif (sys.argv[1] == "-sj" and number_of_argument == 4):
        save_json_level(sys.argv[2], sys.argv[3])

    elif (sys.argv[1] == "-aj" and number_of_argument == 3):
        add_file(sys.argv[2])

    elif (sys.argv[1] == "-ac" and number_of_argument == 3):
        add_level_card(sys.argv[2])

    elif (sys.argv[1] == "-acr" and number_of_argument == 4):
        add_level_card_range(sys.argv[2], sys.argv[3])

    elif (sys.argv[1] == "-rm" and number_of_argument == 3):
        remove_level(sys.argv[2])

    elif (sys.argv[1] == "-u" and number_of_argument == 3):
        update_level(sys.argv[2])

    elif (sys.argv[1] == "-ld" and number_of_argument == 3):
        list_downloads(sys.argv[2])

    elif (sys.argv[1] == "-ad" and number_of_argument == 6):
        add_download(sys.argv[2], sys.argv[3], sys.argv[4], sys.argv[5])

    else:
        print_info()
