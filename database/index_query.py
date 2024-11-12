"""Insert or update all index data"""
import os
import sys
import sqlite3
import data_factory

# Set the working directory to the script's location to ensure relative paths work correctly
os.chdir(os.path.dirname(os.path.abspath(__file__)))


def check_trle_doubles():
    """
    Checks for duplicate entries in the Trle table.

    This function connects to the 'index.db' SQLite database and queries the Trle table
    to identify any duplicate records based on certain fields (trleExternId, author,
    title, difficulty, duration, class, type, and release). It groups the results by
    these fields and counts occurrences. If any grouped records appear more than once,
    they are considered duplicates, and those entries are returned and printed.
    """
    # Connect to the SQLite database
    connection = sqlite3.connect('index.db')
    cursor = connection.cursor()

    # Execute query to find duplicates
    result = query_return_fetchall("""
        SELECT
            tr.trleExternId,
            tr.author,
            tr.title,
            tr.difficulty,
            tr.duration,
            tr.class,
            tr.type,
            tr.release,
            COUNT(*) AS record_count
        FROM Trle AS tr
        GROUP BY
            tr.trleExternId,
            tr.author,
            tr.title,
            tr.difficulty,
            tr.duration,
            tr.class,
            tr.type,
            tr.release
        HAVING COUNT(*) > 1  -- Only include groups with more than one record
        ORDER BY record_count DESC;
    """, None, cursor)

    # Print the results, which includes any duplicate records found
    print(result)

    # Close the database connection
    connection.close()


def query_return_fetchall(query, params, cursor):
    """
    Executes a SELECT query and fetches all results from the database.

    Parameters:
    - query (str): The SQL query string to execute. Must be a SELECT query.
    - params (tuple or None): Parameters to bind to the query, or None if there are no parameters.
    - cursor (sqlite3.Cursor): The database cursor used to execute the query.

    Returns:
    - list: The result set from the query as a list of tuples.

    Raises:
    - Exits the program if the query is not a SELECT statement or if a database error occurs.
    """
    try:
        # Check if the query is a SELECT statement
        if not query.strip().upper().startswith("SELECT"):
            print("You need to use SELECT with this function")
            sys.exit(1)

        # Execute the query with or without parameters
        if not params:
            cursor.execute(query)
        else:
            cursor.execute(query, params)

        # Fetch and return all rows from the executed query
        return cursor.fetchall()

    except sqlite3.DatabaseError as db_error:
        # Handle database errors by printing an error message and exiting
        print(f"Database error occurred: {db_error}")
        sys.exit(1)


def query_return_id(query, params, cursor):
    """
    Executes a query and returns an ID if available.

    Parameters:
    - query (str): The SQL query to execute.
    - params (tuple): Parameters to bind to the query.
    - cursor (sqlite3.Cursor): The database cursor used to execute the query.

    Returns:
    - int or None: The last inserted row ID if an INSERT query was run,
                   or the first integer result from a SELECT query, if available.
                   Returns None if no valid result is found.

    Raises:
    - Exits the program if an integrity or other database error occurs.
    """
    try:
        # Execute the query with parameters
        cursor.execute(query, params)

        # If the query is an INSERT statement, return the last inserted row ID
        if query.strip().upper().startswith("INSERT"):
            return cursor.lastrowid

        # Fetch one result for non-INSERT queries
        result = cursor.fetchone()

        # Return the first element of the result if it's a non-negative integer
        if result and isinstance(result[0], int) and result[0] >= 0:
            return result[0]

        # Return None if no valid result is found
        return None

    except sqlite3.IntegrityError as integrity_error:
        # Handle integrity errors (e.g., unique constraint violations)
        print(f"Integrity error occurred with parameters: {params}\n{integrity_error}")
        sys.exit(1)

    except sqlite3.DatabaseError as db_error:
        # Handle general database errors
        print(f"Database error occurred: {db_error}")
        sys.exit(1)


def query_run(query, params, cursor):
    """
    Executes a query that does not return results (e.g., UPDATE, DELETE, or non-returning INSERT).

    Parameters:
    - query (str): The SQL query to execute.
    - params (tuple): Parameters to bind to the query.
    - cursor (sqlite3.Cursor): The database cursor used to execute the query.

    Raises:
    - Exits the program if an integrity or other database error occurs.
    """
    try:
        # Execute the query with parameters
        cursor.execute(query, params)

    # Handle specific database errors
    except sqlite3.IntegrityError as integrity_error:
        # Catch integrity-related issues, such as unique constraint violations
        print(f"Integrity error occurred with parameters: {params}\n{integrity_error}")
        sys.exit(1)

    except sqlite3.DatabaseError as db_error:
        # Catch any other database errors
        print(f"Database error occurred: {db_error}")
        sys.exit(1)


def insert_trle_page(page):
    """Insert TRLE page data into the database."""
    levels = page.get('levels')
    if not levels:
        print("Empty page!")
        return

    connection = sqlite3.connect('index.db')
    cursor = connection.cursor()

    for level in levels:
        cursor.execute('BEGIN TRANSACTION;')

        # Insert or retrieve Author
        author_id = query_return_id(
            "SELECT AuthorID FROM Author WHERE value = ?",
            (level['author'],),
            cursor
        )
        if author_id is None:
            author_id = query_return_id(
                "INSERT OR IGNORE INTO Author (value) VALUES (?)",
                (level['author'],),
                cursor
            )

        # Insert or retrieve Title
        title_id = query_return_id(
            "SELECT TitleID FROM Title WHERE value = ?",
            (level['title'],),
            cursor
        )

        if title_id is None:
            title_id = query_return_id(
                "INSERT OR IGNORE INTO Title (value) VALUES (?)",
                (level['title'],),
                cursor
            )

        # Retrieve or set Difficulty
        difficulty_id = None if not level['difficulty'] else query_return_id(
            "SELECT DifficultyID FROM Difficulty WHERE trle = ?",
            (level['difficulty'],),
            cursor
        )

        # Retrieve or set Duration
        duration_id = None if not level['duration'] else query_return_id(
            "SELECT DurationID FROM Duration WHERE trle = ?",
            (level['duration'],),
            cursor
        )

        # Retrieve or set Class
        class_id = None if not level['class'] else query_return_id(
            "SELECT ClassID FROM Class WHERE value = ?",
            (level['class'],),
            cursor
        )

        # Retrieve or set Type (mandatory)
        if level['type']:
            type_id = query_return_id(
                "SELECT TypeID FROM Type WHERE value = ?",
                (level['type'],),
                cursor
            )
        else:
            print(f"Missing type for level ID: {level['trle_id']}. Skipping.")
            continue

        in_db = query_return_fetchall("""
            SELECT *
            FROM Trle
            WHERE trleExternId = ?
                AND author = ?
                AND title = ?
                AND (difficulty IS ? OR difficulty = ?)
                AND (duration IS ? OR duration = ?)
                AND (class IS ? OR class = ?)
                AND type = ?
                AND release = ?;
        """, (
            level['trle_id'], author_id, title_id,
            difficulty_id, difficulty_id,
            duration_id, duration_id,
            class_id, class_id,
            type_id, level['release']
        ), cursor)

        if not in_db:
            # Insert into Trle table
            query_insert = """
            INSERT OR IGNORE INTO Trle (
                trleExternId, author, title, difficulty, duration, class, type, release
            ) VALUES (?,?,?,?,?,?,?,?)
            """
            query_run(query_insert, (
                level['trle_id'],
                author_id,
                title_id,
                difficulty_id,
                duration_id,
                class_id,
                type_id,
                level['release']
            ), cursor)
            cursor.execute('COMMIT;')
        else:
            cursor.execute('ROLLBACK;')
            continue

    connection.commit()
    connection.close()


def insert_trcustoms_page(page):
    """Take the defined trcustom page data and insert every record in the database."""
    if not page.get('levels'):
        print("Empty page!")
        return

    connection = sqlite3.connect('index.db')
    cursor = connection.cursor()

    def get_or_insert(query_select, query_insert, value):
        record_id = query_return_id(query_select, (value,), cursor)
        if record_id is None:
            record_id = query_return_id(query_insert, (value,), cursor)
        return record_id

    for level in page['levels']:
        cursor.execute('BEGIN TRANSACTION;')

        # Title
        title_id = get_or_insert(
            "SELECT TitleID FROM Title WHERE value = ?",
            "INSERT OR IGNORE INTO Title (value) VALUES (?)",
            level['title']
        )

        # Difficulty
        difficulty_id = get_or_insert(
            "SELECT DifficultyID FROM Difficulty WHERE trcustoms = ?",
            None,
            level['difficulty']
        ) if level.get('difficulty') else None

        # Duration
        duration_id = get_or_insert(
            "SELECT DurationID FROM Duration WHERE trcustoms = ?",
            None,
            level['duration']
        ) if level.get('duration') else None

        # Type (mandatory)
        if level.get('type'):
            type_id = get_or_insert(
                "SELECT TypeID FROM Type WHERE value = ?",
                None,
                level['type']
            )
        else:
            print("Type should not be missing")
            sys.exit(1)

        in_db = query_return_fetchall("""
            SELECT *
            FROM Trcustoms
            WHERE trcustomsExternId = ?
                AND title = ?
                AND (difficulty IS ? OR difficulty = ?)
                AND (duration IS ? OR duration = ?)
                AND engine = ?
                AND cover = ?
                AND coverMd5sum = ?
                AND release = ?;
        """, (
            level['trcustoms_id'], title_id,
            difficulty_id, difficulty_id,
            duration_id, duration_id,
            type_id, level['cover'],
            level['cover_md5sum'],
            level['release']
        ), cursor)

        if not in_db:
            # Insert into Trcustoms table
            query_insert = """
            INSERT OR IGNORE INTO Trcustoms (
                trcustomsExternId, title, difficulty, duration, engine, cover, coverMd5sum, release
            ) VALUES (?, ?, ?, ?, ?, ?, ?, ?)
            """
            trcustoms_id = query_return_id(query_insert, (
                level['trcustoms_id'],
                title_id, difficulty_id, duration_id,
                type_id, level['cover'],
                level['cover_md5sum'],
                level['release']
                ), cursor
            )
        else:
            cursor.execute('ROLLBACK;')
            continue

        # Insert Authors, Genres, Tags
        for author in level['authors']:
            author_id = query_return_id(
                "SELECT AuthorID FROM Author WHERE value = ?",
                (author, ),
                cursor
            )
            if not author_id:
                author_id = query_return_id(
                    "INSERT OR IGNORE INTO Author (value) VALUES (?)",
                    (author, ),
                    cursor
                )
            query_run(
                "INSERT OR IGNORE INTO TrcustomsAuthorsList (authorID, trcustomsID) VALUES (?, ?)",
                (author_id, trcustoms_id),
                cursor
            )
        for genre in level['genres']:
            genre_id = query_return_id(
                "SELECT GenreID FROM Genre WHERE value = ?",
                (genre, ),
                cursor
            )
            if not genre_id:
                genre_id = query_return_id(
                    "INSERT OR IGNORE INTO Genre (value) VALUES (?)",
                    (genre, ),
                    cursor
                )
            query_run(
                "INSERT OR IGNORE INTO TrcustomsGenresList (genreID, trcustomsID) VALUES (?, ?)",
                (genre_id, trcustoms_id),
                cursor
            )
        for tag in level['tags']:
            tag_id = query_return_id(
                "SELECT TagID FROM Tag WHERE value = ?",
                (tag, ),
                cursor
            )
            if not tag_id:
                tag_id = query_return_id(
                    "INSERT OR IGNORE INTO Tag (value) VALUES (?)",
                    (tag, ),
                    cursor
                )
            query_run(
                "INSERT OR IGNORE INTO TrcustomsTagsList (tagID, trcustomsID) VALUES (?, ?)",
                (tag_id, trcustoms_id),
                cursor
            )
        cursor.execute('COMMIT;')

    connection.commit()
    connection.close()


def get_trle_level_local_by_id(trle_id):
    """Return trle level by id."""
    connection = sqlite3.connect('index.db')
    cursor = connection.cursor()
    if not isinstance(trle_id, int) and trle_id > 0:
        print("function not correctly called with its argument trle_id")
        sys.exit(1)

    result = query_return_fetchall("""
        SELECT
            tr.trleExternId,
            Author.value,
            Title.value,
            Difficulty.trle,
            Duration.trle,
            Class.value,
            Type.value,
            tr.release
        FROM Trle AS tr
        INNER JOIN Author ON (Author.AuthorID = tr.author)
        INNER JOIN Title ON (Title.TitleID = tr.title)
        LEFT JOIN Difficulty ON (Difficulty.DifficultyID = tr.difficulty)
        LEFT JOIN Duration ON (Duration.DurationID = tr.duration)
        LEFT JOIN Class ON (Class.ClassID = tr.class)
        INNER JOIN Type ON (Type.TypeID = tr.type)
        WHERE tr.trleExternId = ?
        ORDER BY tr.release DESC, tr.trleExternId DESC;
        """, (trle_id, ), cursor
    )

    records = []
    for record in result:
        level = data_factory.make_trle_level_data()
        level['trle_id'] = record[0]
        level['author'] = record[1]
        level['title'] = record[2]
        level['difficulty'] = record[3]
        level['duration'] = record[4]
        level['class'] = record[5]
        level['type'] = record[6]
        level['release'] = record[7]
        records.append(level)
    connection.close()
    return records


def get_trcustoms_level_local_by_id(trcustoms_id):
    """Return trcustoms level by id."""
    connection = sqlite3.connect('index.db')
    cursor = connection.cursor()
    if not isinstance(trcustoms_id, int) and trcustoms_id > 0:
        print("function not correctly called with its argument trcustoms_id")
        sys.exit(1)

    result = query_return_fetchall("""
        SELECT
            tc.TrcustomsID,
            GROUP_CONCAT(DISTINCT Author.value) AS authors,  -- Concatenate
            Title.value,
            GROUP_CONCAT(DISTINCT Tag.value) AS tags,
            GROUP_CONCAT(DISTINCT Genre.value) AS genres,
            Type.value,
            tc.release,
            Difficulty.trcustoms,
            Duration.trcustoms,
            tc.cover,
            tc.coverMd5sum
        FROM Trcustoms AS tc
        INNER JOIN Title ON (Title.TitleID = tc.title)
        LEFT JOIN Difficulty ON (Difficulty.DifficultyID = tc.difficulty)
        LEFT JOIN Duration ON (Duration.DurationID = tc.duration)
        INNER JOIN Type ON (Type.TypeID = tc.engine)
        LEFT JOIN TrcustomsAuthorsList AS tal ON tal.trcustomsID = tc.TrcustomsID
        LEFT JOIN Author ON tal.authorID = Author.AuthorID
        LEFT JOIN TrcustomsGenresList AS tgl ON tgl.trcustomsID = tc.TrcustomsID
        LEFT JOIN Genre ON tgl.genreID = Genre.GenreID
        LEFT JOIN TrcustomsTagsList AS ttl ON ttl.trcustomsID = tc.TrcustomsID
        LEFT JOIN Tag ON ttl.tagID = Tag.TagID
        WHERE tc.trcustomsExternId = ?
        ORDER BY tc.release DESC, tc.TrcustomsID DESC;
        """, (trcustoms_id, ), cursor
    )

    level = data_factory.make_trcustoms_level_data()
    level['trcustoms_id'] = result[0][0]
    level['authors'] = result[0][1].split(',') if result[0][1] else []
    level['title'] = result[0][2]
    level['tags'] = result[0][3].split(',') if result[0][3] else []
    level['genres'] = result[0][4].split(',') if result[0][4] else []
    level['type'] = result[0][5]
    level['release'] = result[0][6]
    level['difficulty'] = result[0][7]
    level['duration'] = result[0][8]
    level['cover'] = result[0][9]
    level['cover_md5sum'] = result[0][10]
    connection.close()
    return level


def get_trle_page_local(offset, sort_latest_first=False):
    """Get a trle page."""
    connection = sqlite3.connect('index.db')
    cursor = connection.cursor()

    limit = 20

    rec = query_return_fetchall(
        "SELECT COUNT(*) AS total_count FROM Trle;",
        None,
        cursor
    )[0][0]

    if offset > rec:
        sys.exit(1)

    page = data_factory.make_trle_page_data()
    page['offset'] = offset
    page['records_total'] = rec

    order_direction = 1 if sort_latest_first else 0

    result = query_return_fetchall("""
        SELECT
            tr.trleExternId,
            Author.value,
            Title.value,
            Difficulty.trle,
            Duration.trle,
            Class.value,
            Type.value,
            tr.release
        FROM Trle AS tr
        INNER JOIN Author ON (Author.AuthorID = tr.author)
        INNER JOIN Title ON (Title.TitleID = tr.title)
        LEFT JOIN Difficulty ON (Difficulty.DifficultyID = tr.difficulty)
        LEFT JOIN Duration ON (Duration.DurationID = tr.duration)
        LEFT JOIN Class ON (Class.ClassID = tr.class)
        INNER JOIN Type ON (Type.TypeID = tr.type)
        GROUP BY tr.TrleID  -- Group by the TrleID to get unique records
        ORDER BY
            CASE WHEN ? = 0 THEN tr.release END ASC,
            CASE WHEN ? = 0 THEN tr.trleExternId END ASC,
            CASE WHEN ? = 1 THEN tr.release END DESC,
            CASE WHEN ? = 1 THEN tr.trleExternId END DESC
        LIMIT ? OFFSET ?;
        """, (order_direction, order_direction, order_direction, order_direction, limit, offset),
                cursor)

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

    connection.commit()
    connection.close()
    return page


def get_trcustoms_page_local(page_number, sort_latest_first=False):
    """Get a trcustoms page."""
    connection = sqlite3.connect('index.db')
    cursor = connection.cursor()

    limit = 20
    offset = (page_number - 1) * limit

    rec = query_return_fetchall(
        "SELECT COUNT(*) AS total_count FROM Trcustoms;",
        None,
        cursor
    )[0][0]

    page = data_factory.make_trcustoms_page_data()
    total = (rec + 19) // 20
    if page_number > total:
        sys.exit(1)
    page['current_page'] = page_number
    page['total_pages'] = total
    page['records_total'] = rec

    order_direction = 1 if sort_latest_first else 0

    result = query_return_fetchall("""
        SELECT
            tc.trcustomsExternId,
            GROUP_CONCAT(DISTINCT Author.value) AS authors,
            Title.value,
            GROUP_CONCAT(DISTINCT Tag.value) AS tags,
            GROUP_CONCAT(DISTINCT Genre.value) AS genres,
            Type.value,
            tc.release,
            Difficulty.trcustoms,
            Duration.trcustoms,
            tc.cover,
            tc.coverMd5sum
        FROM Trcustoms AS tc
        INNER JOIN Title ON (Title.TitleID = tc.title)
        LEFT JOIN Difficulty ON (Difficulty.DifficultyID = tc.difficulty)
        LEFT JOIN Duration ON (Duration.DurationID = tc.duration)
        INNER JOIN Type ON (Type.TypeID = tc.engine)
        LEFT JOIN TrcustomsAuthorsList AS tal ON tal.trcustomsID = tc.TrcustomsID
        LEFT JOIN Author ON tal.authorID = Author.AuthorID
        LEFT JOIN TrcustomsGenresList AS tgl ON tgl.trcustomsID = tc.TrcustomsID
        LEFT JOIN Genre ON tgl.genreID = Genre.GenreID
        LEFT JOIN TrcustomsTagsList AS ttl ON ttl.trcustomsID = tc.TrcustomsID
        LEFT JOIN Tag ON ttl.tagID = Tag.TagID
        GROUP BY tc.TrcustomsID
        ORDER BY
            CASE WHEN ? = 0 THEN tc.release END ASC,
            CASE WHEN ? = 0 THEN tc.trcustomsExternId END ASC,
            CASE WHEN ? = 1 THEN tc.release END DESC,
            CASE WHEN ? = 1 THEN tc.trcustomsExternId END DESC
        LIMIT ? OFFSET ?;
    """, (order_direction, order_direction, order_direction, order_direction, limit, offset),
            cursor)

    for row in result:
        level = data_factory.make_trcustoms_level_data()
        level['trcustoms_id'] = row[0]
        level['authors'] = row[1].split(',') if row[1] else []
        level['title'] = row[2]
        level['tags'] = row[3].split(',') if row[3] else []
        level['genres'] = row[4].split(',') if row[4] else []
        level['type'] = row[5]
        level['release'] = row[6]
        level['difficulty'] = row[7]
        level['duration'] = row[8]
        level['cover'] = row[9]
        level['cover_md5sum'] = row[10]
        page['levels'].append(level)

    connection.commit()
    connection.close()
    return page
