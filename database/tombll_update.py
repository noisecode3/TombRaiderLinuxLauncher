"""
All Update queries.

Only pure queries and sometimes json data here.
If we can access the json array directly or use all of the data
equal to the make_trle_tombll_data() from the data_factory to access data, we do it here.
"""
import tombll_common


def database_level(data, level_id, con):
    """Update an existing Level entry and return its associated infoID.

    This function updates a Level record in the database with the provided 'body'
    and 'walkthrough' fields from the `data` dictionary. It targets the record
    identified by the given `level_id`.

    After the update, it retrieves and returns the associated `infoID` for further use.

    Args:
        data (dict): A dictionary containing level data, including optional
                     'body' and 'walkthrough' fields.
        level_id (int): The ID of the Level to be updated.
        con (sqlite3.Connection): SQLite database connection.

    Returns:
        int: The associated infoID of the updated Level.
    """
    # SQL query to update the Level record
    query = "UPDATE Level SET body = ?, walkthrough = ? WHERE LevelID = ?"

    # Prepare arguments for the update
    arg = (
        data.get('body'),
        data.get('walkthrough'),
        level_id
    )

    # Run the update query
    tombll_common.query_run(query, arg, con)

    # Retrieve the associated infoID
    query = "SELECT infoID FROM Level WHERE LevelID = ?"
    return tombll_common.query_return_id(query, (level_id,), con)


def database_info(data, info_id, con):
    """Update an existing Info record with new level metadata.

    This function updates a record in the Info table using the provided data dictionary.
    It looks up related IDs for difficulty, duration, type, and class based on their string
    values, and uses those IDs to update the corresponding fields.

    Args:
        data (dict): Dictionary containing level metadata, such as 'title', 'release',
                     'difficulty', 'duration', 'type', and 'class'.
        info_id (int): The InfoID of the record to update.
        con (sqlite3.Connection): SQLite database connection.
    """
    # Retrieve or assign InfoDifficultyID
    info_difficulty = data.get('difficulty')
    info_difficulty_id = None
    if info_difficulty:
        query = "SELECT InfoDifficultyID FROM InfoDifficulty WHERE value = ?"
        info_difficulty_id = tombll_common.query_return_id(query, (info_difficulty,), con)

    # Retrieve or assign InfoDurationID
    info_duration = data.get('duration')
    info_duration_id = None
    if info_duration:
        query = "SELECT InfoDurationID FROM InfoDuration WHERE value = ?"
        info_duration_id = tombll_common.query_return_id(query, (info_duration,), con)

    # Retrieve or assign InfoTypeID (allowing None)
    info_type = data.get('type') or None
    query = "SELECT InfoTypeID FROM InfoType WHERE value = ?"
    info_type_id = tombll_common.query_return_id(query, (info_type,), con)

    # Retrieve or assign InfoClassID (allowing None)
    info_class = data.get('class') or None
    query = "SELECT InfoClassID FROM InfoClass WHERE value = ?"
    info_class_id = tombll_common.query_return_id(query, (info_class,), con)

    # Update the existing Info record
    query = '''
        UPDATE Info
        SET title = ?, release = ?, difficulty = ?, duration = ?, type = ?, class = ?
        WHERE InfoID = ?
    '''
    arg = (
        data.get('title'),
        data.get('release'),
        info_difficulty_id,
        info_duration_id,
        info_type_id,
        info_class_id,
        info_id,
    )

    # Perform the update and return the same info_id
    tombll_common.query_run(query, arg, con)
