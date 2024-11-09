import os
import sys
import sqlite3

import data_factory
os.chdir(os.path.dirname(os.path.abspath(__file__)))


def check_trle_doubles():
    connection = sqlite3.connect('index.db')
    cursor = connection.cursor()
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
        COUNT(*) as record_count
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
    HAVING COUNT(*) > 1
    ORDER BY record_count DESC;""", None, cursor)
    print(result)
    connection.close()


def query_return_fetchall(query, params, cursor):
    try:
        if not query.strip().upper().startswith("SELECT"):
            print("You need to use SELECT with this function")
            sys.exit(1)
        if not params:
            cursor.execute(query)
        else:
            cursor.execute(query, params)
        return cursor.fetchall()
    except sqlite3.DatabaseError as db_error:
        print(f"Database error occurred: {db_error}")
        sys.exit(1)


def query_return_id(query, params, cursor):
    try:
        cursor.execute(query, params)
        if query.strip().upper().startswith("INSERT"):
            return cursor.lastrowid
        result = cursor.fetchone()
        if result and isinstance(result[0], int) and result[0] >= 0:
            return result[0]
        return None
    except sqlite3.IntegrityError as integrity_error:
        print(f"Integrity error occurred with parameters: {params}\n{integrity_error}")
        sys.exit(1)
    except sqlite3.DatabaseError as db_error:
        print(f"Database error occurred: {db_error}")
        sys.exit(1)


def query_run(query, params, cursor):
    try:
        cursor.execute(query, params)
    except sqlite3.IntegrityError as integrity_error:
        print(f"Integrity error occurred with parameters: {params}\n{integrity_error}")
        sys.exit(1)
    except sqlite3.DatabaseError as db_error:
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


def get_trle_page_local(offset, sortCreatedFirst=False):
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
        ORDER BY tr.release DESC, tr.trleExternId DESC
        LIMIT ? OFFSET ?;
        """, (limit, offset), cursor
    )
    # Process result to format the output as needed
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


def get_trcustoms_page_local(page_number, sortCreatedFirst=False):
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
        GROUP BY tc.TrcustomsID  -- Group by the TrcustomsID to get unique records
        ORDER BY tc.release DESC, tc.TrcustomsID DESC
        LIMIT ? OFFSET ?;
        """, (limit, offset), cursor
    )
    # Process result to format the output as needed
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
