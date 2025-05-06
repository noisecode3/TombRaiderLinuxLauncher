"""
All Delete queries.

Only pure queries and sometimes json data here.
If we can access the json array directly or use all of the data
equal to the make_trle_tombll_data() from the data_factory to access data, we do it here.
"""
import tombll_common


def database_picture_last(con):
    """
    Delete Picture records if it's not linked to any other level.

    Args:
        con (sqlite3.Connection): An open SQLite database connection.

    Returns:
        None
    """
    query = '''
        DELETE FROM Picture
        WHERE PictureID NOT IN (
            SELECT DISTINCT pictureID FROM Screens
        );
    '''
    tombll_common.query_run(query, None, con)


def database_zip_last(con):
    """
    Delete Zip records if it's not linked to any other level.

    Args:
        con (sqlite3.Connection): An open SQLite database connection.

    Returns:
        None
    """
    query = '''
        DELETE FROM Zip
        WHERE ZipID NOT IN (
            SELECT DISTINCT zipID FROM ZipList
        );
    '''
    tombll_common.query_run(query, None, con)


def database_tag_last(con):
    """
    Delete Tag records if it's not linked to any other level.

    Args:
        con (sqlite3.Connection): An open SQLite database connection.

    Returns:
        None
    """
    query = '''
        DELETE FROM Tag
        WHERE TagID NOT IN (
            SELECT DISTINCT tagID FROM TagList
        );
    '''
    tombll_common.query_run(query, None, con)


def database_genre_last(con):
    """
    Delete Genre records if it's not linked to any other level.

    Args:
        con (sqlite3.Connection): An open SQLite database connection.

    Returns:
        None
    """
    query = '''
        DELETE FROM Genre
        WHERE genreID NOT IN (
            SELECT DISTINCT genreID FROM GenreList
        );
    '''
    tombll_common.query_run(query, None, con)


def database_author_last(con):
    """
    Delete Author records if it's not linked to any other level.

    Args:
        con (sqlite3.Connection): An open SQLite database connection.

    Returns:
        None
    """
    query = '''
        DELETE FROM Author
        WHERE AuthorID NOT IN (
            SELECT DISTINCT authorID FROM AuthorList
        );
    '''
    tombll_common.query_run(query, None, con)


def database_level(level_id, con):
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
    # Delete the screen picture entries related to the level and potentially the last one
    tombll_common.query_run('DELETE FROM Screens WHERE levelID = ?;', (level_id, ), con)
    database_picture_last(con)

    # Delete the zip files entries related to the level and potentially the last one
    tombll_common.query_run('DELETE FROM ZipList WHERE levelID = ?;', (level_id, ), con)
    database_zip_last(con)

    # Delete the tag entries related to the level and potentially the last one
    tombll_common.query_run('DELETE FROM TagList WHERE levelID = ?;', (level_id, ), con)
    database_tag_last(con)

    # Delete the Genre entries related to the level and potentially the last one
    tombll_common.query_run('DELETE FROM GenreList WHERE levelID = ?;', (level_id, ), con)
    database_genre_last(con)

    # Delete the Author entries related to the level and potentially the last one
    tombll_common.query_run('DELETE FROM AuthorList WHERE levelID = ?;', (level_id, ), con)
    database_author_last(con)

    # Delete the Info row associated with the level
    query = '''
        DELETE FROM Info
        WHERE InfoID IN (
            SELECT infoID
            FROM Level WHERE LevelID = ?
        );
    '''
    tombll_common.query_run(query, (level_id, ), con)

    # Finally, delete the level itself
    tombll_common.query_run('DELETE FROM Level WHERE LevelID = ?;', (level_id, ), con)


def database_author(author_id, level_id, con):
    """
    Delete a Author only if it's linked to level ID.

    Deleting the entry in the AuthorList middle table.
    Then remove the corresponding Author record.

    Args:
        author_id (int): The ID of the author in Author table.
        level_id (int): The ID of the level in the Level table.
        con (sqlite3.Connection): An open SQLite database connection.

    Returns:
        None
    """
    # First, delete from AuthorList
    query = '''
        DELETE FROM AuthorList
        WHERE authorID = ? AND levelID = ?
    '''
    tombll_common.query_run(query, (author_id, level_id), con)

    # Then, try to delete from Author if it's not linked to any other level
    database_author_last(con)


def database_genre(genre_id, level_id, con):
    """
    Delete a Genre only if it's linked to level ID.

    Deleting the entry in the GenreList middle table.
    Then remove the corresponding genre record.

    Args:
        genre_id (int): The ID of the genre in Genre table.
        level_id (int): The ID of the level in the Level table.
        con (sqlite3.Connection): An open SQLite database connection.

    Returns:
        None
    """
    # First, delete from GenreList
    query = '''
        DELETE FROM GenreList
        WHERE genreID = ? AND levelID = ?
    '''
    tombll_common.query_run(query, (genre_id, level_id), con)

    # Then, try to delete from Genre if it's not linked to any other level
    database_genre_last(con)


def database_tag(tag_id, level_id, con):
    """
    Delete a Tag only if it's linked to level ID.

    Deleting the entry in the TagList middle table.
    Then remove the corresponding tag record.

    Args:
        tag_id (int): The ID of the tag in Tag table.
        level_id (int): The ID of the level in the Level table.
        con (sqlite3.Connection): An open SQLite database connection.

    Returns:
        None
    """
    # First, delete from TagList
    query = '''
        DELETE FROM TagList
        WHERE tagID = ? AND levelID = ?
    '''
    tombll_common.query_run(query, (tag_id, level_id), con)

    # Then, try to delete from Tag if it's not linked to any other level
    database_tag_last(con)


def database_picture(picture_id, level_id, con):
    """
    Delete a Picture only if it's linked to level ID.

    Deleting the entry in Screens middle table.
    Then remove the corresponding picture record.

    Args:
        picture_id (int): The ID of the picture in Picture table.
        level_id (int): The ID of the level in the Level table.
        con (sqlite3.Connection): An open SQLite database connection.

    Returns:
        None
    """
    # First, delete from Screens
    query = '''
        DELETE FROM Screens
        WHERE pictureID = ? AND levelID = ?
    '''
    tombll_common.query_run(query, (picture_id, level_id), con)

    # Then, try to delete from Picture if it's not linked to any other level
    database_picture_last(con)


def database_zip_file(zip_id, level_id, con):
    """
    Delete a Zip file only if it's linked to level ID.

    Deleting the entry in ZipList middle table
    Then remove the corresponding zip file record.

    Args:
        zip_id (int): The ID of the zip file in Zip table.
        level_id (int): The ID of the level in the Level table.
        con (sqlite3.Connection): An open SQLite database connection.

    Returns:
        None
    """
    # First, delete from ZipList
    query = '''
        DELETE FROM ZipList
        WHERE zipID = ? AND levelID = ?
    '''
    tombll_common.query_run(query, (zip_id, level_id), con)

    # Then, try to delete from Zip if it's not linked to any other level
    database_zip_last(con)
