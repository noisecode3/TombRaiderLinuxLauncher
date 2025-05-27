"""
All Read queries.

Only pure queries and sometimes json data here.
If we can access the json array directly or use all of the data
equal to the make_trle_tombll_data() from the data_factory to access data, we do it here.
"""
import tombll_common
import data_factory


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


def database_author_ids(level_id, con):
    """
    Get author ID form level ID.

    This function queries the 'AuthorList' table using the given `level_id`
    to determine if the author is already present. It helps avoid inserting
    duplicates when updating.

    Args:
        level_id (int or str): The level ID to.
        con (sqlite3.Connection): An active SQLite database connection.

    Returns:
        list of int: ID numbers if the level exists in the database, None otherwise.
    """
    query = '''
        SELECT AuthorList.authorID FROM AuthorList
        JOIN Author ON AuthorList.authorID = Author.AuthorID
        WHERE AuthorList.levelID = ?
    '''
    return tombll_common.query_return_everything(query, (level_id, ), con)


def database_genre_ids(level_id, con):
    """
    Get genre ID form level ID.

    This function queries the 'GenreList' table using the given `level_id`
    to determine if the genre is already present. It helps avoid inserting
    duplicates when updating.

    Args:
        level_id (int or str): The level ID to.
        con (sqlite3.Connection): An active SQLite database connection.

    Returns:
        list of int: ID numbers if the level exists in the database, None otherwise.
    """
    query = '''
        SELECT GenreList.genreID FROM GenreList
        JOIN Genre ON GenreList.genreID = Genre.GenreID
        WHERE GenreList.levelID = ?
    '''
    return tombll_common.query_return_everything(query, (level_id, ), con)


def database_picture_ids(level_id, con):
    """
    Get a list of pictures entries associated with a specific level.

    This function queries the database to retrieve all unique pictures
    linked to the given level via the `Screens` relation. The results
    are fetched and returned.

    Args:
        level_id (int): The ID of the level for which to retrieve ZIP files.
        con (sqlite3.Connection): An active SQLite database connection.

    Returns:
        List of int: List of all pictures IDs.
    """
    query = '''
        SELECT Screens.pictureID
        FROM Level
        JOIN Screens ON Level.LevelID = Screens.levelID
        WHERE Level.LevelID = ?
    '''

    # Fetch all rows from the executed query
    return tombll_common.query_return_everything(query, (level_id, ), con)


def database_tag_ids(level_id, con):
    """
    Get tag ID form level ID.

    This function queries the 'TagList' table using the given `level_id`
    to determine if the tag is already present. It helps avoid inserting
    duplicates when updating.

    Args:
        level_id (int or str): The level ID to.
        con (sqlite3.Connection): An active SQLite database connection.

    Returns:
        list of int: ID numbers if the level exists in the database, None otherwise.
    """
    query = '''
        SELECT TagList.tagID FROM TagList
        JOIN Tag ON TagList.tagID = Tag.TagID
        WHERE TagList.levelID = ?
    '''
    return tombll_common.query_return_everything(query, (level_id, ), con)


def trle_page(offset, con, limit=20, sort_latest_first=False):
    """Get a trle page."""
    page = data_factory.make_trle_page_data()
    page['offset'] = offset

    order_direction = 1 if sort_latest_first else 0

    result = []
    page['records_total'] = tombll_common.query_return_everything(
        "SELECT COUNT(*) FROM Level",
        None,
        con)[0][0]

    result = tombll_common.query_return_everything("""
        SELECT
            Info.trleID,
            Author.value,
            Info.title,
            InfoDifficulty.value,
            InfoDuration.value,
            InfoClass.value,
            InfoType.value,
            Info.release
        FROM Info
        INNER JOIN Level ON (Info.InfoID = Level.infoID)
        INNER JOIN AuthorList ON (Level.LevelID = AuthorList.levelID)
        INNER JOIN Author ON (Author.AuthorID = AuthorList.authorID)
        LEFT JOIN InfoDifficulty ON (InfoDifficulty.InfoDifficultyID = Info.difficulty)
        LEFT JOIN InfoDuration ON (InfoDuration.InfoDurationID = Info.duration)
        INNER JOIN InfoType ON (InfoType.InfoTypeID = Info.type)
        LEFT JOIN InfoClass ON (InfoClass.InfoClassID = Info.class)
        GROUP BY Info.trleID  -- Group by the trleID to get unique records?? needed?
        ORDER BY
            CASE WHEN ? = 0 THEN Info.release END ASC,
            CASE WHEN ? = 1 THEN Info.release END DESC
        LIMIT ? OFFSET ?;
        """, (order_direction, order_direction, limit, offset), con)

    for row in result:
        level = data_factory.make_trle_level_data()
        level['trle_id'] = row[0]
        level['author'] = row[1]
        level['title'] = row[2]
        level['difficulty'] = row[3]
        level['duration'] = row[4]
        level['class'] = row[5]
        level['type'] = row[6]
        level['release'] = row[7]
        page['levels'].append(level)

    return page


def trle_cover_picture(trle_id, con):
    """Get TRLE cover picture."""
    return tombll_common.query_return_everything("""
        SELECT
            Picture.data
        FROM Info
        INNER JOIN Level ON (Info.InfoID = Level.infoID)
        INNER JOIN Screens ON (Level.LevelID = Screens.levelID)
        INNER JOIN Picture ON (Screens.pictureID = Picture.PictureID)
        WHERE Info.trleID = ? AND Screens.position = 0
        """, (trle_id, ), con)[0][0]
