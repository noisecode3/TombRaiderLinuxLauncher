"""
Make index database for trle.net TRCustoms.org search data
The idea is grab basic and small level infromation from internet
to be able to use on a local computer for searching it should not
be a big database.


When we index in the app we do so
based on release date. So I provide a start index database
and the app will download the last "missing page" one time
a day limit. So it will just be like visiting trle.net
and trcustoms 1 page traffic per day and the user can
have a local synced list of trles.
It should be a 200MB database with all levels
"""
import re
import shutil
import time
import sys
import os
import sqlite3
import logging
import hashlib
import socket
import uuid
import random
import tempfile
from urllib.parse import urlparse, urlencode, parse_qs
from datetime import datetime
from io import BytesIO
import gc
import requests
from bs4 import BeautifulSoup
#from tqdm import tqdm
from cryptography import x509
from cryptography.hazmat.backends import default_backend
from PIL import Image
import ueberzug as ueberzug_root
import ueberzug.lib.v0 as ueberzug

gc.collect()
os.chdir(os.path.dirname(os.path.abspath(__file__)))

CERT = '/etc/ssl/certs/ca-certificates.crt'

# Set up logging
logging.basicConfig(level=logging.DEBUG, format='%(asctime)s %(levelname)s:%(message)s')
logging.getLogger("requests").setLevel(logging.DEBUG)

def acquire_lock():
    """Create a TCP socket to ensure single instance."""
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    try:
        # Bind to localhost and a specific port
        sock.bind(('127.0.0.1', 55234))
        return sock
    except socket.error:
        logging.error("Another instance is already running")
        sys.exit(1)


def release_lock(sock):
    """Release the socket and close the connection."""
    sock.close()


def make_index_database():
    """
    This database keep records from trle.net and trcustoms.org
    The purpose of this is to keep track of all so called
    "Tomb Raider Level Editor" custom levels. So that it can be
    used for looking up levels offline without directly needing
    to use those web sites.
    """

    connection = sqlite3.connect('index.db')
    cursor = connection.cursor()

    # The Version table is used to keep track of this database version
    # with only one record

    cursor.execute('''
    CREATE TABLE Version (
        id INTEGER PRIMARY KEY CHECK (id = 1), -- The primary key must always be 1
        value TEXT -- Your desired columns
    )''')

    cursor.execute('''INSERT INTO Version (id, value) VALUES (1, '0.0.1')''')

    cursor.execute('''
    CREATE TRIGGER limit_singleton
    BEFORE INSERT ON Version
    WHEN (SELECT COUNT(*) FROM Version) >= 1
    BEGIN
        SELECT RAISE(FAIL, 'Only one record is allowed.');
    END;''')

    # Single attribute table (look-up table)

    cursor.execute('''
    CREATE TABLE Title (
        TitleID INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
        value TEXT NOT NULL UNIQUE
    )''')

    # Used to keep track of levels on trle and trcustoms
    # that is the same level but with minor level name mismatch

    cursor.execute('''
    CREATE TABLE TitleIsSame (
        titleTrle INTEGER NOT NULL,
        titleTrcustoms INTEGER NOT NULL,
        PRIMARY KEY (titleTrle, titleTrcustoms),
        FOREIGN KEY (titleTrle) REFERENCES Title(TitleID),
        FOREIGN KEY (titleTrcustoms) REFERENCES Title(TitleID)
    )''')


    # Single attribute table with middle tables (look-up table)

    cursor.execute('''
    CREATE TABLE Class (
        ClassID INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
        value TEXT NOT NULL UNIQUE
    )''')

    cursor.execute('''
    CREATE TABLE ClassList (
        classID INTEGER NOT NULL,
        levelCardID INTEGER NOT NULL,
        PRIMARY KEY (classID, levelCardID),
        FOREIGN KEY (classID) REFERENCES Class(InfoClassID),
        FOREIGN KEY (levelCardID) REFERENCES LevelCard(LevelCardID)
    )''')

    # Single attribute table (look-up table)
    # There could only be one type per level

    cursor.execute('''
    CREATE TABLE Type (
        TypeID INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
        value TEXT NOT NULL UNIQUE
    )''')

    cursor.execute('''
    CREATE TABLE LevelCard (
        LevelCardID INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
        title INTEGER NOT NULL,
        type INTEGER NOT NULL,
        picture BLOB NOT NULL, -- about 3706 WebP images of 320x240 about 155-110 MB
        release DATE NOT NULL,
        trleId INT,
        trcustomsId INT,
        FOREIGN KEY (title) REFERENCES Title(TitleID),
        FOREIGN KEY (type) REFERENCES Type(TypeID)
    )''')

    cursor.execute('''
    CREATE TABLE TrcustomsKey (
        TrcustomsKeyID INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
        serial TEXT NOT NULL UNIQUE,
        cert TEXT NOT NULL UNIQUE
    )''')

    cursor.execute('''
    CREATE TABLE TrleKey (
        TrleKeyID INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
        serial TEXT NOT NULL UNIQUE,
        cert TEXT NOT NULL UNIQUE
    )''')

    # Dynamic attribute tables with middle tables (look-up, adding when first encountered)

    cursor.execute('''
    CREATE TABLE Author (
        AuthorID INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
        value TEXT NOT NULL UNIQUE
    )''')

    cursor.execute('''
    CREATE TABLE AuthorList (
        authorID INTEGER NOT NULL,
        levelCardID INTEGER NOT NULL,
        PRIMARY KEY (authorID, levelCardID),
        FOREIGN KEY (authorID) REFERENCES Author(AuthorID),
        FOREIGN KEY (levelCardID) REFERENCES LevelCard(LevelCardID)
    )''')

    # The title + authors should be a 1:1 match between trle
    # and trcustoms, both has "same" table for this.
    # So "Lara Croft" could be an author on trle but the
    # same author was "LaraCroft" on trcustoms. White-space
    # and other chars people might change like - ; : , ! &
    # could be cleared so it can try to match

    cursor.execute('''
    CREATE TABLE AuthorsIsSame (
        authorTrle INTEGER NOT NULL,
        authorTrcustoms INTEGER NOT NULL,
        PRIMARY KEY (authorTrle, authorTrcustoms),
        FOREIGN KEY (authorTrle) REFERENCES Author(AuthorID),
        FOREIGN KEY (authorTrcustoms) REFERENCES Author(AuthorID)
    )''')

    # Double attribute table with middle tables (look-up table)

    cursor.execute('''
    CREATE TABLE Difficulty (
        DifficultyID INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
        trle TEXT NOT NULL UNIQUE,
        trcustoms TEXT NOT NULL UNIQUE
    )''')

    cursor.execute('''
    CREATE TABLE DifficultyList (
        difficultyID INTEGER NOT NULL,
        levelCardID INTEGER NOT NULL,
        PRIMARY KEY (difficultyID, levelCardID),
        FOREIGN KEY (difficultyID) REFERENCES Difficulty(DifficultyID),
        FOREIGN KEY (levelCardID) REFERENCES LevelCard(LevelCardID)
    )''')

    cursor.execute('''
    CREATE TABLE Duration (
        DurationID INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
        trle TEXT NOT NULL UNIQUE,
        trcustoms TEXT NOT NULL UNIQUE
    )''')

    cursor.execute('''
    CREATE TABLE DurationList (
        durationID INTEGER NOT NULL,
        levelCardID INTEGER NOT NULL,
        PRIMARY KEY (durationID, levelCardID),
        FOREIGN KEY (durationID) REFERENCES Duration(DurationID),
        FOREIGN KEY (levelCardID) REFERENCES LevelCard(LevelCardID)
    )''')

    cursor.execute('''
    CREATE TABLE Genre (
        GenreID INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
        value TEXT NOT NULL UNIQUE
    )''')

    cursor.execute('''
    CREATE TABLE GenreList (
        genreID INTEGER NOT NULL,
        levelCardID INTEGER NOT NULL,
        PRIMARY KEY (genreID, levelCardID),
        FOREIGN KEY (genreID) REFERENCES Genre(GenreID),
        FOREIGN KEY (levelCardID) REFERENCES LevelCard(LevelCardID)
    )''')

    cursor.execute('''
    CREATE TABLE Tag (
        TagID INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
        value TEXT NOT NULL UNIQUE
    )''')

    cursor.execute('''
    CREATE TABLE TagList (
        tagID INTEGER NOT NULL,
        levelCardID INTEGER NOT NULL,
        PRIMARY KEY (tagID, levelCardID),
        FOREIGN KEY (tagID) REFERENCES Tag(TagID),
        FOREIGN KEY (levelCardID) REFERENCES LevelCard(LevelCardID)
    )''')

    # On trle.net one level can have more the one index.
    # It usually happens because there is more then one
    # author. But I think I have seen it with class also.

    cursor.execute('''
    CREATE TABLE Trle (
        TrleID INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
        trleExternId INTEGER NOT NULL,
        author INTEGER NOT NULL,
        title INTEGER NOT NULL,
        difficulty INTEGER,
        duration INTEGER,
        class INTEGER,
        type INTEGER NOT NULL,
        release DATE NOT NULL,
        FOREIGN KEY (author) REFERENCES Author(AuthorID),
        FOREIGN KEY (title) REFERENCES Title(TitleID),
        FOREIGN KEY (difficulty) REFERENCES Difficulty(DifficultyID),
        FOREIGN KEY (duration) REFERENCES Duration(DurationID),
        FOREIGN KEY (class) REFERENCES Class(ClassID),
        FOREIGN KEY (type) REFERENCES Type(TypeID),
        UNIQUE (trleExternId, author, title, difficulty, duration, class, type, release)
    )''')

    cursor.execute('''
    CREATE TRIGGER prevent_invalid_deletions_trle
    BEFORE DELETE ON Trle
    FOR EACH ROW
    WHEN
        -- Prevent deleting the last record
        (SELECT COUNT(*) FROM Trle) <= 10 OR
        -- Prevent deleting more than one record at a time
        (SELECT COUNT(*) FROM Trle WHERE ROWID IN (SELECT ROWID FROM deleted)) > 1 OR
        -- Prevent deleting a record that doesn't have the oldest release date
        OLD.release != (SELECT MIN(release) FROM Trle)
    BEGIN
        SELECT RAISE(FAIL, 'Incorrect use of table!');
    END;''')

    cursor.execute('''
    CREATE TABLE Trcustoms (
        TrcustomsID INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
        trcustomsExternId INTEGER NOT NULL,
        title INTEGER NOT NULL,
        difficulty INTEGER,
        duration INTEGER,
        engine INTEGER NOT NULL,
        cover TEXT NOT NULL,
        coverMd5sum TEXT NOT NULL,
        release DATE NOT NULL,
        FOREIGN KEY (title) REFERENCES Title(TitleID),
        FOREIGN KEY (difficulty) REFERENCES Difficulty(DifficultyID),
        FOREIGN KEY (duration) REFERENCES Duration(DurationID),
        FOREIGN KEY (engine) REFERENCES Type(TypeID),
        UNIQUE (trcustomsExternId, title, difficulty, duration, engine, release)
    )''')

    cursor.execute('''
    CREATE TRIGGER prevent_invalid_deletions_trcustoms
    BEFORE DELETE ON Trcustoms
    FOR EACH ROW
    WHEN
        -- Prevent deleting the last record
        (SELECT COUNT(*) FROM Trcustoms) <= 10 OR
        -- Prevent deleting more than one record at a time
        (SELECT COUNT(*) FROM Trcustoms WHERE ROWID IN (SELECT ROWID FROM deleted)) > 1 OR
        -- Prevent deleting a record that doesn't have the oldest release date
        OLD.release != (SELECT MIN(release) FROM Trcustoms)
    BEGIN
        SELECT RAISE(FAIL, 'Incorrect use of table!');
    END;''')

    cursor.execute('''
    CREATE TABLE TrcustomsAuthorsList (
        authorID INTEGER NOT NULL,
        trcustomsID INTEGER NOT NULL,
        PRIMARY KEY (authorID, trcustomsID),
        FOREIGN KEY (authorID) REFERENCES Author(AuthorID),
        FOREIGN KEY (trcustomsID) REFERENCES Trcustoms(TrcustomsID)
    )''')

    cursor.execute('''
    CREATE TABLE TrcustomsTagsList (
        tagID INTEGER NOT NULL,
        trcustomsID INTEGER NOT NULL,
        PRIMARY KEY (tagID, trcustomsID),
        FOREIGN KEY (tagID) REFERENCES Tag(TagID),
        FOREIGN KEY (trcustomsID) REFERENCES Trcustoms(TrcustomsID)
    )''')

    cursor.execute('''
    CREATE TABLE TrcustomsGenresList (
        genreID INTEGER NOT NULL,
        trcustomsID INTEGER NOT NULL,
        PRIMARY KEY (genreID, trcustomsID),
        FOREIGN KEY (genreID) REFERENCES Genre(GenreID),
        FOREIGN KEY (trcustomsID) REFERENCES Trcustoms(TrcustomsID)
    )''')

    connection.commit()
    connection.close()


def add_static_data():
    """class, type, difficulty, duration"""
    connection = sqlite3.connect('index.db')
    cursor = connection.cursor()

    class_values = [
        'Alien/Space', 'Atlantis', 'Base/Lab', 'Cambodia', 'Castle', 
        'Cave/Cat', 'City', 'Coastal', 'Cold/Snowy', 'Desert', 'Easter', 
        'Egypt', 'Fantasy/Surreal', 'Home', 'Ireland', 'Joke', 'Jungle', 
        'Kids', 'Library', 'Mystery/Horror', 'nc', 'Nordic/Celtic', 
        'Oriental', 'Persia', 'Pirates', 'Remake', 'Rome/Greece', 'Ship', 
        'Shooter', 'South America', 'South Pacific', 'Steampunk', 'Train', 
        'Venice', 'Wild West', 'Xmas', 'Young Lara'
    ]
    cursor.executemany("INSERT INTO Class (value) VALUES (?)",
                       [(v,) for v in class_values])

    type_values = ['TR1', 'TR2', 'TR3', 'TR4', 'TR5', 'TEN']
    cursor.executemany("INSERT INTO Type (value) VALUES (?)",
                       [(v,) for v in type_values])

    difficulty_values = [
        ('easy', 'Easy'), ('medium', 'Medium'), ('challenging', 'Hard'),
        ('very challenging', 'Very hard')
    ]
    cursor.executemany("INSERT INTO Difficulty (trle, trcustoms) VALUES (?, ?)",
                       difficulty_values)

    duration_values = [
        ('short', 'Short (<1 hour)'), ('medium', 'Medium (1+ hours)'),
        ('long', 'Long (3+ hours)'), ('very long', 'Very long (6+ hours)')
    ]
    cursor.executemany("INSERT INTO Duration (trle, trcustoms) VALUES (?, ?)",
                       duration_values)

    connection.commit()
    connection.close()


def make_trle_page_data():
    return {
        "offset": 0,
        "records_total": 0,
        "levels": [],
    }


def make_trle_level_data():
    return {
        "trle_id": 0,
        "author": "",
        "title": "",
        "difficulty": "",
        "duration": "",
        "class": "",
        "type": "",
        "release": "",
    }


def make_trcustoms_page_data():
    return {
        "current_page": 0,
        "total_pages": 0,
        "records_total": 0,
        "levels": [],
    }


def make_trcustoms_level_data():
    return {
        "trcustoms_id": 0,
        "authors": [],
        "title": "",
        "tags": [],
        "genres": [],
        "type": "",
        "release": "",
        "difficulty": "",
        "duration": "",
        "cover": "",
        "cover_md5sum": "",
    }


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
        level = make_trle_level_data()
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

    level = make_trcustoms_level_data()
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

    page = make_trle_page_data()
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
        level = make_trle_level_data()
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


def get_trle_page(offset, sortCreatedFirst=False):
    params = {
        "atype": "",
        "author": "",
        "level": "",
        "class": "",
        "type": "",
        "difficulty": "",
        "durationclass": "",
        "rating": "",
        "sortidx": 8,
        "sorttype": 2 if sortCreatedFirst else 1,
        "idx": "" if offset == 0 else str(offset)
    }
    query_string = urlencode(params)
    url = f"https://www.trle.net/pFind.php?{query_string}"
    soup = BeautifulSoup(get_response(url, 'text/html'), 'html.parser')
    page = make_trle_page_data()
    page['offset'] = offset
    span = soup.find('span', class_='navText')
    if span:
        page['records_total'] = int(span.text.strip().split()[0])
    table = soup.find('table', class_='FindTable')
    rows = table.find_all('tr')
    for row in rows[1:]:
        cells = row.find_all('td')
        level = make_trle_level_data()
        for idx, cell in enumerate(cells):
            if idx == 0:  # Author Nickname
                level['author'] = cell.get_text(strip=True)
            elif idx == 5:  # Level Name/Id
                link = cell.find('a', href=True)
                if link and '/sc/levelfeatures.php' in link['href']:
                    level['trle_id'] = link['href'].split('lid=')[-1]
                    level['title'] = cell.get_text(strip=True)
            elif idx == 6:  # Difficulty
                level['difficulty'] = cell.get_text(strip=True)
            elif idx == 7:  # Duration
                level['duration'] = cell.get_text(strip=True)
            elif idx == 8:  # Class
                level['class'] = cell.get_text(strip=True)
            elif idx == 10:  # Type
                level['type'] = cell.get_text(strip=True)
            elif idx == 13:  # Released
                level['release'] = convert_to_iso(cell.get_text(strip=True))
        page['levels'].append(level)
    return page


def print_trle_page(page):
    print(f"{page['offset'] + 20} of {page['records_total']} records")
    levels = page['levels']

    # Column widths for even spacing
    column_widths = [20, 20, 70, 20, 15, 15, 10, 20]

    headers = ["ID", "Author", "Level Name", "Difficulty",
               "Duration", "Class", "Type", "Released"]
    # Print the headers
    for i, header in enumerate(headers):
        print(header.ljust(column_widths[i]), end="")
    print("")  # Move to the next line after printing headers

    # Print the level data
    for row in levels:
        cell_data = []  # Collect the cell data to print in one line
        for idx, k in enumerate(row.keys()):
            cell = str(row[k])  # Convert each cell value to string
            width = column_widths[idx]  # Get the correct column width
            truncated_text = cell[:width].ljust(width)  # Truncate and pad the text
            cell_data.append(truncated_text)
        print("".join(cell_data))  # Print the row in one line


def print_trcustoms_page(page):
    for level in page['levels']:
        print(f"\nid: {level['trcustoms_id']} tile: {level['title']}")
        print(f"authors: {level['authors']}")
        print(f"tags: {level['tags']}")
        print(f"created: {level['release']}")
        print(f"duration: {level['duration']} difficulty: {level['difficulty']}")
        print(f"type: {level['type']}")
        print(f"genres: {level['genres']}")
        print(f"picture_url: {level['cover']}")
        print(f"picture_md5sum: {level['cover_md5sum']}")

    print(f"\nPage number:{page['current_page']} of {page['total_pages']}")
    print(f"Total records: {page['records_total']}")


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

    page = make_trcustoms_page_data()
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
        level = make_trcustoms_level_data()
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


def get_trcustoms_page(page_number, sortCreatedFirst=False):
    host = "https://trcustoms.org/api/levels/"
    if sortCreatedFirst:
        sort="-created"
    else:
        sort="created"
    params = {
        "sort": sort,
        "is_approved": 1,
        "page": "" if page_number == 0 else str(page_number)
    }
    query_string = urlencode(params)
    url = f"{host}?{query_string}"
    data = get_response(url, 'application/json')

    page = make_trcustoms_page_data()
    page['current_page'] = data['current_page']
    page['total_pages'] = data['last_page']
    page['records_total'] = data['total_count']

    results = data['results']
    for item in results:
        repacked_data = make_trcustoms_level_data()
        for author in item['authors']:
            repacked_data['authors'].append(author['username'])
        for tag in item['tags']:
            repacked_data['tags'].append(tag['name'])
        for genre in item['genres']:
            repacked_data['genres'].append(genre['name'])
        repacked_data['release'] = convert_to_iso(item['created'])
        repacked_data['cover'] = item['cover']['url']
        repacked_data['cover_md5sum'] = item['cover']['md5sum']
        repacked_data['trcustoms_id'] = item['id']
        repacked_data['title'] = item['name']
        repacked_data['type'] = item['engine']['name']
        repacked_data['difficulty'] = item['difficulty'].get('name', None) \
            if item['difficulty'] else None
        repacked_data['duration'] = item['duration'].get('name', None) \
            if item['duration'] else None
        page['levels'].append(repacked_data)
    return page

def test_trle():
    print_trle_page(get_trle_page(0, True))
    offset = 0
    while True:
        user_input = input("Press Enter for the next page (or type 'q' to quit: ")
        if user_input.lower() == 'q':
            print("Exiting...")
            break
        offset += 20
        print_trle_page(get_trle_page(offset, True))


def test_trcustoms():
    page = get_trcustoms_page(1, True)
    print_trcustoms_page(page)
    offset = 1
    while True:
        user_input = input("Press Enter for the next page (or type 'q' to quit: ")
        if user_input.lower() == 'q':
            print("Exiting...")
            break
        offset += 1
        page = get_trcustoms_page(offset, True)
        print_trcustoms_page(page)


def test_trcustoms_local():
    page = get_trcustoms_page_local(1, True)
    print_trcustoms_page(page)
    offset = 1
    while True:
        user_input = input("Press Enter for the next page (or type 'q' to quit: ")
        if user_input.lower() == 'q':
            print("Exiting...")
            break
        offset += 1
        page = get_trcustoms_page_local(offset, True)
        print_trcustoms_page(page)


def test_trle_local():
    offset = 0
    print_trle_page(get_trle_page_local(offset, True))
    while True:
        user_input = input("Press Enter for the next page (or type 'q' to quit: ")
        if user_input.lower() == 'q':
            print("Exiting...")
            break
        offset += 20
        print_trle_page(get_trle_page_local(offset, True))


def get_response(url, content_type):
    valid_content_types = [
        'text/html',
        'application/json',
        'application/pkix-cert',
        'image/jpeg',
        'image/png'
    ]

    if content_type not in valid_content_types:
        logging.error("Invalid content type: %s", content_type)
        sys.exit(1)

    max_retries = 3
    delay = 40
    retries = 0
    response = None

    while retries < max_retries:
        try:
            response = requests.get(url, verify=CERT, timeout=5)
            response.raise_for_status()  # Raises an HTTPError for bad responses (4xx/5xx)
            break  # Exit loop on success
        except requests.exceptions.Timeout:
            retries += 1
            logging.error("Request to %s timed out, retrying (%d/%d)...", url, retries, max_retries)
            if retries < max_retries:
                time.sleep(delay)  # Wait for 40 seconds before retrying
            else:
                logging.error("Max retries reached. Exiting.")
                sys.exit(1)
        except requests.exceptions.RequestException as response_error:
            logging.error("Failed to retrieve content: %s", response_error)
            sys.exit(1)

    # Get the Content-Type header once and reuse
    response_content_type = response.headers.get('Content-Type', '').split(';')[0].strip()

    if response_content_type == 'text/html':
        return response.text
    if response_content_type == 'application/json':
        return response.json()
    if response_content_type in ['image/jpeg', 'image/png']:
        return response.content
    if response_content_type == 'application/pkix-cert':
        # It seems to freak out crt.sh if there is no sleep here
        time.sleep(5)
        validate_pem(response.text)
        return response.content

    logging.error("Unexpected content type: %s, expected %s",
        response_content_type,
        content_type
    )
    sys.exit(1)


def check_ueberzug():
    try:
        version_str = ueberzug_root.__version__
        version_parts = [int(v) for v in version_str.split('.')]  # Split and convert to integers
        # Check if version is at least 18.2.3
        if version_parts < [18, 2, 3]:
            print("Your version of ueberzug is too old.")
            print("Please upgrade by running the following:")
            print("pip install git+https://github.com/ueber-devel/ueberzug.git")
            print("")
            print("ueberzug is packaged on most popular distros debian, arch, fedora, gentoo")
            print("and void, please request the package maintainer of your favourite distro to")
            print("package the latest release of ueberzug.")
            return False
        return True
    except AttributeError:
        print("Could not determine the version of ueberzug.")
        return False


def cover_resize_to_webp(input_img):
    img = Image.open(BytesIO(input_img))
    # Resize the image to 320x240
    img_width = 80
    img_height = 60
    # Convert to terminal character size
    #width_in_chars, height_in_chars = convert_pixels_to_chars(img_width, img_height)
    #img = img.resize((img_width, img_height))
    img = img.resize((320, 240))
    webp_image = BytesIO()

    # Convert the image to .webp format
    img.save(webp_image, format='WEBP')

    # Get the image data as bytes
    return webp_image.getvalue()


def get_trle_cover(trle_id):
    if not trle_id.isdigit():
        print("Invalid ID number.")
        sys.exit(1)
    url = f"https://www.trle.net/screens/{trle_id}.jpg"

    response = get_response(url, 'image/jpeg')
    return cover_resize_to_webp(response)


def is_valid_uuid(value):
    try:
        uuid_obj = uuid.UUID(value, version=4)
        return str(uuid_obj) == value
    except ValueError:
        return False

def calculate_md5(data):
    """Calculate the MD5 checksum of the given data."""
    md5_hash = hashlib.md5(usedforsecurity=False)
    md5_hash.update(data)
    return md5_hash.hexdigest()

def get_trcustoms_cover(image_uuid, md5sum, image_format):
    """ A test for getting pictures from internet and displaying on the terminal"""
    if not is_valid_uuid(image_uuid):
        print("Invalid image UUID.")
        sys.exit(1)
    if image_format not in ["jpg", "png"]:
        print("Invalid image format.")
        sys.exit(1)

    url = f"https://data.trcustoms.org/media/level_images/{image_uuid}.{image_format}"
    if image_format == "jpg":
        image_format = "jpeg"
    response = get_response(url, f"image/{image_format}")

    # Check if the MD5 sum matches
    downloaded_md5sum = calculate_md5(response)
    if downloaded_md5sum != md5sum:
        print(f"MD5 mismatch: Expected {md5sum}, got {downloaded_md5sum}")
        sys.exit(1)

    # Save the image to a temporary file
    with tempfile.NamedTemporaryFile(delete=False, suffix=".webp") as temp_image_file:
        temp_image_file.write(cover_resize_to_webp(response))
        temp_image_path = temp_image_file.name

    # Example menu items, later database
    menu_items = []
    #for _ in range(item_rows * item_columns):
    #for _ in range(14): # some search result example
    for _ in range(20): # page example
        menu_items.append(
        {
            'title': "Time Goes By",
            'duration': "Short (<1 hour)",
            'difficulty': "Easy",
            'type': "TR1",
            'authors': ['Drobridski'],
            'genres': ['Fantasy', 'Humor'],
            'tags': [],
            'created': "2024-05-13"
        })
    # Display the menu
    if check_ueberzug():
        display_menu(menu_items, temp_image_path)


# Function to display the menu for multiple items, with pictures in the terminal
# This works with Xterm/UXterm and Alacritty
# Tested with xfce4-terminal, Terminator and other with fancy tabs and menus wont work
# Developed with on x11 and python3 ueberzug with pip from
# https://github.com/ueber-devel/ueberzug/
# xterm is supported 100% with TrueType
# uxterm -fa 'TrueType' -fs 12
# will calculate "space" correctly for all font sizes -fs N
# Alacritty is 100% working with fonts that have the same spacing as

# [font]
# size = 15.0

# [font.bold]
# family = "Hack"
# style = "Bold"

# [font.bold_italic]
# family = "Hack"
# style = "Bold Italic"

# [font.italic]
# family = "Hack"
# style = "Italic"

# [font.normal]
# family = "Hack"
# style = "Regular"


def display_menu(items, image_path):
    """Display a list of items with images next to them."""
    supported_terminals = ['alacritty', 'xterm']
    term = os.getenv('TERM', '').lower()
    print(term)
    if term not in supported_terminals:
        print("Terminal not supported.")
        sys.exit(1)
    if not shutil.which('ueberzug'):
        print("ueberzug not found in $PATH.")
        sys.exit(1)
    print("\033c", end="")
    print("")

    terminal_size = shutil.get_terminal_size()
    max_rows = (terminal_size.lines - 2) // 7
    max_columns = terminal_size.columns // 79

    if max_columns < 1:
        print("Screen width too small")
        sys.exit(1)

    with ueberzug.Canvas() as canvas:
        full_board = max_rows * max_columns
        display_items = items[:full_board]
        remaining_items = items[full_board:]
        for i in range(len(display_items)):
            if i == len(display_menu.identifiers):
                cover = canvas.create_placement(
                    f'cover_{i}',
                    x=(i % max_columns) * 79 + 2,
                    y=(i // max_columns) * 7 + 1,
                    scaler=ueberzug.ScalerOption.COVER.value,
                    width=16, height=6
                )
                cover.path = image_path
                cover.visibility = ueberzug.Visibility.VISIBLE
                display_menu.identifiers.append(cover)
            elif i < len(display_menu.identifiers):
                cover = display_menu.identifiers[i]
                with canvas.lazy_drawing:
                    cover.x=(i % max_columns) * 79 + 2
                    cover.y=(i // max_columns) * 7 + 1
                    cover.path = image_path
                    cover.visibility = ueberzug.Visibility.VISIBLE
            else:
                print(f"Index {i} is out of bounds.")

        # Calculate full rows and handle the last row separately
        full_rows = len(display_items) // max_columns
        last_row_items = len(display_items) % max_columns

        # Display full rows
        for row in range(full_rows):
            row_offset = row * max_columns
            print_row(display_items, row_offset, max_columns)

        # Display last row if there are any remaining items
        if last_row_items > 0:
            row_offset = full_rows * max_columns
            print_row(display_items, row_offset, last_row_items)

        awn = input(f"  {len(display_items)} of {len(items)} results, " +\
                "Press 'q' and Enter to exit, else press Enter for next page...")

        print("\033c", end="")
        if awn == 'q':
            sys.exit(0)

        # Remove all previous images
        for identifier in display_menu.identifiers:
            # Create the remove command
            identifier.visibility = ueberzug.Visibility.INVISIBLE

        if len(remaining_items) > 0:
            display_menu(remaining_items, image_path)  # Recursive call with remaining items

display_menu.identifiers = [] # never delete me baby

def print_row(items, row_offset, columns):
    """Helper function to print a single row."""
    # Title row
    for column in range(columns):
        print(f"{' '*19}{items[row_offset + column]['title'][:60]:<60}", end="")
    print("")

    # Duration and difficulty row
    for column in range(columns):
        field = (
            f"{' '*19}Duration: {items[row_offset + column]['duration']}"
            f" Difficulty: {items[row_offset + column]['difficulty']}"
        )
        print(field[:79].ljust(79), end="")
    print("")

    # Release date and type row
    for column in range(columns):
        field = (
            f"{' '*19}Release Date: {items[row_offset + column]['created']}"
            f" Type: {items[row_offset + column]['type']}"
        )
        print(field[:79].ljust(79), end="")
    print("")

    for column in range(columns):
        print( \
            f"{' '*19}Author: " +\
            f"{', '.join(map(str, items[row_offset + column]['authors']))[:52]:<52}",
            end=""
        )
        print("")

    for column in range(columns):
        print( \
            f"{' '*19}Genre: " +\
            f"{', '.join(map(str, items[row_offset + column]['genres']))[:53]:<53}",
            end=""
        )
    print("")

    for column in range(columns):
        print(
            f"{' '*19}Tag: " +\
            f"{', '.join(map(str, items[row_offset + column]['tags']))[:55]:<55}",
            end=""
        )
    print("\n")


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



def validate_pem(pem):
    # Check if the response contains a PEM key
    pem_pattern = r'-----BEGIN CERTIFICATE-----(.*?)-----END CERTIFICATE-----'
    pem_match = re.search(pem_pattern, pem, re.DOTALL)
        #pem_key = pem_match.group(0)
        #print("PEM Key found:\n", pem_key)
    if not pem_match:
        print("PEM Key not found.")
        sys.exit(1)


def print_key_list(html):
    """scrape keys and key status here
       we cant depend on local keys from package manger that might be incomplete"""
    soup = BeautifulSoup(html, 'html.parser')
    # Find the table containing the keys
    table = soup.find_all('table')[2]  # Adjust index if necessary

    # Iterate over the rows (skipping the header row)
    ids = []
    for row in table.find_all('tr')[1:]:
        key_column = row.find_all('td')[0]  # Get the first column
        key_striped = key_column.text.strip()  # Extract the key text
        print(f"Key: {key_striped}")
        ids.append(key_striped)

    return ids


def validate_downloaded_key(id_number, expected_serial):
    pem_key = get_response(f"https://crt.sh/?d={id_number}", 'application/pkix-cert')

    # Load the certificate
    certificate = x509.load_pem_x509_certificate(pem_key, default_backend())

    # Extract the serial number and convert it to hex (without leading '0x')
    hex_serial = f'{certificate.serial_number:x}'

    # Compare the serial numbers
    if hex_serial == expected_serial:
        print("The downloaded PEM key matches the expected serial number.")
    else:
        logging.error("Serial mismatch! Expected: %s, but got: %s", expected_serial, hex_serial)
        sys.exit(1)


def get_key(id_number):
    # Input validation
    if not id_number.isdigit():
        print("Invalid ID number.")
        sys.exit(1)
    html = get_response(f"https://crt.sh/?id={id_number}", 'text/html')

    # Create a BeautifulSoup object
    soup = BeautifulSoup(html, 'html.parser')
    body_tag = soup.find("body")
    td_text_tag = body_tag.find("td", "text")
    a_tag = td_text_tag.find('a', string=lambda text: text and 'Serial' in text)
    href = a_tag['href']

    # Parse the query string to get the 'serial' parameter
    query_params = parse_qs(urlparse(href).query)
    serial_number = query_params.get('serial', [None])[0]

    # Normalize serial by stripping leading zeros
    serial_number = serial_number.lstrip('0')

    if not serial_number:
        print("Serial Number tag not found.")
        sys.exit(1)

    print("Serial Number:", serial_number)

    validate_downloaded_key(id_number, serial_number)


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


def convert_to_iso(date_str):
    """Convert date string from various formats to ISO-8601 (YYYY-MM-DD) format."""

    # Try to parse '01-Jan-2024' format
    try:
        return datetime.strptime(date_str, '%d-%b-%Y').strftime('%Y-%m-%d')
    except ValueError:
        pass

    # Try to parse '2024-09-24T15:12:19.212984Z' ISO format with time and microseconds
    try:
        return datetime.strptime(date_str, '%Y-%m-%dT%H:%M:%S.%fZ').strftime('%Y-%m-%d')
    except ValueError:
        pass

    # Try to parse '1999-08-29T00:00:00Z' ISO format without microseconds
    try:
        return datetime.strptime(date_str, '%Y-%m-%dT%H:%M:%SZ').strftime('%Y-%m-%d')
    except ValueError:
        pass

    raise ValueError(f"Unsupported date format: {date_str}")


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
            difficulty_id, difficulty_id,  # Handling NULL with IS or =
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
            difficulty_id, difficulty_id,  # Handling NULL with IS or =
            duration_id, duration_id,   # Handling NULL with IS or = this is fucking crazy
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


def test_insert_trle_book():
    """This must not bother the server but at the same time its not
       meant for normal users but the database admin to run this. Not
       bother here means to not be "detected" as a robot and limit request rate.
       Not detected in behavior imply that there is some random delay about
       0-5 + 10 seconds just like someone would browser thru the levels and click
       the ">" arrow for 30-45 min. We get all the records for indexing them here."""

    # Get the first page to determine the total number of records
    page = get_trle_page(0)
    total_records = page['records_total']

    # Insert the first page of data
    insert_trle_page(page)
    print(f"Records number:20 of {total_records}")
    delay = random.uniform(10, 15)
    time.sleep(delay)

    # Start offset at 20 and loop through all records in steps of 20
    offset = 20
    while offset < total_records:
        # Fetch the next page of data
        page = get_trle_page(offset)
        insert_trle_page(page)

        # Increment offset by 20 for the next batch
        if offset + 20 > total_records:
            offset = total_records
        else:
            offset += 20
        print(f"Records number:{offset} of {total_records}")

        # Introduce a random delay between 10 and 15 seconds
        delay = random.uniform(10, 15)
        time.sleep(delay)


def test_insert_trcustoms_book():
    # Get the first page to determine the total number of records
    page = get_trcustoms_page(1)
    total_pages = page['total_pages']

    # Insert the first page of data
    insert_trcustoms_page(page)
    print(f"Page number:1 of {total_pages}")
    delay = random.uniform(10, 15)
    time.sleep(delay)

    # Start at page 2 and loop through all records in steps of 20
    page_number = 2
    while page_number <= total_pages:
        # Fetch the next page of data
        page = get_trcustoms_page(page_number)
        insert_trcustoms_page(page)
        print(f"Page number:{page_number} of {total_pages}")

        # Increment page number by 1 for the next batch
        page_number += 1

        # Introduce a random delay between 10 and 15 seconds
        delay = random.uniform(10, 15)
        time.sleep(delay)




def update_keys_trle(certList):
    """Check if identical certificate"""
    connection = sqlite3.connect('index.db')
    cursor = connection.cursor()
    """ 
    CREATE TABLE TrcustomsKey (
        TrcustomsKeyID INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
        serial TEXT NOT NULL UNIQUE,
        cert TEXT NOT NULL UNIQUE
    )
    CREATE TABLE TrleKey (
        TrleKeyID INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
        serial TEXT NOT NULL UNIQUE,
        cert TEXT NOT NULL UNIQUE
    """
    
    db_keys = query_return_fetchall("SELECT * FROM TrleKey", None, cursor)




    connection.commit()
    connection.close()


if __name__ == '__main__':
    lock_sock = acquire_lock()
    try:
        if len(sys.argv) != 2:
            logging.info("Usage: python3 make_index_database.py COMMAND")
            logging.info("COMMAND = new, trcustoms, trle, trcustoms_local, trle_local, insert_trcustoms_book, insert_trle_book, trcustoms_key, trle_key")
            sys.exit(1)
        else:
            COMMAND = sys.argv[1]
            if COMMAND == "new":
                make_index_database()
                add_static_data()
            if COMMAND == "check_trle_doubles":
                check_trle_doubles()
            if COMMAND == "insert_trle_book":
                test_insert_trle_book()
            if COMMAND == "insert_trcustoms_book":
                test_insert_trcustoms_book()
            if COMMAND == "trle":
                test_trle()
            if COMMAND == "trle_id":
                print(len(get_trle_level_local_by_id(3528)))
            if COMMAND == "trcustoms_id":
                print(get_trcustoms_level_local_by_id(23))

            if COMMAND == "trcustoms_pic":
                get_trcustoms_cover("6e3df3e0-9e29-4195-b531-3c0b4e9e1719", "1f2280d16fb96328953e7d099acbe19d", "png")


            if COMMAND == "test_font_size":
                print(estimate_font_size())

            if COMMAND == "trle_local":
                test_trle_local()
            if COMMAND == "trcustoms":
                test_trcustoms()
            if COMMAND == "trcustoms_local":
                test_trcustoms_local()
            if COMMAND == "trcustoms_key":
                resp = get_response("https://crt.sh/?q=trcustoms.org&exclude=expired", 'text/html')
                key_list = print_key_list(resp)
                for key in key_list:
                    get_key(key)
            if COMMAND == "trle_key":
                resp = get_response("https://crt.sh/?q=www.trle.net&exclude=expired", 'text/html')
                key_list = print_key_list(resp)
                for key in key_list:
                    get_key(key)
    finally:
        release_lock(lock_sock)
