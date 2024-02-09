"""
Make database file
"""
import sqlite3
import json
import os

script_dir = os.path.dirname(os.path.abspath(__file__))
os.chdir(script_dir)

conn = sqlite3.connect('tombll.db')
c = conn.cursor()

c.execute('''
CREATE TABLE InfoDifficulty (
    InfoDifficultyID INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
    value TEXT NOT NULL
)''')

c.execute('''
CREATE TABLE InfoDuration (
    InfoDurationID INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
    value TEXT NOT NULL
)''')

c.execute('''
CREATE TABLE InfoType (
    InfoTypeID INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
    value TEXT NOT NULL
)''')

c.execute('''
CREATE TABLE InfoClass (
    InfoClassID INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
    value TEXT NOT NULL
)''')

c.execute('''
CREATE TABLE Info (
    InfoID INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
    title TEXT NOT NULL,
    author TEXT NOT NULL,
    release DATE NOT NULL,
    difficulty INT NOT NULL,
    duration INT NOT NULL,
    type INT NOT NULL,
    class INT NOT NULL,
    FOREIGN KEY (difficulty) REFERENCES InfoDifficulty(InfoDifficultyID),
    FOREIGN KEY (duration) REFERENCES InfoDuration(InfoDurationID),
    FOREIGN KEY (type) REFERENCES InfoType(InfoTypeID),
    FOREIGN KEY (class) REFERENCES InfoClass(InfoClassID)
)''')

c.execute('''
CREATE TABLE Zip (
    ZipID INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
    name TEXT NOT NULL,
    size FLOAT NOT NULL,
    md5sum TEXT NOT NULL,
    url TEXT
)''')

c.execute('''
CREATE TABLE Files (
    FileID INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
    md5sum TEXT NOT NULL,
    path TEXT NOT NULL
)''')

c.execute('''
CREATE TABLE LevelFileList (
    fileID INTEGER NOT NULL,
    levelID INTEGER NOT NULL,
    PRIMARY KEY (fileID, levelID),
    FOREIGN KEY (fileID) REFERENCES Files(FileID),
    FOREIGN KEY (levelID) REFERENCES Level(LevelID)
)''')

c.execute('''
CREATE TABLE GameFileList (
    fileID INTEGER NOT NULL,
    gameID INTEGER NOT NULL,
    PRIMARY KEY (fileID, gameID),
    FOREIGN KEY (fileID) REFERENCES Files(FileID),
    FOREIGN KEY (gameID) REFERENCES Game(GameID)
)''')

c.execute('''
CREATE TABLE Picture (
    PictureID INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
    data BLOB NOT NULL
)''')

c.execute('''
CREATE TABLE Game (
    GameID INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
    name TEXT NOT NULL
)''')

c.execute('''
CREATE TABLE Level (
    LevelID INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
    body TEXT NOT NULL,
    walkthrough  TEXT NOT NULL,
    zipID INTEGER NOT NULL,
    infoID INTEGER NOT NULL,
    FOREIGN KEY (zipID) REFERENCES Zip(ZipID),
    FOREIGN KEY (infoID) REFERENCES Info(InfoID)
)''')

c.execute('''
CREATE TABLE Screens (
    pictureID INTEGER NOT NULL,
    levelID INTEGER NOT NULL,
    PRIMARY KEY (pictureID, levelID),
    FOREIGN KEY (pictureID) REFERENCES Picture(PictureID),
    FOREIGN KEY (levelID) REFERENCES Level(LevelID)
)''')

c.execute("INSERT INTO InfoClass (value) VALUES (?)", ('Alien/Space',))
c.execute("INSERT INTO InfoClass (value) VALUES (?)", ('Atlantis',))
c.execute("INSERT INTO InfoClass (value) VALUES (?)", ('Base/Lab',))
c.execute("INSERT INTO InfoClass (value) VALUES (?)", ('Cambodia',))
c.execute("INSERT INTO InfoClass (value) VALUES (?)", ('Castle',))
c.execute("INSERT INTO InfoClass (value) VALUES (?)", ('Cave/Cat',))
c.execute("INSERT INTO InfoClass (value) VALUES (?)", ('City',))
c.execute("INSERT INTO InfoClass (value) VALUES (?)", ('Coastal',))
c.execute("INSERT INTO InfoClass (value) VALUES (?)", ('Cold/Snowy',))
c.execute("INSERT INTO InfoClass (value) VALUES (?)", ('Desert',))
c.execute("INSERT INTO InfoClass (value) VALUES (?)", ('Easter',))
c.execute("INSERT INTO InfoClass (value) VALUES (?)", ('Egypt',))
c.execute("INSERT INTO InfoClass (value) VALUES (?)", ('Fantasy/Surreal',))
c.execute("INSERT INTO InfoClass (value) VALUES (?)", ('Home',))
c.execute("INSERT INTO InfoClass (value) VALUES (?)", ('Ireland',))
c.execute("INSERT INTO InfoClass (value) VALUES (?)", ('Joke',))
c.execute("INSERT INTO InfoClass (value) VALUES (?)", ('Jungle',))
c.execute("INSERT INTO InfoClass (value) VALUES (?)", ('Kids',))
c.execute("INSERT INTO InfoClass (value) VALUES (?)", ('Library',))
c.execute("INSERT INTO InfoClass (value) VALUES (?)", ('Mystery/Horror',))
c.execute("INSERT INTO InfoClass (value) VALUES (?)", ('nc',))
c.execute("INSERT INTO InfoClass (value) VALUES (?)", ('Nordic/Celtic',))
c.execute("INSERT INTO InfoClass (value) VALUES (?)", ('Oriental',))
c.execute("INSERT INTO InfoClass (value) VALUES (?)", ('Persia',))
c.execute("INSERT INTO InfoClass (value) VALUES (?)", ('Pirates',))
c.execute("INSERT INTO InfoClass (value) VALUES (?)", ('Remake',))
c.execute("INSERT INTO InfoClass (value) VALUES (?)", ('Rome/Greece',))
c.execute("INSERT INTO InfoClass (value) VALUES (?)", ('Ship',))
c.execute("INSERT INTO InfoClass (value) VALUES (?)", ('Shooter',))
c.execute("INSERT INTO InfoClass (value) VALUES (?)", ('South America',))
c.execute("INSERT INTO InfoClass (value) VALUES (?)", ('South Pacific',))
c.execute("INSERT INTO InfoClass (value) VALUES (?)", ('Steampunk',))
c.execute("INSERT INTO InfoClass (value) VALUES (?)", ('Train',))
c.execute("INSERT INTO InfoClass (value) VALUES (?)", ('Venice',))
c.execute("INSERT INTO InfoClass (value) VALUES (?)", ('Wild West',))
c.execute("INSERT INTO InfoClass (value) VALUES (?)", ('Xmas',))
c.execute("INSERT INTO InfoClass (value) VALUES (?)", ('Young Lara',))

c.execute("INSERT INTO InfoDifficulty (value) VALUES (?)", ('easy',))
c.execute("INSERT INTO InfoDifficulty (value) VALUES (?)", ('medium',))
c.execute("INSERT INTO InfoDifficulty (value) VALUES (?)", ('challenging',))
c.execute("INSERT INTO InfoDifficulty (value) VALUES (?)", ('very challenging',))

c.execute("INSERT INTO InfoType (value) VALUES (?)", ('TR1',))
c.execute("INSERT INTO InfoType (value) VALUES (?)", ('TR2',))
c.execute("INSERT INTO InfoType (value) VALUES (?)", ('TR3',))
c.execute("INSERT INTO InfoType (value) VALUES (?)", ('TR4',))
c.execute("INSERT INTO InfoType (value) VALUES (?)", ('TR5',))
c.execute("INSERT INTO InfoType (value) VALUES (?)", ('TEN',))

c.execute("INSERT INTO InfoDuration (value) VALUES (?)", ('short',))
c.execute("INSERT INTO InfoDuration (value) VALUES (?)", ('medium',))
c.execute("INSERT INTO InfoDuration (value) VALUES (?)", ('long',))
c.execute("INSERT INTO InfoDuration (value) VALUES (?)", ('very long',))

# Add Game File data
for i in range(1, 6):
    file = f'fileList-TR{i}.json'
    
    # Load data from JSON file
    with open(file, 'r') as json_file:
        file_info = json.load(json_file)

    name = file_info.get("name")

    c.execute("INSERT INTO Game (name) VALUES (?)", (name,))
    game_id = c.lastrowid

    file_data_list = file_info.get("file_data_list")

    for entry in file_data_list:
        relative_path = entry.get("filename")
        file_md5 = entry.get("md5sum")

        if relative_path and file_md5:
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
            # Check if the combination of fileID and gameID already exists in GameFileList
            c.execute("SELECT 1 FROM GameFileList WHERE fileID = ? AND gameID = ?", (file_id, game_id))
            existing_combination = c.fetchone()

            if not existing_combination:
                # Combination doesn't exist, insert it into GameFileList
                c.execute("INSERT INTO GameFileList (fileID, gameID) VALUES (?, ?)", (file_id, game_id))
            else:
                # Combination already exists, print a message or handle it as needed
                print(f"Combination of FileID {file_id} and LevelID {game_id} already exists in GameFileList. Skipping insertion.")

        except sqlite3.IntegrityError as e:
            # Print more details about the uniqueness violation
            print(f"Uniqueness violation in GameFileList: {e}")
            print(f"FileID: {file_id}, LevelID: {game_id}")

conn.commit()
conn.close()
