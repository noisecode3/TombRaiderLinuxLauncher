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
if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Usage: python3 addData.py FILE.json")
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
    print("Another instance is already running")
    sys.exit(1)

time.sleep(2)

with open(file, 'r') as json_file:
    file_info = json.load(json_file)

zip_url = file_info.get('download_url')
response = requests.get(zip_url)
zip_content = response.content

md5_hash = hashlib.md5(zip_content).hexdigest()
zip_md5 = file_info.get('zipFileMd5')
if md5_hash != zip_md5:
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
c.execute('''INSERT INTO Info (title, author, release, difficulty, duration, type, class)
VALUES (?,?,?,?,?,?,?)''', (
    info_title,
    info_author,
    info_releaseDate,
    info_difficulty,
    info_duration,
    info_type,
    info_class)
)
c.execute("SELECT last_insert_rowid()")
info_id = c.fetchone()[0]

screen_url = file_info.get('screen')
screen_response = requests.get(screen_url)
screen_content = screen_response.content
screen_file_name = os.path.basename(screen_url)
c.execute("INSERT INTO Picture (name, data) VALUES (?, ?)", (screen_file_name, screen_content))
c.execute("SELECT last_insert_rowid()")
screen_id = c.fetchone()[0]

# for screen_large in file_info.get("screensLarge", []):
#    response = requests.get(screen_large)
#    screen_content = response.content
#    file_name = os.path.basename(screen_large)
#    c.execute("INSERT INTO Picture (name, data) VALUES (?, ?)", (file_name, screen_content))

level_body = file_info.get('body')
level_walkthrough = file_info.get('walkthrough')

try:
    c.execute("INSERT INTO Level (screenName, screenLargeNames, body, walkthrough, pictureID, zipID, infoID) VALUES (?, ?, ?, ?, ?, ?, ?)",
    ("", "", level_body, level_walkthrough, screen_id, zip_id, info_id))
except sqlite3.Error as e:
    print(f"SQLite error: {e}")

c.execute("SELECT MAX(LevelID) FROM Level")
level_id = c.fetchone()[0]
print("Current level_id:", level_id)

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

            # Check if the file with the same md5sum already exists in Files table
            c.execute("SELECT FileID FROM Files WHERE md5sum = ? AND path = ?", (file_md5, relative_path))
            existing_file = c.fetchone()

            if existing_file:
                # File already exists, use the existing FileID
                file_id = existing_file[0]
                print(f"File with md5sum {file_md5} and path {relative_path} already exists. Using existing FileID: {file_id}")
            else:
                # File doesn't exist, insert it into Files table
                c.execute("INSERT INTO Files (md5sum, path) VALUES (?, ?)", (file_md5, relative_path))
                file_id = c.lastrowid
                print(f"Inserted new file with md5sum {file_md5}. New FileID: {file_id}")

        try:
            # Check if the combination of fileID and levelID already exists in LevelFileList
            c.execute("SELECT 1 FROM LevelFileList WHERE fileID = ? AND levelID = ?", (file_id, level_id))
            existing_combination = c.fetchone()

            if not existing_combination:
                # Combination doesn't exist, insert it into LevelFileList
                c.execute("INSERT INTO LevelFileList (fileID, levelID) VALUES (?, ?)", (file_id, level_id))
            else:
                # Combination already exists, print a message or handle it as needed
                print(f"Combination of FileID {file_id} and LevelID {level_id} already exists in LevelFileList. Skipping insertion.")

        except sqlite3.IntegrityError as e:
            # Print more details about the uniqueness violation
            print(f"Uniqueness violation in LevelFileList: {e}")
            print(f"FileID: {file_id}, LevelID: {level_id}")

conn.commit()
conn.close()

shutil.rmtree('extracted_files')
os.remove(zip_name)

lock_fd.close()
os.remove(lock_file)
