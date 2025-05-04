"""
All Delete queries.

Only pure queries and sometimes json data here.
If we can access the json array directly or use all of the data
equal to the make_trle_tombll_data() from the data_factory to access data, we do it here.
"""
import tombll_common


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
    tombll_common.query_run(query, (level_id, ), con)

    # Delete the screen entries related to the level
    tombll_common.query_run('DELETE FROM Screens WHERE levelID = ?;', (level_id, ), con)

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
    tombll_common.query_run(query, (level_id, ), con)

    # Delete the relationship between zip files and the level
    tombll_common.query_run('DELETE FROM ZipList WHERE levelID = ?;', (level_id, ), con)

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
    tombll_common.query_run(query, (level_id, ), con)

    # Delete the relationship between tags and the level
    tombll_common.query_run('DELETE FROM TagList WHERE levelID = ?;', (level_id, ), con)

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
    tombll_common.query_run(query, (level_id, ), con)

    # Delete the relationship between genres and the level
    tombll_common.query_run('DELETE FROM GenreList WHERE levelID = ?;', (level_id, ), con)

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
    tombll_common.query_run(query, (level_id, ), con)

    # Delete the relationship between authors and the level
    tombll_common.query_run('DELETE FROM AuthorList WHERE levelID = ?;', (level_id, ), con)

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

    After deleting the author then it remove the corresponding entry in the AuthorList middle table.

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

    # Then, try to delete from Genre if it's not linked to any other level
    query = '''
        DELETE FROM Author
        WHERE AuthorID IN (
            SELECT authorID
            FROM AuthorList
            WHERE authorID = ?
            GROUP BY authorID
            HAVING COUNT(DISTINCT levelID) = 0
        )
    '''
    tombll_common.query_run(query, (author_id, ), con)


def database_genre(genre_id, level_id, con):
    """
    Delete a Genre only if it's linked to level ID.

    After deleting the genre then it remove the corresponding entry in the GenreList middle table.

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
    query = '''
        DELETE FROM Genre
        WHERE GenreID IN (
            SELECT genreID
            FROM GenreList
            WHERE genreID = ?
            GROUP BY genreID
            HAVING COUNT(DISTINCT levelID) = 0
        )
    '''
    tombll_common.query_run(query, (genre_id, ), con)


def database_tag(tag_id, level_id, con):
    """
    Delete a Tag only if it's linked to level ID.

    After deleting the tag then it remove the corresponding entry in the TagList middle table.

    Args:
        tag_id (int): The ID of the tag in Tag table.
        level_id (int): The ID of the level in the Level table.
        con (sqlite3.Connection): An open SQLite database connection.

    Returns:
        None
    """
    # First, delete from ZipList
    query = '''
        DELETE FROM TagList
        WHERE tagID = ? AND levelID = ?
    '''
    tombll_common.query_run(query, (tag_id, level_id), con)

    # Then, try to delete from Tag if it's not linked to any other level
    query = '''
        DELETE FROM Tag
        WHERE TagID IN (
            SELECT tagID
            FROM TagList
            WHERE tagID = ?
            GROUP BY tagID
            HAVING COUNT(DISTINCT levelID) = 0
        )
    '''
    tombll_common.query_run(query, (tag_id, ), con)


def database_picture(picture_id, level_id, con):
    """
    Delete a Picture only if it's linked to level ID.

    After deleting the picture then it remove the corresponding entry in Screens middle table.

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
    query = '''
        DELETE FROM Picture
        WHERE PictureID IN (
            SELECT pictureID
            FROM Screens
            WHERE pictureID = ?
            GROUP BY pictureID
            HAVING COUNT(DISTINCT levelID) = 0
        )
    '''
    tombll_common.query_run(query, (picture_id, ), con)


def database_zip_file(zip_id, level_id, con):
    """
    Delete a Zip only if it's linked to level ID.

    After deleting the zip file then it remove the corresponding entry in ZipList middle table.

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
    query = '''
        DELETE FROM Zip
        WHERE ZipID IN (
            SELECT zipID
            FROM ZipList
            WHERE zipID = ?
            GROUP BY zipID
            HAVING COUNT(DISTINCT levelID) = 0
        )
    '''
    tombll_common.query_run(query, (zip_id, ), con)
