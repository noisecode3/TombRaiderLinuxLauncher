"""
All Insert queries.

Only pure queries and sometimes json data here.
"""
from __future__ import annotations

import logging

import tombll_common
from data_factory import Picture, TrleTombll, ZipFile

# Set up logging
logging.basicConfig(level=logging.DEBUG, format='%(asctime)s %(levelname)s:%(message)s')
logging.getLogger("requests").setLevel(logging.DEBUG)
logger = logging.getLogger(__name__)


def database_author(author, level_id, con):
    """Add a author to the database, linking with a specific level.

    This function checks if a author already exists in the database.
    If it does it will use this ID number. If not, it inserts the author.
    Then creates an entry in the AuthorList table that will link the author
    to the specified level ID.

    Args:
        author (str): Author name value to insert.
        level_id (int): The ID of the level to link author with.
        con (sqlite3.Connection): SQLite database connection.

    Returns:
        int: The ID of the newly inserted author or existing one.
    """
    # SQL queries for selecting, inserting, and linking author
    query_select_id = "SELECT AuthorID FROM Author WHERE value = ?"
    query_insert = "INSERT INTO Author (value) VALUES (?)"
    query_insert_middle = "INSERT OR IGNORE INTO AuthorList (authorID, levelID) VALUES (?, ?)"

    # Try to get the existing author ID; if none, insert a new author and get its ID
    author_id = tombll_common.query_return_id(query_select_id, (author,), con)
    if author_id is None:
        author_id = tombll_common.query_return_id(query_insert, (author,), con)

    # Link the author with the level in AuthorList table
    tombll_common.query_run(query_insert_middle, (author_id, level_id), con)
    return author_id


def database_authors(authors_array, level_id, con):
    """Add authors array to the database, linking them with a specific level.

    Args:
        authors_array (list of str): List of author names.
        level_id (int): The ID of the level to link authors with.
        con (sqlite3.Connection): SQLite database connection.

    Returns:
        set of int: The set of IDs for the newly inserted authors or the existing one.
    """
    ids = set()
    for author in authors_array:
        ids.add(database_author(author, level_id, con))
    return ids


def database_genre(genre, level_id, con):
    """Add a genre to the database, linking them with a specific level.

    This function checks if the genre already exists in the database.
    If it does it will use this ID number. If not, it inserts the genre.
    Then creates an entry in the GenreList table that will link the genre
    to the specified level.

    Args:
        genre (str): Genre names.
        level_id (int): The ID of the level to link genre with.
        con (sqlite3.Connection): SQLite database connection.

    Returns:
        int: The ID of the newly inserted genre of existing one.
    """
    # SQL queries for selecting, inserting, and linking genre
    query_select_id = "SELECT GenreID FROM Genre WHERE value = ?"
    query_insert = "INSERT INTO Genre (value) VALUES (?)"
    query_insert_middle = "INSERT OR IGNORE INTO GenreList (genreID, levelID) VALUES (?, ?)"

    # Try to get the existing genre ID; if none, insert a new genre and get its ID
    genre_id = tombll_common.query_return_id(query_select_id, (genre,), con)
    if genre_id is None:
        genre_id = tombll_common.query_return_id(query_insert, (genre,), con)

    # Link the genre with the level in GenreList table
    tombll_common.query_run(query_insert_middle, (genre_id, level_id), con)
    return genre_id


def database_genres(genres_array, level_id, con):
    """Add genres to the database, linking them with a specific level.

    Args:
        genres_array (list of str): List of genre names.
        level_id (int): The ID of the level to link genres with.
        con (sqlite3.Connection): SQLite database connection.

    Returns:
        set of int: The set of IDs for the newly inserted genres or the existing one.
    """
    for genre in genres_array:
        database_genre(genre, level_id, con)


def database_tag(tag, level_id, con):
    """Add a tag to the database, linking them with a specific level.

    This function checks if the tag already exists in the database.
    If it does it will use this ID number. If not, it inserts the tag.
    Then creates an entry in the TagList table that will link the tag
    to the specified level.

    Args:
        tags_array (list of str): List of tag names.
        level_id (int): The ID of the level to link tags with.
        con (sqlite3.Connection): SQLite database connection.

    Returns:
        int: The ID of the newly inserted tag of existing one.
    """
    # SQL queries for selecting, inserting, and linking tags
    query_select_id = "SELECT TagID FROM Tag WHERE value = ?"
    query_insert = "INSERT INTO Tag (value) VALUES (?)"
    query_insert_middle = "INSERT OR IGNORE INTO TagList (tagID, levelID) VALUES (?, ?)"

    # Try to get the existing tag ID; if not found, insert a new tag and get its ID
    tag_id = tombll_common.query_return_id(query_select_id, (tag,), con)
    if tag_id is None:
        tag_id = tombll_common.query_return_id(query_insert, (tag,), con)

    # Link the tag with the level in TagList table
    tombll_common.query_run(query_insert_middle, (tag_id, level_id), con)
    return tag_id


def database_tags(tags_array, level_id, con):
    """Add tags to the database, linking them with a specific level.

    Args:
        tags_array (list of str): List of tag names.
        level_id (int): The ID of the level to link tags with.
        con (sqlite3.Connection): SQLite database connection.

    Returns:
        set of int: The set of IDs for the newly inserted tags or the existing one.
    """
    for tag in tags_array:
        database_tag(tag, level_id, con)


def database_zip_file(zip_file, level_id, con):
    """Add a ZIP file to the database, linking it with a specific level.

    Args:
        zip_file (ZipFile): ZIP file details (name, size, md5, url, version, release).
        level_id (int): The ID of the level to link the ZIP file with.
        con (sqlite3.Connection): SQLite database connection.

    Returns:
        int: The ID of the inserted or existing zip file.
    """
    query_select_id = "SELECT ZipID FROM Zip WHERE name = ?"
    query_insert = '''
        INSERT INTO Zip (name, size, md5sum, url, version, release)
        VALUES (?, ?, ?, ?, ?, ?)
    '''

    # Convert empty values to None
    insert_args = (
        tombll_common.make_empty_null(zip_file.name),
        tombll_common.make_empty_null(zip_file.size),
        tombll_common.make_empty_null(zip_file.md5),
        tombll_common.make_empty_null(zip_file.url),
        tombll_common.make_empty_null(zip_file.version),
        tombll_common.make_empty_null(zip_file.release),
    )

    zip_id = tombll_common.query_return_id(query_select_id, (zip_file.name,), con)
    if zip_id is None:
        zip_id = tombll_common.query_return_id(query_insert, insert_args, con)

    query_insert_middle = "INSERT OR IGNORE INTO ZipList (zipID, levelID) VALUES (?, ?)"
    tombll_common.query_run(query_insert_middle, (zip_id, level_id), con)
    return zip_id


def database_zip_files(zip_files: list[ZipFile] | None, level_id, con):
    """Add ZIP files to the database, linking them with a specific level.

    Args:
        zip_files (list[ZipFile] | None): ZIP file details; empty or None
            means nothing to do.
        level_id (int): The ID of the level to link ZIP files with.
        con (sqlite3.Connection): SQLite database connection.

    Returns:
        set[int]: IDs of the inserted or existing zip files; empty set if none.
    """
    return {database_zip_file(zip_file, level_id, con) for zip_file in zip_files or ()}


def database_screen(picture, level_id, con):
    """Add a screen image to the database and link it to a specific level.

    Args:
        picture (Picture): Picture with data (bytes), md5sum (str), position (int).
        level_id (int): The ID of the level to link the screen image with.
        con (sqlite3.Connection): SQLite database connection.

    Returns:
        int: The ID of the inserted or existing picture.
    """
    query_select_id = "SELECT PictureID FROM Picture WHERE md5sum = ?"
    query_insert = "INSERT OR IGNORE INTO Picture (md5sum, data) VALUES (?, ?)"

    picture_id = tombll_common.query_return_id(query_select_id, (picture.md5sum,), con)
    if picture_id is None:
        picture_id = tombll_common.query_return_id(
            query_insert, (picture.md5sum, picture.data), con)

    query_insert_screen = '''
        INSERT OR IGNORE
        INTO Screens (pictureID, levelID, position)
        VALUES (?, ?, ?)
    '''
    tombll_common.query_run(
        query_insert_screen, (picture_id, level_id, picture.position), con)
    return picture_id


def database_screens(pictures: list[Picture] | None, level_id, con):
    """Add multiple screen images to the database and link them to a level.

    Args:
        pictures (list[Picture] | None): Screen images to insert; empty or
            None means nothing to do.
        level_id (int): The ID of the level to associate the images with.
        con (sqlite3.Connection): SQLite database connection.

    Returns:
        set[int]: IDs of the inserted or existing pictures; empty set if none.
    """
    return {database_screen(picture, level_id, con) for picture in pictures or ()}


def database_level(data: TrleTombll, con):
    """Insert a level entry into the Level table and return its ID.

    Args:
        data (TrleTombll): Level data, including body and walkthrough.
        con (sqlite3.Connection): SQLite database connection.

    Returns:
        int: The ID of the newly inserted level.
    """
    query = "INSERT INTO Level (body, walkthrough, infoID) VALUES (?, ?, ?)"
    arg = (data.body, data.walkthrough, database_info(data, con))

    level_id = tombll_common.query_return_id(query, arg, con)
    logger.info("Current tombll level_id: %s", level_id)
    return level_id


def database_info(data: TrleTombll, con):
    """Insert an Info record, resolving attribute IDs first.

    Args:
        data (TrleTombll): Level information attributes.
        con (sqlite3.Connection): SQLite database connection.

    Returns:
        int: The ID of the newly inserted Info record.
    """
    info_difficulty_id = None
    if data.difficulty:
        query = "SELECT InfoDifficultyID FROM InfoDifficulty WHERE value = ?"
        info_difficulty_id = tombll_common.query_return_id(
            query, (data.difficulty,), con)

    info_duration_id = None
    if data.duration:
        query = "SELECT InfoDurationID FROM InfoDuration WHERE value = ?"
        info_duration_id = tombll_common.query_return_id(
            query, (data.duration,), con)

    # Empty string -> None (allowed)
    query = "SELECT InfoTypeID FROM InfoType WHERE value = ?"
    info_type_id = tombll_common.query_return_id(query, (data.type or None,), con)

    query = "SELECT InfoClassID FROM InfoClass WHERE value = ?"
    info_class_id = tombll_common.query_return_id(query, (data.class_ or None,), con)

    query = '''
        INSERT INTO Info (title, release, difficulty, duration, type, class, trleID, trcustomsID)
        VALUES (?, ?, ?, ?, ?, ?, ?, ?)
    '''
    arg = (
        data.title,
        data.release,
        info_difficulty_id,
        info_duration_id,
        info_type_id,
        info_class_id,
        tombll_common.make_empty_null(data.trle_id),
        tombll_common.make_empty_null(data.trcustoms_id),
    )
    return tombll_common.query_return_id(query, arg, con)
