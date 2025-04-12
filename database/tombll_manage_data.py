"""Manage data for the tombll database."""
import os
import sys
import sqlite3

import tombll_common
import data_factory


def print_info():
    """Print help info."""
    print("Usage: python3 tombll_manage_data.py [options]")
    print("  Options:")
    print("      -l List level records")
    print("      -r [Level.LevelID] Remove one level")
    print("      -ad [Level.LevelID, Zip.name, Zip.size, Zip.md5sum]")
    print("             Optional and in order [Zip.url, Zip.version, Zip.release] Add a zip file")
    print("      -ld [Level.LevelID] List download records")


def print_list(con):
    """
    Retrieve and prints a list of level information.

    This function executes a SQL query that joins multiple tables
    (`Level`, `Info`, `AuthorList`, and `Author`) to get the relevant
    data for each level. It then iterates over the query results and prints each row.

    Args:
        con (sqlite3.Connection): An open SQLite database connection.

    Returns:
        None: This function does not return any value.
        It prints the results directly to the console.

    Example:
        print_list(con)  # Prints level information from the database.
    """
    query = '''
        SELECT Level.LevelID, Info.Title, Author.value
        FROM Level
        JOIN Info ON Level.infoID = Info.InfoID
        JOIN AuthorList ON Level.LevelID = AuthorList.levelID
        JOIN Author ON AuthorList.authorID = Author.AuthorID
        GROUP BY Level.LevelID
    '''

    # Fetch all rows from the executed query
    results = tombll_common.query_return_everything(query, None, con)

    # Iterate over the results and print each row
    for row in results:
        print(row)


def print_download_list(level_id, con):
    """
    Print a list of ZIP file entries associated with a specific level.

    This function queries the database to retrieve all unique ZIP files
    linked to the given level via the `ZipList` relation. The results
    are fetched and printed row by row.

    Args:
        level_id (int): The ID of the level for which to retrieve ZIP files.
        con (sqlite3.Connection): An active SQLite database connection.

    Returns:
        None: This function prints the query results to the console.

    Example:
        print_download_list(1, con)  # Prints all ZIP files linked to level 1.
    """
    query = '''
        SELECT Zip.*
        FROM Level
        JOIN ZipList ON Level.LevelID = ZipList.levelID
        JOIN Zip ON ZipList.zipID = Zip.ZipID
        WHERE Level.LevelID = ?
        GROUP BY Zip.ZipID
    '''

    # Fetch all rows from the executed query
    results = tombll_common.query_return_everything(query, (level_id,), con)

    # Print each row in the result
    for row in results:
        print(row)


def add_zip_to_level(level_id, data, con):
    """
    Add a new ZIP file and links it to a level via ZipList.

    Args:
        level_id (int): ID of the level to link the ZIP to.
        data (dict):
            name (str): ZIP file name (must be unique).
            size (float): File size.
            md5sum (str): MD5 checksum of the file.
            url (str): Download URL (can be None).
            version (int): Version number (can be None).
            release (str): Release date in ISO format (can be None).
        con (sqlite3.Connection): Open database connection.
    """
    # Insert ZIP entry into Zip table
    zip_query = '''
        INSERT INTO Zip (name, size, md5sum, url, version, release)
        VALUES (?, ?, ?, ?, ?, ?)
    '''
    arg = (data['name'], data['size'], data['md5'],
           data['url'], data['version'], data['release'])
    zip_id = tombll_common.query_return_id(zip_query, arg, con)

    # Link ZIP to Level in ZipList
    link_query = '''
        INSERT INTO ZipList (zipID, levelID)
        VALUES (?, ?)
    '''
    tombll_common.query_run(link_query, (zip_id, level_id), con)


def remove_level(level_id, con):
    """
    Remove a level and all its associated data from the database.

    This function executes a series of SQL queries to remove all data related to a level:
    including entries in `Picture`, `Zip`, `Tag`, `Genre`, `Author`, and their respective
    middle tables (`Screens`, `ZipList`, `TagList`, `GenreList`, `AuthorList`), as well
    as its `Info` and `Level` records.

    Args:
        level_id (int): The ID of the level in the Level table.
        con (sqlite3.Connection): An open SQLite database connection.

    Returns:
        None

    Example:
        remove_level(6)  # Removes level with ID 6 from the database.
    """
    # Delete pictures associated with the level, if they are used only by this level
    query = '''
        DELETE FROM Picture
        WHERE PictureID IN (
            SELECT pictureID
            FROM Screens
            GROUP BY pictureID
            HAVING COUNT(DISTINCT levelID) = 1 AND MAX(levelID) = ?
        );
    '''
    tombll_common.query_run(query, (level_id,), con)

    # Delete the screen entries related to the level
    tombll_common.query_run('DELETE FROM Screens WHERE levelID = ?;', (level_id,), con)

    # Delete zip files associated with the level, if they are used only by this level
    query = '''
        DELETE FROM Zip
        WHERE ZipID IN (
            SELECT zipID
            FROM ZipList
            GROUP BY zipID
            HAVING COUNT(DISTINCT levelID) = 1 AND MAX(levelID) = ?
        );
    '''
    tombll_common.query_run(query, (level_id,), con)

    # Delete the relationship between zip files and the level
    tombll_common.query_run('DELETE FROM ZipList WHERE levelID = ?;', (level_id,), con)

    # Delete tags associated with the level, if they are used only by this level
    query = '''
        DELETE FROM Tag
        WHERE TagID IN (
            SELECT tagID
            FROM TagList
            GROUP BY tagID
            HAVING COUNT(DISTINCT levelID) = 1 AND MAX(levelID) = ?
        );
    '''
    tombll_common.query_run(query, (level_id,), con)

    # Delete the relationship between tags and the level
    tombll_common.query_run('DELETE FROM TagList WHERE levelID = ?;', (level_id,), con)

    # Delete genres associated with the level, if they are used only by this level
    query = '''
        DELETE FROM Genre
        WHERE genreID IN (
            SELECT genreID
            FROM GenreList
            GROUP BY genreID
            HAVING COUNT(DISTINCT levelID) = 1 AND MAX(levelID) = ?
        );
    '''
    tombll_common.query_run(query, (level_id,), con)

    # Delete the relationship between genres and the level
    tombll_common.query_run('DELETE FROM GenreList WHERE levelID = ?;', (level_id,), con)

    # Delete authors only used by this level, if it's the last one associated with them
    query = '''
        DELETE FROM Author
        WHERE AuthorID IN (
            SELECT authorID
            FROM AuthorList
            GROUP BY authorID
            HAVING COUNT(DISTINCT levelID) = 1 AND MAX(levelID) = ?
        );
    '''
    tombll_common.query_run(query, (level_id,), con)

    # Delete the relationship between authors and the level
    tombll_common.query_run('DELETE FROM AuthorList WHERE levelID = ?;', (level_id,), con)

    # Delete the Info row associated with the level
    query = '''
        DELETE FROM Info
        WHERE InfoID IN (
            SELECT infoID
            FROM Level WHERE LevelID = ?
        );
    '''
    tombll_common.query_run(query, (level_id,), con)

    # Finally, delete the level itself
    tombll_common.query_run('DELETE FROM Level WHERE LevelID = ?;', (level_id,), con)


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
    elif (sys.argv[1] == "-r" and number_of_argument == 3):
        cur = main_con.cursor()
        cur.execute("BEGIN;")
        remove_level(sys.argv[2], main_con)
        main_con.commit()
        print(f"Level {sys.argv[2]} and related authors removed successfully.")
    elif (sys.argv[1] == "-ld" and number_of_argument == 3):
        print_download_list(sys.argv[2], main_con)
    elif (sys.argv[1] == "-ad" and number_of_argument >= 5):
        main_data = data_factory.make_zip_file()
        main_data["name"] = sys.argv[3]
        main_data["size"] = sys.argv[4]
        main_data["md5"] = sys.argv[5]
        main_data["url"] = sys.argv[6] if number_of_argument == 7 else None
        main_data["release"] = sys.argv[7] if number_of_argument == 8 else None
        main_data["version"] = sys.argv[8] if number_of_argument == 9 else None
        add_zip_to_level(sys.argv[2], main_data, main_con)
        main_con.commit()
    else:
        print_info()
    main_con.close()
