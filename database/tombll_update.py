"""
All Update queries.

Only pure queries and sometimes json data here.
"""
import tombll_common
from data_factory import TrleTombll


def database_level(data: TrleTombll, level_id, con):
    """Update an existing Level entry and return its associated infoID.

    Args:
        data (TrleTombll): Level data; only 'body' and 'walkthrough' are used.
        level_id (int): The ID of the Level to be updated.
        con (sqlite3.Connection): SQLite database connection.

    Returns:
        int: The associated infoID of the updated Level.
    """
    query = "UPDATE Level SET body = ?, walkthrough = ? WHERE LevelID = ?"
    arg = (data.body, data.walkthrough, level_id)
    tombll_common.query_run(query, arg, con)

    query = "SELECT infoID FROM Level WHERE LevelID = ?"
    return tombll_common.query_return_id(query, (level_id,), con)


def database_info(data: TrleTombll, info_id, con):
    """Update an existing Info record with new level metadata.

    Args:
        data (TrleTombll): Level metadata (title, release, difficulty,
                           duration, type, class_).
        info_id (int): The InfoID of the record to update.
        con (sqlite3.Connection): SQLite database connection.
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
    info_type = data.type or None
    query = "SELECT InfoTypeID FROM InfoType WHERE value = ?"
    info_type_id = tombll_common.query_return_id(query, (info_type,), con)

    info_class = data.class_ or None
    query = "SELECT InfoClassID FROM InfoClass WHERE value = ?"
    info_class_id = tombll_common.query_return_id(query, (info_class,), con)

    query = '''
        UPDATE Info
        SET title = ?, release = ?, difficulty = ?, duration = ?, type = ?, class = ?
        WHERE InfoID = ?
    '''
    arg = (
        data.title,
        data.release,
        info_difficulty_id,
        info_duration_id,
        info_type_id,
        info_class_id,
        info_id,
    )
    tombll_common.query_run(query, arg, con)
