"""
Take tombll json file and add it to the database
"""
import os
import sys
import sqlite3
import json
import logging

import scrape

def get_tombll_json(path):
    """Load and parse a JSON file from a specified path.

    Attempts to open and read a JSON file, parsing its content into a dictionary.
    Handles errors for file not found, JSON decoding issues, and other I/O problems.

    Args:
        path (str): The path to the JSON file.

    Returns:
        dict: Parsed content of the JSON file.

    Exits:
        Logs an error and exits the program if the file cannot be read or parsed.
    """
    try:
        # Open the file with UTF-8 encoding
        with open(path, mode='r', encoding='utf-8') as json_file:
            try:
                # Parse and return JSON content
                return json.load(json_file)
            except json.JSONDecodeError as json_error:
                # Log and exit if JSON content is invalid
                logging.error("Error decoding JSON from file '%s': %s", path, json_error)
                sys.exit(1)
    except FileNotFoundError:
        # Log and exit if file is not found
        logging.error("File not found: '%s'", path)
        sys.exit(1)
    except IOError as file_error:
        # Log and exit if any other I/O error occurs
        logging.error("I/O error occurred while opening file '%s': %s", path, file_error)
        sys.exit(1)


def query_return_id(query, params, con):
    """Execute a SQL query and return an ID.

    If the query is an INSERT, this function returns the last inserted row ID.
    For other queries, it fetches and returns the first integer result, if it exists and is
    non-negative.

    Args:
        query (str): SQL query to execute.
        params (tuple): Parameters for the query.
        con (sqlite3.Connection): SQLite database connection.

    Returns:
        int or None: The ID from the query result, or None if not found.

    Exits:
        Logs an error and exits if a database error occurs.
    """
    cursor = con.cursor()
    try:
        # Execute the query with provided parameters
        cursor.execute(query, params)

        # Check if it's an INSERT query to return the last inserted row ID
        if query.strip().upper().startswith("INSERT"):
            return cursor.lastrowid

        # For non-INSERT queries, fetch and validate the first result
        result = cursor.fetchone()
        if result and isinstance(result[0], int) and result[0] >= 0:
            return result[0]

        return None  # Return None if no valid ID is found

    except sqlite3.DatabaseError as db_error:
        # Log the database error and exit
        logging.error("Database error occurred: %s", db_error)
        sys.exit(1)


def query_run(query, params, con):
    """Execute a SQL query with the provided parameters.

    Args:
        query (str): The SQL query to execute.
        params (tuple): Parameters to substitute in the SQL query.
        con (sqlite3.Connection): SQLite database connection.

    Exits:
        Logs an error and exits if a database error occurs.
    """
    cursor = con.cursor()
    try:
        # Execute the query with provided parameters
        cursor.execute(query, params)
    except sqlite3.DatabaseError as db_error:
        # Log the database error and exit the program
        logging.error("Database error occurred: %s", db_error)
        sys.exit(1)


def make_empty_null(value):
    """Convert empty strings or zero float values to None.

    Args:
        value: The value to check, which can be a string, float, or other types.

    Returns:
        None if the value is an empty string or exactly 0.0; otherwise, returns the original value.
    """
    if value in ("", 0.0):
        return None
    return value


def add_authors_to_database(authors_array, level_id, con):
    """Add authors to the database, linking them with a specific level.

    This function checks if each author in `authors_array` already exists in the database.
    If not, it inserts the author and creates an entry in the AuthorList table
    to link the author to the specified level.

    Args:
        authors_array (list of str): List of author names.
        level_id (int): The ID of the level to link authors with.
        con (sqlite3.Connection): SQLite database connection.
    """
    for author in authors_array:
        # SQL queries for selecting, inserting, and linking authors
        query_select_id = "SELECT AuthorID FROM Author WHERE value = ?"
        query_insert = "INSERT INTO Author (value) VALUES (?)"
        query_insert_middle = "INSERT INTO AuthorList (authorID, levelID) VALUES (?, ?)"

        # Try to get the existing author ID; if none, insert a new author and get its ID
        author_id = query_return_id(query_select_id, (author,), con)
        if author_id is None:
            author_id = query_return_id(query_insert, (author,), con)

        # Link the author with the level in AuthorList table
        query_run(query_insert_middle, (author_id, level_id), con)


def add_genres_to_database(genres_array, level_id, con):
    """Add genres to the database, linking them with a specific level.

    This function checks if each genre in `genres_array` already exists in the database.
    If not, it inserts the genre and creates an entry in the GenreList table
    to link the genre to the specified level.

    Args:
        genres_array (list of str): List of genre names.
        level_id (int): The ID of the level to link genres with.
        con (sqlite3.Connection): SQLite database connection.
    """
    for genre in genres_array:
        # SQL queries for selecting, inserting, and linking genres
        query_select_id = "SELECT GenreID FROM Genre WHERE value = ?"
        query_insert = "INSERT INTO Genre (value) VALUES (?)"
        query_insert_middle = "INSERT INTO GenreList (genreID, levelID) VALUES (?, ?)"

        # Try to get the existing genre ID; if none, insert a new genre and get its ID
        genre_id = query_return_id(query_select_id, (genre,), con)
        if genre_id is None:
            genre_id = query_return_id(query_insert, (genre,), con)

        # Link the genre with the level in GenreList table
        query_run(query_insert_middle, (genre_id, level_id), con)


def add_tags_to_database(tags_array, level_id, con):
    """Add tags to the database, linking them with a specific level.

    This function checks if each tag in `tags_array` already exists in the database.
    If not, it inserts the tag and creates an entry in the TagList table
    to link the tag to the specified level.

    Args:
        tags_array (list of str): List of tag names.
        level_id (int): The ID of the level to link tags with.
        con (sqlite3.Connection): SQLite database connection.
    """
    for tag in tags_array:
        # SQL queries for selecting, inserting, and linking tags
        query_select_id = "SELECT TagID FROM Tag WHERE value = ?"
        query_insert = "INSERT INTO Tag (value) VALUES (?)"
        query_insert_middle = "INSERT INTO TagList (tagID, levelID) VALUES (?, ?)"

        # Try to get the existing tag ID; if not found, insert a new tag and get its ID
        tag_id = query_return_id(query_select_id, (tag,), con)
        if tag_id is None:
            tag_id = query_return_id(query_insert, (tag,), con)

        # Link the tag with the level in TagList table
        query_run(query_insert_middle, (tag_id, level_id), con)


def add_zip_files_to_database(zip_files_array, level_id, con):
    """Add ZIP files to the database, linking them with a specific level.

    This function inserts each ZIP file in `zip_files_array` into the Zip table
    if the file's attributes are provided, setting any empty or missing values to `None`.
    It then links the inserted ZIP file to the specified level in the ZipList table.

    Args:
        zip_files_array (list of dict): List of ZIP file details, each represented as a dictionary
        with keys 'name', 'size', 'md5', 'url', 'version', 'release'.
        level_id (int): The ID of the level to link ZIP files with.
        con (sqlite3.Connection): SQLite database connection.
    """
    for zip_file in zip_files_array:
        # SQL queries for inserting ZIP file data and linking ZIP files to a level
        query_insert = (
            "INSERT INTO Zip (name, size, md5sum, url, version, release) "
            "VALUES (?, ?, ?, ?, ?, ?)"
        )

        # Prepare arguments for inserting the ZIP file, converting empty values to None
        insert_args = (
            make_empty_null(zip_file.get('name')),
            make_empty_null(zip_file.get('size')),
            make_empty_null(zip_file.get('md5')),
            make_empty_null(zip_file.get('url')),
            make_empty_null(zip_file.get('version')),
            make_empty_null(zip_file.get('release'))
        )

        # Insert the ZIP file and get its ID
        zip_id = query_return_id(query_insert, insert_args, con)

        # Link the ZIP file to the level in ZipList table
        query_insert_middle = "INSERT INTO ZipList (zipID, levelID) VALUES (?, ?)"
        middle_args = (zip_id, level_id)
        query_run(query_insert_middle, middle_args, con)


def add_screen_to_database(screen, level_id, con):
    """Add a screen image to the database and link it to a specific level.

    This function checks if the screen URL points to the TRLE.net screens URL path,
    then downloads the corresponding image as a .webp file, and inserts it into the
    Picture table. It also creates an association with the specified level in the Screens table.

    Args:
        screen (str): URL of the screen image.
        level_id (int): The ID of the level to link the screen image with.
        con (sqlite3.Connection): SQLite database connection.
    """
    # Ensure the screen URL matches the TRLE.net screens directory
    if screen.startswith("https://www.trle.net/screens/"):
        # Fetch the .webp image data for the screen
        webp_image_data = scrape.get_trle_cover(screen.replace("https://www.trle.net/screens/", ""))

        # Insert the .webp image data into the Picture table and retrieve its ID
        query_insert_picture = "INSERT INTO Picture (data) VALUES (?)"
        picture_id = query_return_id(query_insert_picture, (webp_image_data,), con)

        # Link the inserted picture to the specified level in the Screens table
        query_insert_screen = "INSERT INTO Screens (pictureID, levelID) VALUES (?, ?)"
        query_run(query_insert_screen, (picture_id, level_id), con)


def add_screens_to_database(large_screens_array, level_id, con):
    """Add multiple screen images to the database and associate them with a specific level.

    This function iterates over an array of screen URLs, adding each to the database and linking it
    to the specified level using the `add_screen_to_database` helper function.

    Args:
        large_screens_array (list): List of URLs of screen images.
        level_id (int): The ID of the level to associate the screen images with.
        con (sqlite3.Connection): SQLite database connection.
    """
    # Iterate over each screen URL in the provided array and add it to the database
    for screen in large_screens_array:
        add_screen_to_database(screen, level_id, con)


def add_level_to_database(data, con):
    """Insert a level entry into the Level table and return its ID.

    This function inserts a new level into the database, using information provided in the
    `data` dictionary. It retrieves or creates an associated `infoID` using the
    `add_info_to_database` helper function, then logs and returns the `level_id` of
    the inserted level.

    Args:
        data (dict): A dictionary containing level data, including 'body' and 'walkthrough' fields.
        con (sqlite3.Connection): SQLite database connection.

    Returns:
        int: The ID of the newly inserted level.
    """
    # SQL query to insert a new level record
    query = "INSERT INTO Level (body, walkthrough, infoID) VALUES (?, ?, ?)"

    # Prepare arguments for the insertion, including the infoID from `add_info_to_database`
    arg = (
        data.get('body'),          # Level body content
        data.get('walkthrough'),    # Level walkthrough content
        add_info_to_database(data, con)  # Retrieve or create infoID
    )

    # Execute the query and get the ID of the inserted level
    level_id = query_return_id(query, arg, con)

    # Log the current level ID for debugging or tracking purposes
    logging.info("Current tombll level_id: %s", level_id)

    return level_id


def add_info_to_database(data, con):
    """Insert or retrieve IDs for level information attributes and add an Info record.

    This function retrieves or sets IDs for various level attributes (difficulty, duration,
    type, class) and uses them to insert a new record into the Info table.

    Args:
        data (dict): A dictionary containing level information attributes.
        con (sqlite3.Connection): SQLite database connection.

    Returns:
        int: The ID of the newly inserted or existing Info record.
    """
    # Retrieve or assign InfoDifficultyID, or set to None if not specified
    info_difficulty = data.get('difficulty')
    info_difficulty_id = None
    if info_difficulty:
        query = "SELECT InfoDifficultyID FROM InfoDifficulty WHERE value = ?"
        info_difficulty_id = query_return_id(query, (info_difficulty,), con)

    # Retrieve or assign InfoDurationID, or set to None if not specified
    info_duration = data.get('duration')
    info_duration_id = None
    if info_duration:
        query = "SELECT InfoDurationID FROM InfoDuration WHERE value = ?"
        info_duration_id = query_return_id(query, (info_duration,), con)

    # Retrieve or assign InfoTypeID, allowing None if not specified
    info_type = data.get('type') or None
    query = "SELECT InfoTypeID FROM InfoType WHERE value = ?"
    info_type_id = query_return_id(query, (info_type,), con)

    # Retrieve or assign InfoClassID, allowing None if not specified
    info_class = data.get('class') or None
    query = "SELECT InfoClassID FROM InfoClass WHERE value = ?"
    info_class_id = query_return_id(query, (info_class,), con)

    # Insert a new Info record with the retrieved or default IDs
    query = (
        "INSERT INTO Info (title, release, difficulty, duration, type, class, trleID, trcustomsID) "
        "VALUES (?, ?, ?, ?, ?, ?, ?, ?)"
    )
    arg = (
        data.get('title'),          # Title of the level
        data.get('release'),        # Release date
        info_difficulty_id,         # Retrieved or default difficulty ID
        info_duration_id,           # Retrieved or default duration ID
        info_type_id,               # Retrieved or default type ID
        info_class_id,              # Retrieved or default class ID
        data.get('trle_id'),        # TRLE ID if available
        data.get('trcustoms_id')    # TRCustoms ID if available
    )

    return query_return_id(query, arg, con)


def add_tombll_json_to_database(data, con):
    """Insert level data and related details into the database.

    This function inserts a level record into the database and subsequently
    adds associated authors, genres, tags, zip files, screen, and large screens.

    Args:
        data (dict): A dictionary containing level data and related details.
        con (sqlite3.Connection): SQLite database connection.
    """
    # Insert level information and obtain the generated level ID
    level_id = add_level_to_database(data, con)

    # Add related information only if corresponding data is present
    if data.get('authors'):
        add_authors_to_database(data['authors'], level_id, con)
    if data.get('genres'):
        add_genres_to_database(data['genres'], level_id, con)
    if data.get('tags'):
        add_tags_to_database(data['tags'], level_id, con)
    if data.get('zip_files'):
        add_zip_files_to_database(data['zip_files'], level_id, con)

    # Single screen image, checked for None in the add_screen_to_database function
    add_screen_to_database(data.get('screen'), level_id, con)

    # Add large screens if they are provided
    if data.get('large_screens'):
        add_screens_to_database(data['large_screens'], level_id, con)


if __name__ == "__main__":
    if len(sys.argv) != 2:
        logging.error("Usage: python3 addData.py FILE.json")
        sys.exit(1)
    DATA = get_tombll_json(sys.argv[1])
    CON = sqlite3.connect(os.path.dirname(os.path.abspath(__file__)) + '/tombll.db')
    add_tombll_json_to_database(DATA, CON)
    CON.commit()
    CON.close()
