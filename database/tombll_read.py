"""
All Read queries.

Only pure queries and sometimes json data here.
If we can access the json array directly or use all of the data
equal to the make_trle_tombll_data() from the data_factory to access data, we do it here.
"""
import tombll_common


def database_level_list(con):
    """
    Retrieve and returns a list of level information.

    This function executes a SQL query that joins multiple tables
    (`Level`, `Info`, `AuthorList`, and `Author`) to get the relevant
    data for each level. It then iterates over the query results and prints each row.

    Args:
        con (sqlite3.Connection): An open SQLite database connection.

    Returns:
        List: List of all levels.
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
    return tombll_common.query_return_everything(query, None, con)


def database_zip_list(level_id, con):
    """
    Get a list of ZIP file entries associated with a specific level.

    This function queries the database to retrieve all unique ZIP files
    linked to the given level via the `ZipList` relation. The results
    are fetched and returned.

    Args:
        level_id (int): The ID of the level for which to retrieve ZIP files.
        con (sqlite3.Connection): An active SQLite database connection.

    Returns:
        None: List of all zip files.
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
    return tombll_common.query_return_everything(query, (level_id, ), con)


def database_picture_list(level_id, con):
    """
    Get a list of pictures entries associated with a specific level.

    This function queries the database to retrieve all unique pictures
    linked to the given level via the `Screens` relation. The results
    are fetched and returned.

    Args:
        level_id (int): The ID of the level for which to retrieve ZIP files.
        con (sqlite3.Connection): An active SQLite database connection.

    Returns:
        None: List of all pictures.
    """
    query = '''
        SELECT Screens.pictureID
        FROM Level
        JOIN Screens ON Level.LevelID = Screens.levelID
        WHERE Level.LevelID = ?
    '''

    # Fetch all rows from the executed query
    return tombll_common.query_return_everything(query, (level_id, ), con)


def database_level_id(trle_id, con):
    """
    Get level ID form TRLE lid.

    This function queries the 'Info' table using the given `trle_id` to determine
    if the level is already present. It helps avoid inserting duplicates.

    Args:
        trle_id (int or str): The TRLE level ID to check for existence.
        con (sqlite3.Connection): An active SQLite database connection.

    Returns:
        int: ID number if the level ID exists in the database, None otherwise.
    """
    query = '''
        SELECT Level.LevelID FROM Level
        JOIN Info ON Level.infoID = Info.InfoID
        WHERE Info.trleID = ?
    '''
    return tombll_common.query_return_id(query, (trle_id, ), con)


def database_author_id(level_id, author_name, con):
    """
    Get author ID form level ID and author name.

    This function queries the 'AuthorList' table using the given `level_id`
    and author_name to determine if the author is already present.
    It helps avoid inserting duplicates.

    Args:
        level_id (int or str): The level ID to.
        author_name (str): The TRLE author name.
        con (sqlite3.Connection): An active SQLite database connection.

    Returns:
        int: ID number if the author ID exists in the database, None otherwise.
    """
    query = '''
        SELECT AuthorList.authorID FROM AuthorList
        JOIN Author ON AuthorList.authorID = Author.AuthorID
        WHERE AuthorList.levelID = ? AND Author.value = ?
    '''
    return tombll_common.query_return_id(query, (level_id, author_name), con)


def database_author_list(level_id, con):
    """
    Get a list of author entries associated with a specific level.

    This function queries the database to retrieve all unique authors
    linked to the given level via the `AuthorList` relation. The results
    are fetched and returned.

    Args:
        level_id (int): The ID of the level for which to retrieve authors.
        con (sqlite3.Connection): An active SQLite database connection.

    Returns:
        None: List of all authors.
    """
    query = '''
        SELECT Author.value FROM AuthorList
        JOIN Author ON AuthorList.authorID = Author.AuthorID
        WHERE AuthorList.levelID = ?
    '''

    # Fetch all rows from the executed query
    return tombll_common.query_return_everything(query, (level_id, ), con)
