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
It should be a 400MB database with all levels
"""

#import re
import sys
import os
#import json
#import hashlib
import sqlite3
import logging
import socket
from urllib.parse import urlencode
import requests
from bs4 import BeautifulSoup
#from tqdm import tqdm

os.chdir(os.path.dirname(os.path.abspath(__file__)))

# Set up logging
logging.basicConfig(level=logging.DEBUG, format='%(asctime)s %(levelname)s:%(message)s')
logging.getLogger("requests").setLevel(logging.DEBUG)

CERT = '/etc/ssl/certs/ca-certificates.crt'

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
    the title + authors should be a 1:1 match between trle
    and trcustoms, so "Lara Croft" could be on trle but the
    same author was "LaraCroft" on trcustoms. White-space
    and other chars people might change like - ; : , ! &
    could be cleared so it can try to match by taking out one
    of them at a time

    On trle.net one level can have more the one index.
    It usually happens because there is more then one
    author. But I think I have seen it with class also.

    LevelCard table
        title   STRING
        release DATE
        type    INT
        picture BLOB about (as of now) 3640 WebP images of 640x480
                about 355.47 MB

    Tables with one or two attributes
    Key tables
        TrleKey
        TrcustomsKey

    Id table
        TrleID
        TrcustomsID
        TombllID

    Dynamic attribute tables with middle tables (look-up, adding when first encountered)
        Author
        Genre
        Tag

    Static attribute tables with middle tables (look-up table)
        Class
        Type

    Double static attribute with middle tables (look-up table)
        Difficulty
        Duration

    """
    connection = sqlite3.connect('index.db')
    cursor = connection.cursor()

    cursor.execute('''
    CREATE TABLE Type (
        InfoTypeID INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
        value TEXT NOT NULL UNIQUE
    )''')

    cursor.execute('''
    CREATE TABLE Id (
        ID INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
        tombllID INTEGER,
        trleID INTEGER,
        trcustomsID INTEGER
    )''')

    cursor.execute('''
    CREATE TABLE LevelCard (
        LevelCardID INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
        title TEXT NOT NULL,
        release DATE NOT NULL,
        type INT NOT NULL,
        picture BLOB NOT NULL,
        ID INT NOT NULL,
        FOREIGN KEY (type) REFERENCES Type(InfoTypeID),
        FOREIGN KEY (ID) REFERENCES Id(ID)
    )''')

    cursor.execute('''
    CREATE TABLE TrcustomsKey (
        InfoTypeID INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
        value TEXT NOT NULL UNIQUE
    )''')

    cursor.execute('''
    CREATE TABLE TrleKey (
        InfoTypeID INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
        value TEXT NOT NULL UNIQUE
    )''')

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
    CREATE TABLE Class (
        InfoClassID INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
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


def print_trle_page(soup, offset):
    span = soup.find('span', class_='navText')
    if span:
        record_count = int(span.text.strip().split()[0])
        print(f"{offset+20} of {record_count} records")

    table = soup.find('table', class_='FindTable')
    keep_indices = [0, 1, 5, 6, 7, 8, 10, 13]
    rows = table.find_all('tr')
    # Column widths for even spacing
    column_widths = [20, 20, 70, 20, 15, 15, 10, 20]

    headers = ["Nickname", "Author's Name", "Level Name/Info", "Difficulty",
               "Duration", "Class", "Type", "Released"]
    for i, header in enumerate(headers):
        print(header.ljust(column_widths[i]), end="")
    print("")

    for row in rows[1:]:
        cells = row.find_all('td')
        cell_data = []  # Collect the cell data to print in one line

        for idx, cell in enumerate(cells):
            if idx in keep_indices:
                # Check for <a> tags with the lid link
                link = cell.find('a', href=True)
                if link and '/sc/levelfeatures.php' in link['href']:
                    # Extract lid number with title
                    lid_number = link['href'].split('lid=')[-1]
                    text = cell.get_text(strip=True)
                    cell_data.append(f'{lid_number} {text}'.ljust(column_widths[len(cell_data)]))
                else:
                    # Extract the cell text and append it with even spacing
                    text = cell.get_text(strip=True)
                    cell_data.append(text.ljust(column_widths[len(cell_data)]))

        print("".join(cell_data))


def get_trle_page(offset):
    host = "https://www.trle.net/pFind.php"
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
        "sorttype": 2,
        "idx": "" if offset == 0 else str(offset)
    }
    query_string = urlencode(params)
    url = f"{host}?{query_string}"
    response = requests.get(url, verify=CERT, timeout=5)
    if response.status_code != 200:
        logging.error('Failed to retrieve iframe content. Status code: %s', response.status_code)
        sys.exit(1)
    return response.text


def print_trcustoms_page(json):
    print(json)# ye I know I was lazy but it looks good as it is, somewhere in there is a joke...


def get_trcustoms_page(page):
    host = "https://trcustoms.org/api/levels/"
    params = {
        "page": "" if page == 0 else str(page)
    }
    query_string = urlencode(params)
    url = f"{host}?{query_string}"
    response = requests.get(url, verify=CERT, timeout=5)
    if response.status_code != 200:
        logging.error('Failed to retrieve iframe content. Status code: %s', response.status_code)
        sys.exit(1)
    return response.text


def test_trle():
    soup = BeautifulSoup(get_trle_page(0), 'html.parser')
    print_trle_page(soup, 0)
    offset = 0
    while True:
        user_input = input("Press Enter for the next page (or type 'q' to quit: ")
        if user_input.lower() == 'q':
            print("Exiting...")
            break
        offset += 20
        soup = BeautifulSoup(get_trle_page(offset), 'html.parser')
        print_trle_page(soup, offset)


def test_trcustoms():
    page = get_trcustoms_page(1)
    print_trcustoms_page(page)
    offset = 1
    while True:
        user_input = input("Press Enter for the next page (or type 'q' to quit: ")
        if user_input.lower() == 'q':
            print("Exiting...")
            break
        offset += 1
        page = get_trcustoms_page(offset)
        print_trcustoms_page(page)


if __name__ == '__main__':
    lock_sock = acquire_lock()
    try:
        if len(sys.argv) != 2:
            logging.info("Usage: python3 make_index_database.py COMMAND")
            logging.info("COMMAND = new, testTrle, testTrcustoms")
            sys.exit(1)
        else:
            COMMAND = sys.argv[1]
            if COMMAND == "new":
                make_index_database()
                add_static_data()
            if COMMAND == "trle":
                test_trle()
            if COMMAND == "trcustoms":
                test_trcustoms()
    finally:
        release_lock(lock_sock)
