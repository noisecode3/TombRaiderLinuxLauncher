"""Make database file for trle.net and TRCustoms.org."""
import sqlite3
import json
import os

os.chdir(os.path.dirname(os.path.abspath(__file__)))


CONNECTION = sqlite3.connect('tombll.db')
C = CONNECTION.cursor()

C.execute('''
CREATE TABLE InfoDifficulty (
    InfoDifficultyID INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
    value TEXT NOT NULL UNIQUE
)''')

C.execute('''
CREATE TABLE InfoDuration (
    InfoDurationID INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
    value TEXT NOT NULL UNIQUE
)''')

C.execute('''
CREATE TABLE InfoType (
    InfoTypeID INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
    value TEXT NOT NULL UNIQUE
)''')

C.execute('''
CREATE TABLE InfoClass (
    InfoClassID INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
    value TEXT NOT NULL UNIQUE
)''')

C.execute('''
CREATE TABLE Info (
    InfoID INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
    title TEXT NOT NULL,
    release DATE NOT NULL,
    difficulty INT,
    duration INT,
    type INT NOT NULL,
    class INT,
    trleID INT UNIQUE,
    trcustomsID INT UNIQUE,
    FOREIGN KEY (difficulty) REFERENCES InfoDifficulty(InfoDifficultyID),
    FOREIGN KEY (duration) REFERENCES InfoDuration(InfoDurationID),
    FOREIGN KEY (type) REFERENCES InfoType(InfoTypeID),
    FOREIGN KEY (class) REFERENCES InfoClass(InfoClassID)
)''')

C.execute('''
CREATE TABLE Author (
    AuthorID INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
    value TEXT NOT NULL UNIQUE
)''')

C.execute('''
CREATE TABLE Genre (
    GenreID INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
    value TEXT NOT NULL UNIQUE
)''')

C.execute('''
CREATE TABLE Tag (
    TagID INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
    value TEXT NOT NULL UNIQUE
)''')

C.execute('''
CREATE TABLE Zip (
    ZipID INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
    name TEXT NOT NULL UNIQUE,
    size FLOAT NOT NULL,
    md5sum TEXT NOT NULL,
    url TEXT UNIQUE,
    version INT,
    release DATE
)''')

C.execute('''
CREATE TABLE File (
    FileID INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
    md5sum TEXT NOT NULL,
    path TEXT NOT NULL,
    UNIQUE (md5sum, path)
)''')

C.execute('''
CREATE TABLE LevelFileList (
    fileID INTEGER NOT NULL,
    levelID INTEGER NOT NULL,
    PRIMARY KEY (fileID, levelID),
    FOREIGN KEY (fileID) REFERENCES File(FileID),
    FOREIGN KEY (levelID) REFERENCES Level(LevelID)
)''')

C.execute('''
CREATE TABLE GameFileList (
    fileID INTEGER NOT NULL,
    gameID INTEGER NOT NULL,
    PRIMARY KEY (fileID, gameID),
    FOREIGN KEY (fileID) REFERENCES File(FileID),
    FOREIGN KEY (gameID) REFERENCES Game(GameID)
)''')

C.execute('''
CREATE TABLE Picture (
    PictureID INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
    data BLOB NOT NULL
)''')

C.execute('''
CREATE TABLE Game (
    GameID INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
    name TEXT NOT NULL
)''')

C.execute('''
CREATE TABLE Level (
    LevelID INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
    body TEXT NOT NULL,
    walkthrough TEXT NOT NULL,
    infoID INTEGER NOT NULL,
    FOREIGN KEY (infoID) REFERENCES Info(InfoID)
)''')

C.execute('''
CREATE TABLE Screens (
    pictureID INTEGER NOT NULL,
    levelID INTEGER NOT NULL,
    PRIMARY KEY (pictureID, levelID),
    FOREIGN KEY (pictureID) REFERENCES Picture(PictureID),
    FOREIGN KEY (levelID) REFERENCES Level(LevelID)
)''')

C.execute('''
CREATE TABLE ZipList (
    zipID INTEGER NOT NULL,
    levelID INTEGER NOT NULL,
    PRIMARY KEY (zipID, levelID),
    FOREIGN KEY (zipID) REFERENCES Zip(ZipID),
    FOREIGN KEY (levelID) REFERENCES Level(LevelID)
)''')

C.execute('''
CREATE TABLE AuthorList (
    authorID INTEGER NOT NULL,
    levelID INTEGER NOT NULL,
    PRIMARY KEY (authorID, levelID),
    FOREIGN KEY (authorID) REFERENCES Author(AuthorID),
    FOREIGN KEY (levelID) REFERENCES Level(LevelID)
)''')

C.execute('''
CREATE TABLE GenreList (
    genreID INTEGER NOT NULL,
    levelID INTEGER NOT NULL,
    PRIMARY KEY (genreID, levelID),
    FOREIGN KEY (genreID) REFERENCES Genre(GenreID),
    FOREIGN KEY (levelID) REFERENCES Level(LevelID)
)''')

C.execute('''
CREATE TABLE TagList (
    tagID INTEGER NOT NULL,
    levelID INTEGER NOT NULL,
    PRIMARY KEY (tagID, levelID),
    FOREIGN KEY (tagID) REFERENCES Tag(TagID),
    FOREIGN KEY (levelID) REFERENCES Level(LevelID)
)''')

C.execute("INSERT INTO infoclass (value) VALUES (?)", ('Alien/Space',))
C.execute("INSERT INTO InfoClass (value) VALUES (?)", ('Atlantis',))
C.execute("INSERT INTO InfoClass (value) VALUES (?)", ('Base/Lab',))
C.execute("INSERT INTO InfoClass (value) VALUES (?)", ('Cambodia',))
C.execute("INSERT INTO InfoClass (value) VALUES (?)", ('Castle',))
C.execute("INSERT INTO InfoClass (value) VALUES (?)", ('Cave/Cat',))
C.execute("INSERT INTO InfoClass (value) VALUES (?)", ('City',))
C.execute("INSERT INTO InfoClass (value) VALUES (?)", ('Coastal',))
C.execute("INSERT INTO InfoClass (value) VALUES (?)", ('Cold/Snowy',))
C.execute("INSERT INTO InfoClass (value) VALUES (?)", ('Desert',))
C.execute("INSERT INTO InfoClass (value) VALUES (?)", ('Easter',))
C.execute("INSERT INTO InfoClass (value) VALUES (?)", ('Egypt',))
C.execute("INSERT INTO InfoClass (value) VALUES (?)", ('Fantasy/Surreal',))
C.execute("INSERT INTO InfoClass (value) VALUES (?)", ('Home',))
C.execute("INSERT INTO InfoClass (value) VALUES (?)", ('Ireland',))
C.execute("INSERT INTO InfoClass (value) VALUES (?)", ('Joke',))
C.execute("INSERT INTO InfoClass (value) VALUES (?)", ('Jungle',))
C.execute("INSERT INTO InfoClass (value) VALUES (?)", ('Kids',))
C.execute("INSERT INTO InfoClass (value) VALUES (?)", ('Library',))
C.execute("INSERT INTO InfoClass (value) VALUES (?)", ('Mystery/Horror',))
C.execute("INSERT INTO InfoClass (value) VALUES (?)", ('nc',))
C.execute("INSERT INTO InfoClass (value) VALUES (?)", ('Nordic/Celtic',))
C.execute("INSERT INTO InfoClass (value) VALUES (?)", ('Oriental',))
C.execute("INSERT INTO InfoClass (value) VALUES (?)", ('Persia',))
C.execute("INSERT INTO InfoClass (value) VALUES (?)", ('Pirates',))
C.execute("INSERT INTO InfoClass (value) VALUES (?)", ('Remake',))
C.execute("INSERT INTO InfoClass (value) VALUES (?)", ('Rome/Greece',))
C.execute("INSERT INTO InfoClass (value) VALUES (?)", ('Ship',))
C.execute("INSERT INTO InfoClass (value) VALUES (?)", ('Shooter',))
C.execute("INSERT INTO InfoClass (value) VALUES (?)", ('South America',))
C.execute("INSERT INTO InfoClass (value) VALUES (?)", ('South Pacific',))
C.execute("INSERT INTO InfoClass (value) VALUES (?)", ('Steampunk',))
C.execute("INSERT INTO InfoClass (value) VALUES (?)", ('Train',))
C.execute("INSERT INTO InfoClass (value) VALUES (?)", ('Venice',))
C.execute("INSERT INTO InfoClass (value) VALUES (?)", ('Wild West',))
C.execute("INSERT INTO InfoClass (value) VALUES (?)", ('Xmas',))
C.execute("INSERT INTO InfoClass (value) VALUES (?)", ('Young Lara',))

C.execute("INSERT INTO InfoDifficulty (value) VALUES (?)", ('easy',))
C.execute("INSERT INTO InfoDifficulty (value) VALUES (?)", ('medium',))
C.execute("INSERT INTO InfoDifficulty (value) VALUES (?)", ('challenging',))
C.execute("INSERT INTO InfoDifficulty (value) VALUES (?)", ('very challenging',))

C.execute("INSERT INTO InfoType (value) VALUES (?)", ('TR1',))
C.execute("INSERT INTO InfoType (value) VALUES (?)", ('TR2',))
C.execute("INSERT INTO InfoType (value) VALUES (?)", ('TR3',))
C.execute("INSERT INTO InfoType (value) VALUES (?)", ('TR4',))
C.execute("INSERT INTO InfoType (value) VALUES (?)", ('TR5',))
C.execute("INSERT INTO InfoType (value) VALUES (?)", ('TEN',))

C.execute("INSERT INTO InfoDuration (value) VALUES (?)", ('short',))
C.execute("INSERT INTO InfoDuration (value) VALUES (?)", ('medium',))
C.execute("INSERT INTO InfoDuration (value) VALUES (?)", ('long',))
C.execute("INSERT INTO InfoDuration (value) VALUES (?)", ('very long',))

# Add Game File data
for i in range(1, 6):
    # Load data from JSON file
    with open(f'data/fileList-TR{i}.json', 'r', encoding='utf-8') as json_file:
        file_info = json.load(json_file)

    name = file_info.get("name")

    C.execute("INSERT INTO Game (name) VALUES (?)", (name,))
    game_id = C.lastrowid

    file_data_list = file_info.get("file_data_list")

    for entry in file_data_list:
        relative_path = entry.get("filename")
        file_md5 = entry.get("md5sum")

        if relative_path and file_md5:
            # Check if the file with the same md5sum already exists in File table
            C.execute(
                "SELECT FileID FROM File WHERE md5sum = ? AND path = ?",
                (file_md5, relative_path)
            )
            existing_file = C.fetchone()

            if existing_file:
                # File already exists, use the existing FileID
                file_id = existing_file[0]
                print(
                    "File with md5sum %s and path %s already exists. Using existing FileID: %s",
                    file_md5,
                    relative_path,
                    file_id
                )
            else:
                # File doesn't exist, insert it into File table
                C.execute(
                    "INSERT INTO File (md5sum, path) VALUES (?, ?)",
                    (file_md5, relative_path)
                )
                file_id = C.lastrowid
                print(f"Inserted new file with md5sum {file_md5}. New FileID: {file_id}")

            try:
                # Check if the combination of fileID and gameID already exists in GameFileList
                C.execute(
                    "SELECT 1 FROM GameFileList WHERE fileID = ? AND gameID = ?",
                    (file_id, game_id)
                )
                existing_combination = C.fetchone()

                if not existing_combination:
                    # Combination doesn't exist, insert it into GameFileList
                    C.execute(
                        "INSERT INTO GameFileList (fileID, gameID) VALUES (?, ?)",
                        (file_id, game_id)
                    )
                else:
                    # Combination already exists, print a message or handle it as needed
                    print(
                        f"Combination of FileID {file_id} and LevelID {game_id} "
                        "already exists in GameFileList. Skipping insertion."
                    )

            except sqlite3.IntegrityError as file_list_error:
                # Print more details about the uniqueness violation
                print(f"Uniqueness violation in GameFileList: {file_list_error}")
                print(f"FileID: {file_id}, LevelID: {game_id}")

CONNECTION.commit()
CONNECTION.close()
