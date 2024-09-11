"""
Take tombll json file and add it to the database
"""
import os
import sys
#import shutil
#import hashlib
import sqlite3
#import zipfile
import json
import logging
import requests
#from tqdm import tqdm
from PIL import Image
from io import BytesIO


# Set up logging
logging.basicConfig(level=logging.DEBUG, format='%(asctime)s %(levelname)s:%(message)s')
logging.getLogger("requests").setLevel(logging.DEBUG)
logging.getLogger("urllib3").setLevel(logging.DEBUG)

CERT = '/etc/ssl/certs/ca-certificates.crt'

def get_tombll_json(path):
    try:
        with open(path, mode='r', encoding='utf-8') as json_file:
            try:
                return json.load(json_file)
            except json.JSONDecodeError as json_error:
                print(f"Error decoding JSON from file {path}: {json_error}")
                sys.exit(1)
    except FileNotFoundError:
        print(f"File not found: {path}")
        sys.exit(1)
    except IOError as file_error:
        print(f"IO error occurred while opening file {path}: {file_error}")
        sys.exit(1)


def connect_database():
    return sqlite3.connect(os.path.dirname(os.path.abspath(__file__)) + '/tombll.db')


def commit_database():
    CONNECTION.commit()


def disconnect_database():
    CONNECTION.close()


def query_return_id(query, params):
    cursor = CONNECTION.cursor()
    try:
        cursor.execute(query, params)
        if query.strip().upper().startswith("INSERT"):
            return cursor.lastrowid
        else:
            result = cursor.fetchone()
            if result and isinstance(result[0], int) and result[0] >= 0:
                return result[0]
            return None
    except sqlite3.DatabaseError as db_error:
        print(f"Database error occurred: {db_error}")
        sys.exit(1)


def query_run(query, params):
    cursor = CONNECTION.cursor()
    try:
        cursor.execute(query, params)
    except sqlite3.DatabaseError as db_error:
        print(f"Database error occurred: {db_error}")
        sys.exit(1)
    #except sqlite3.Error as insert_error:
    #    logging.error("SQLite error: %s", insert_error)


def make_empty_null(value):
    """ Turn empty string or unspecified size into None """
    if value == "":
        return None
    if value == 0.0:
        return None
    return value


def add_authors_to_database(authors_array, level_id):
    for author in authors_array:
        query_select_id = "SELECT AuthorID FROM Author WHERE value = ?"
        query_insert = "INSERT INTO Author (value) VALUES (?)"
        query_insert_middle = "INSERT INTO AuthorList (authorID, levelID) VALUES (?, ?)"
        author_id = query_return_id(query_select_id, (author,))
        if author_id is None:
            author_id = query_return_id(query_insert, (author,))
        query_run(query_insert_middle, (author_id, level_id))


def add_genres_to_database(genres_array, level_id):
    for genre in genres_array:
        query_select_id = "SELECT GenreID FROM Genre WHERE value = ?"
        query_insert = "INSERT INTO Genre (value) VALUES (?)"
        query_insert_middle = "INSERT INTO GenreList (genreID, levelID) VALUES (?, ?)"
        genre_id = query_return_id(query_select_id, (genre,))
        if genre_id is None:
            genre_id = query_return_id(query_insert, (genre,))
        query_run(query_insert_middle, (genre_id, level_id))


def add_tags_to_database(tags_array, level_id):
    for tag in tags_array:
        query_select_id = "SELECT TagID FROM Tag WHERE value = ?"
        query_insert = "INSERT INTO Tag (value) VALUES (?)"
        query_insert_middle = "INSERT INTO TagList (tagID, levelID) VALUES (?, ?)"
        tag_id = query_return_id(query_select_id, (tag,))
        if tag_id is None:
            tag_id = query_return_id(query_insert, (tag,))
        query_run(query_insert_middle, (tag_id, level_id))


def add_zip_files_to_database(zip_files_array, level_id):
    for zip_file in zip_files_array:
        query_insert = "INSERT INTO Zip (name, size, md5sum, url, version, release)"\
                       "VALUES (?,?,?,?,?,?)"
        insert_arg =(
            make_empty_null(zip_file.get('name')),
            make_empty_null(zip_file.get('size')),
            make_empty_null(zip_file.get('md5')),
            make_empty_null(zip_file.get('url')),
            make_empty_null(zip_file.get('release')),
            make_empty_null(zip_file.get('version'))
        )
        query_insert_middle = "INSERT INTO ZipList (zipID, levelID) VALUES (?, ?)"
        middle_arg =(
            query_return_id(query_insert, insert_arg),
            level_id
        )
        query_run(query_insert_middle, middle_arg)


def add_screen_to_database(screen, level_id):
    if screen.startswith("https://www.trle.net/screens/"):
        response = requests.get(screen, verify=CERT, timeout=5)

        # Open the image and convert it to .webp format
        img = Image.open(BytesIO(response.content))
        webp_image = BytesIO()

        # Convert the image to .webp
        img.save(webp_image, format='WEBP')

        # Get the image data as bytes
        webp_image_data = webp_image.getvalue()

        # Insert the .webp image into the database
        query_insert = "INSERT INTO Picture (data) VALUES (?)"
        query_insert_middle = "INSERT INTO Screens (pictureID, levelID) VALUES (?, ?)"

        query_run(query_insert_middle, (
            query_return_id(query_insert, (webp_image_data,)),
            level_id
        ))


"""
def add_screen_to_database(screen, level_id):
    if screen.startswith("https://www.trle.net/screens/"):
        response = requests.get(screen, verify=CERT, timeout=5)
        query_insert = "INSERT INTO Picture (data) VALUES (?)"
        query_insert_middle = "INSERT INTO Screens (pictureID, levelID) VALUES (?, ?)"
        query_run(query_insert_middle,(
            query_return_id(query_insert, (response.content, )),
            level_id
        ))
"""


def add_screens_to_database(large_screens_array, level_id):
    for screen in large_screens_array:
        add_screen_to_database(screen, level_id)


def add_level_to_database():
    query = "INSERT INTO Level (body, walkthrough, infoID) VALUES (?, ?, ?)"
    level_id = query_return_id(query, (DATA.get('body'), DATA.get('walkthrough'), add_info_to_database()))
    logging.info("Current tombll level_id: %s", level_id)
    return level_id


def add_info_to_database():
    info_difficulty = DATA.get('difficulty')
    if info_difficulty != "":
        query = "SELECT InfoDifficultyID FROM InfoDifficulty WHERE value = ?"
        info_difficulty_id = query_return_id(query, (info_difficulty,))
    else:
        info_difficulty_id = None

    info_duration = DATA.get('duration')
    if info_duration != "":
        query = "SELECT InfoDurationID FROM InfoDuration WHERE value = ?"
        info_duration_id = query_return_id(query, (info_duration,))
    else:
        info_duration_id = None

    info_type = DATA.get('type')
    if info_type == "":
        info_type = None
    query = "SELECT InfoTypeID FROM InfoType WHERE value = ?"
    info_type_id = query_return_id(query, (info_type,))

    info_class = DATA.get('class')
    if info_class == "":
        info_class = None
    query = "SELECT InfoClassID FROM InfoClass WHERE value = ?"
    info_class_id = query_return_id(query, (info_class,))

    query = "INSERT INTO Info (title, release, difficulty, duration,"\
            " type, class, trleID, trcustomsID) VALUES (?,?,?,?,?,?,?,?)"
    arg = (
        DATA.get('title'),
        DATA.get('release'),
        info_difficulty_id,
        info_duration_id,
        info_type_id,
        info_class_id,
        DATA.get('trle_id'),
        DATA.get('trcustoms_id')
    )

    return query_return_id(query, arg)


def add_tombll_json_to_database():
    level_id = add_level_to_database()
    add_authors_to_database(DATA.get('authors'), level_id)
    add_genres_to_database(DATA.get('genres'), level_id)
    add_tags_to_database(DATA.get('tags'), level_id)
    add_zip_files_to_database(DATA.get('zip_files'), level_id)
    add_screen_to_database(DATA.get('screen'), level_id)
    add_screens_to_database(DATA.get('large_screens'), level_id)


if __name__ == "__main__":
    if len(sys.argv) != 2:
        logging.error("Usage: python3 addData.py FILE.json")
        sys.exit(1)
    DATA = get_tombll_json(sys.argv[1])
    CONNECTION = connect_database()
    add_tombll_json_to_database()
    commit_database()
    disconnect_database()


"""
def download_file(url, cert, file_name):
    response = requests.get(url, stream=True, verify=cert, timeout=5)
    response.raise_for_status()

    total_size = int(response.headers.get('content-length', 0))
    block_size = 1024  # 1 Kilobyte
    wrote = 0

    with open(file_name, 'wb') as file:
        for data in tqdm(response.iter_content(block_size),\
                total=(total_size // block_size + 1),\
                         unit='KB', unit_scale=True):
            wrote += len(data)
            file.write(data)

    if total_size not in (0 ,wrote):
        logging.error("ERROR, something went wrong with the download")
    else:
        logging.info("Downloaded %s successfully", file_name)
"""
"""
def add_level_file_list_to_database(data):
    with open(zip_name, 'wb') as zip_file:
        zip_file.write(zip_content)

    with zipfile.ZipFile(zip_name, 'r') as zip_ref:
        zip_ref.extractall('extracted_files')

    for root, dirs, files in os.walk('extracted_files'):
        for file in files:
            file_path = os.path.join(root, file)
            relative_path = os.path.relpath(file_path, 'extracted_files')
            with open(file_path, 'rb') as f:
                file_content = f.read()
                file_md5 = hashlib.md5(file_content, usedforsecurity=False).hexdigest()

                c.execute("SELECT FileID FROM Files WHERE md5sum = ? AND path = ?", \
                (file_md5, relative_path))
                existing_file = c.fetchone()

                if existing_file:
                    file_id = existing_file[0]
                    logging.info("File with md5sum %s and path %s"
                             " already exists. Using existing FileID: %s",
                    file_md5,
                    relative_path,
                    file_id
                    )
                else:
                    c.execute("INSERT INTO Files (md5sum, path) VALUES (?, ?)", \
                    (file_md5, relative_path))
                    file_id = c.lastrowid
                    logging.info("Inserted new file with md5sum %s. New FileID: %s", file_md5, file_id)

            try:
                c.execute("SELECT 1 FROM LevelFileList WHERE fileID = ? AND levelID = ?", \
                (file_id, level_id))
                existing_combination = c.fetchone()

                if not existing_combination:
                    c.execute("INSERT INTO LevelFileList (fileID, levelID) VALUES (?, ?)", \
                    (file_id, level_id))
                else:
                    logging.info("Combination of FileID %s and LevelID %s"
                             " already exists in LevelFileList. Skipping insertion.",
                    file_id,
                    level_id
                    )

            except sqlite3.IntegrityError as e:
                logging.error("Uniqueness violation in LevelFileList: %s", e)
                logging.error("FileID: %s, LevelID: %s", file_id, level_id)


    shutil.rmtree('extracted_files')
    os.remove(zip_name)
"""
"""
    response = requests.get(zip_url, verify=CERT, timeout=5)
    zip_content = response.content

    md5_hash = hashlib.md5(zip_content, usedforsecurity=False).hexdigest()
    zip_md5 = data.get('zipFileMd5')
    if md5_hash != zip_md5:
        logging.error("MD5 checksum does not match")
        sys.exit(1)
"""
