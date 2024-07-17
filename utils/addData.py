"""
Take input trle.net json file and add it to the database
"""
import os
import sys
import shutil
import hashlib
import requests
import sqlite3
import zipfile
import fcntl
import time
import json
import logging

# Set up logging
logging.basicConfig(level=logging.DEBUG, format='%(asctime)s %(levelname)s:%(message)s')
logging.getLogger("requests").setLevel(logging.DEBUG)
logging.getLogger("urllib3").setLevel(logging.DEBUG)

def download_file(url, cert, file_name):
    response = requests.get(url, stream=True, verify=cert)
    response.raise_for_status()

    total_size = int(response.headers.get('content-length', 0))
    block_size = 1024  # 1 Kilobyte
    wrote = 0

    with open(file_name, 'wb') as file:
        for data in tqdm(response.iter_content(block_size), total=total_size // block_size, unit='KB', unit_scale=True):
            wrote += len(data)
            file.write(data)

    if total_size != 0 and wrote != total_size:
        logging.error("ERROR, something went wrong with the download")
    else:
        logging.info(f"Downloaded {file_name} successfully")

if __name__ == "__main__":
    if len(sys.argv) != 2:
        logging.error("Usage: python3 addData.py FILE.json")
        sys.exit(1)
    else:
        file = sys.argv[1]

script_dir = os.path.dirname(os.path.abspath(__file__))
os.chdir(script_dir)

lock_file = '/tmp/TRLE.lock'
try:
    lock_fd = open(lock_file, 'w')
    fcntl.flock(lock_fd, fcntl.LOCK_EX | fcntl.LOCK_NB)
except IOError:
    logging.error("Another instance is already running")
    sys.exit(1)

time.sleep(2)

with open(file, 'r') as json_file:
    file_info = json.load(json_file)

zip_url = file_info.get('download_url')
cert = '/home/noisecode3/mySecretVirusFolder/trle-net-chain.pem'

response = requests.get(zip_url, verify=cert)
zip_content = response.content

md5_hash = hashlib.md5(zip_content).hexdigest()
zip_md5 = file_info.get('zipFileMd5')
if md5_hash != zip_md5:
    logging.error("MD5 checksum does not match")
    sys.exit(1)

conn = sqlite3.connect('tombll.db')
c = conn.cursor()

zip_name = file_info.get('zipFileName')
zip_size = file_info.get('zipFileSize')
zip_url = file_info.get('download_url')
c.execute("INSERT INTO Zip (name, size, md5sum, url) VALUES (?, ?, ?, ?)",
          (zip_name, zip_size, md5_hash, zip_url))

c.execute("SELECT last_insert_rowid()")
zip_id = c.fetchone()[0]

info_title = file_info.get('title')
info_author = file_info.get('author')
info_type = file_info.get('type')
info_class = file_info.get('class_')
info_releaseDate = file_info.get('releaseDate')
info_difficulty = file_info.get('difficulty')
info_duration = file_info.get('duration')

c.execute("SELECT InfoDifficultyID FROM InfoDifficulty WHERE value = ?", (info_difficulty,))
difficulty_id = c.fetchone()[0]

c.execute("SELECT InfoDurationID FROM InfoDuration WHERE value = ?", (info_duration,))
duration_id = c.fetchone()[0]

c.execute("SELECT InfoTypeID FROM InfoType WHERE value = ?", (info_type,))
type_id = c.fetchone()[0]

c.execute("SELECT InfoClassID FROM InfoClass WHERE value = ?", (info_class,))
class_id = c.fetchone()[0]

c.execute('''
INSERT INTO Info (title, author, release, difficulty, duration, type, class)
VALUES (?,?,?,?,?,?,?)
''', (
    info_title,
    info_author,
    info_releaseDate,
    difficulty_id,
    duration_id,
    type_id,
    class_id
))

c.execute("SELECT last_insert_rowid()")
info_id = c.fetchone()[0]

level_body = file_info.get('body')
level_walkthrough = file_info.get('walkthrough')

try:
    c.execute("INSERT INTO Level (body, walkthrough, zipID, infoID) VALUES (?, ?, ?, ?)",
    (level_body, level_walkthrough, zip_id, info_id))
except sqlite3.Error as e:
    logging.error(f"SQLite error: {e}")

c.execute("SELECT MAX(LevelID) FROM Level")
level_id = c.fetchone()[0]
logging.info(f"Current level_id: {level_id}")

screen_url = file_info.get('screen')
screen_response = requests.get(screen_url, verify=cert)
screen_content = screen_response.content
screen_file_name = os.path.basename(screen_url)
c.execute("INSERT INTO Picture (data) VALUES (?)", (screen_content,))
c.execute("SELECT last_insert_rowid()")
screen_id = c.fetchone()[0]
c.execute("INSERT INTO Screens (pictureID, levelID) VALUES (?, ?)", (screen_id, level_id))

for screen_large in file_info.get("screensLarge", []):
    response = requests.get(screen_large, verify=cert)
    screen_content = response.content
    file_name = os.path.basename(screen_large)
    c.execute("INSERT INTO Picture (data) VALUES (?)", (screen_content,))
    c.execute("SELECT last_insert_rowid()")
    screen_id = c.fetchone()[0]
    c.execute("INSERT INTO Screens (pictureID, levelID) VALUES (?, ?)", (screen_id, level_id))

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
            file_md5 = hashlib.md5(file_content).hexdigest()

            c.execute("SELECT FileID FROM Files WHERE md5sum = ? AND path = ?", (file_md5, relative_path))
            existing_file = c.fetchone()

            if existing_file:
                file_id = existing_file[0]
                logging.info(f"File with md5sum {file_md5} and path {relative_path} already exists. Using existing FileID: {file_id}")
            else:
                c.execute("INSERT INTO Files (md5sum, path) VALUES (?, ?)", (file_md5, relative_path))
                file_id = c.lastrowid
                logging.info(f"Inserted new file with md5sum {file_md5}. New FileID: {file_id}")

        try:
            c.execute("SELECT 1 FROM LevelFileList WHERE fileID = ? AND levelID = ?", (file_id, level_id))
            existing_combination = c.fetchone()

            if not existing_combination:
                c.execute("INSERT INTO LevelFileList (fileID, levelID) VALUES (?, ?)", (file_id, level_id))
            else:
                logging.info(f"Combination of FileID {file_id} and LevelID {level_id} already exists in LevelFileList. Skipping insertion.")

        except sqlite3.IntegrityError as e:
            logging.error(f"Uniqueness violation in LevelFileList: {e}")
            logging.error(f"FileID: {file_id}, LevelID: {level_id}")

conn.commit()
conn.close()

shutil.rmtree('extracted_files')
os.remove(zip_name)

lock_fd.close()
os.remove(lock_file)
