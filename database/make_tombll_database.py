"""Make database file for trle.net and TRCustoms.org."""
import sqlite3
import json


def run():
    """Run the main funtion for the module and create the database."""
    connection = sqlite3.connect('tombll.db')
    c = connection.cursor()

    c.execute('''
    CREATE TABLE Version (
        id INTEGER PRIMARY KEY CHECK (id = 1),
        value TEXT -- Your desired columns
    )''')

    c.execute('''INSERT INTO Version (id, value) VALUES (1, '0.0.1')''')

    c.execute('''
    CREATE TRIGGER limit_singleton
    BEFORE INSERT ON Version
    WHEN (SELECT COUNT(*) FROM Version) >= 1
    BEGIN
        SELECT RAISE(FAIL, 'Only one record is allowed.');
    END;''')

    _base_tables(c)
    _static_data(c)
    _file_date(c)

    connection.commit()
    connection.close()


def _base_tables(c):
    c.execute('''
    CREATE TABLE InfoDifficulty (
        InfoDifficultyID INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
        value TEXT NOT NULL UNIQUE
    )''')

    c.execute('''
    CREATE TABLE InfoDuration (
        InfoDurationID INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
        value TEXT NOT NULL UNIQUE
    )''')

    c.execute('''
    CREATE TABLE InfoType (
        InfoTypeID INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
        value TEXT NOT NULL UNIQUE
    )''')

    c.execute('''
    CREATE TABLE InfoClass (
        InfoClassID INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
        value TEXT NOT NULL UNIQUE
    )''')

    c.execute('''
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

    c.execute('''
    CREATE TABLE Author (
        AuthorID INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
        value TEXT NOT NULL UNIQUE
    )''')

    c.execute('''
    CREATE TABLE Genre (
        GenreID INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
        value TEXT NOT NULL UNIQUE
    )''')

    c.execute('''
    CREATE TABLE Tag (
        TagID INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
        value TEXT NOT NULL UNIQUE
    )''')

    c.execute('''
    CREATE TABLE Zip (
        ZipID INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
        name TEXT NOT NULL UNIQUE,
        size FLOAT NOT NULL,
        md5sum TEXT NOT NULL,
        url TEXT UNIQUE,
        version INT,
        release DATE
    )''')

    c.execute('''
    CREATE TABLE File (
        FileID INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
        md5sum TEXT NOT NULL,
        path TEXT NOT NULL,
        UNIQUE (md5sum, path)
    )''')

    c.execute('''
    CREATE TABLE LevelFileList (
        fileID INTEGER NOT NULL,
        levelID INTEGER NOT NULL,
        PRIMARY KEY (fileID, levelID),
        FOREIGN KEY (fileID) REFERENCES File(FileID),
        FOREIGN KEY (levelID) REFERENCES Level(LevelID)
    )''')

    c.execute('''
    CREATE TABLE GameFileList (
        fileID INTEGER NOT NULL,
        gameID INTEGER NOT NULL,
        PRIMARY KEY (fileID, gameID),
        FOREIGN KEY (fileID) REFERENCES File(FileID),
        FOREIGN KEY (gameID) REFERENCES Game(GameID)
    )''')

    c.execute('''
    CREATE TABLE Picture (
        PictureID INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
        md5sum TEXT NOT NULL UNIQUE,
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
        walkthrough TEXT NOT NULL,
        infoID INTEGER NOT NULL,
        FOREIGN KEY (infoID) REFERENCES Info(InfoID)
    )''')

    c.execute('''
    CREATE TABLE Screens (
        pictureID INTEGER NOT NULL,
        levelID INTEGER NOT NULL,
        position INTEGER NOT NULL,
        PRIMARY KEY (pictureID, levelID),
        UNIQUE (levelID, position),
        FOREIGN KEY (pictureID) REFERENCES Picture(PictureID),
        FOREIGN KEY (levelID) REFERENCES Level(LevelID)
    )''')

    c.execute('''
    CREATE TABLE ZipList (
        zipID INTEGER NOT NULL,
        levelID INTEGER NOT NULL,
        PRIMARY KEY (zipID, levelID),
        FOREIGN KEY (zipID) REFERENCES Zip(ZipID),
        FOREIGN KEY (levelID) REFERENCES Level(LevelID)
    )''')

    c.execute('''
    CREATE TABLE AuthorList (
        authorID INTEGER NOT NULL,
        levelID INTEGER NOT NULL,
        PRIMARY KEY (authorID, levelID),
        FOREIGN KEY (authorID) REFERENCES Author(AuthorID),
        FOREIGN KEY (levelID) REFERENCES Level(LevelID)
    )''')

    c.execute('''
    CREATE TABLE GenreList (
        genreID INTEGER NOT NULL,
        levelID INTEGER NOT NULL,
        PRIMARY KEY (genreID, levelID),
        FOREIGN KEY (genreID) REFERENCES Genre(GenreID),
        FOREIGN KEY (levelID) REFERENCES Level(LevelID)
    )''')

    c.execute('''
    CREATE TABLE TagList (
        tagID INTEGER NOT NULL,
        levelID INTEGER NOT NULL,
        PRIMARY KEY (tagID, levelID),
        FOREIGN KEY (tagID) REFERENCES Tag(TagID),
        FOREIGN KEY (levelID) REFERENCES Level(LevelID)
    )''')


def _static_data(c):
    info_class_values = [
        ('Alien/Space',), ('Atlantis',), ('Base/Lab',), ('Cambodia',),
        ('Castle',), ('Cave/Cat',), ('City',), ('Coastal',), ('Cold/Snowy',),
        ('Desert',), ('Easter',), ('Egypt',), ('Fantasy/Surreal',), ('Home',),
        ('Ireland',), ('Joke',), ('Jungle',), ('Kids',), ('Library',),
        ('Mystery/Horror',), ('nc',), ('Nordic/Celtic',), ('Oriental',),
        ('Persia',), ('Pirates',), ('Remake',), ('Rome/Greece',), ('Ship',),
        ('Shooter',), ('South America',), ('South Pacific',), ('Steampunk',),
        ('Train',), ('Venice',), ('Wild West',), ('Xmas',), ('Young Lara',)
    ]
    c.executemany("INSERT INTO InfoClass (value) VALUES (?)", info_class_values)

    difficulty_values = [
        ('easy',), ('medium',), ('challenging',), ('very challenging',)
    ]
    c.executemany("INSERT INTO InfoDifficulty (value) VALUES (?)", difficulty_values)

    type_values = [
        ('TR1',), ('TR2',), ('TR3',), ('TR4',), ('TR5',), ('TEN',)
    ]
    c.executemany("INSERT INTO InfoType (value) VALUES (?)", type_values)

    duration_values = [
        ('short',), ('medium',), ('long',), ('very long',)
    ]
    c.executemany("INSERT INTO InfoDuration (value) VALUES (?)", duration_values)


def _file_date(c):
    # Add Game File data
    for i in range(1, 6):
        # Load data from JSON file
        with open(f'data/fileList-TR{i}.json', 'r', encoding='utf-8') as json_file:
            file_info = json.load(json_file)

        name = file_info.get("name")

        c.execute("INSERT INTO Game (name) VALUES (?)", (name,))
        game_id = c.lastrowid

        file_data_list = file_info.get("file_data_list")

        for entry in file_data_list:
            relative_path = entry.get("filename")
            file_md5 = entry.get("md5sum")

            if relative_path and file_md5:
                # Check if the file with the same md5sum already exists in File table
                c.execute(
                    "SELECT FileID FROM File WHERE md5sum = ? AND path = ?",
                    (file_md5, relative_path)
                )
                existing_file = c.fetchone()

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
                    c.execute(
                        "INSERT INTO File (md5sum, path) VALUES (?, ?)",
                        (file_md5, relative_path)
                    )
                    file_id = c.lastrowid
                    print(f"Inserted new file with md5sum {file_md5}. New FileID: {file_id}")

                try:
                    # Check if the combination of fileID and gameID already exists in GameFileList
                    c.execute(
                        "SELECT 1 FROM GameFileList WHERE fileID = ? AND gameID = ?",
                        (file_id, game_id)
                    )
                    existing_combination = c.fetchone()

                    if not existing_combination:
                        # Combination doesn't exist, insert it into GameFileList
                        c.execute(
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
