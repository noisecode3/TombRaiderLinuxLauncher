import sys
import logging
import sqlite3
from PIL import Image
from io import BytesIO

def update_table_schema_and_data(db_path):
    """Update from 1.0.0 database to 1.1.0"""
    conn = sqlite3.connect(db_path)
    cursor = conn.cursor()

    try:
        cursor.execute('BEGIN TRANSACTION;')

        cursor.execute('ALTER TABLE InfoDifficulty RENAME TO InfoDifficulty_old;')
        cursor.execute('ALTER TABLE InfoDuration RENAME TO InfoDuration_old;')
        cursor.execute('ALTER TABLE InfoType RENAME TO InfoType_old;')
        cursor.execute('ALTER TABLE InfoClass RENAME TO InfoClass_old;')

        cursor.execute('''CREATE TABLE InfoDifficulty (
            InfoDifficultyID INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
            value TEXT NOT NULL UNIQUE
        );''')

        cursor.execute('''CREATE TABLE InfoDuration (
            InfoDurationID INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
            value TEXT NOT NULL UNIQUE
        );''')

        cursor.execute('''CREATE TABLE InfoType (
            InfoTypeID INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
            value TEXT NOT NULL UNIQUE
        );''')

        cursor.execute('''CREATE TABLE InfoClass (
            InfoClassID INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
            value TEXT NOT NULL UNIQUE
        );''')

        cursor.execute('''INSERT INTO InfoDifficulty (InfoDifficultyID, value)
            SELECT InfoDifficultyID, value FROM InfoDifficulty_old;''')

        cursor.execute('''INSERT INTO InfoDuration (InfoDurationID, value)
            SELECT InfoDurationID, value FROM InfoDuration_old;''')

        cursor.execute('''INSERT INTO InfoType (InfoTypeID, value)
            SELECT InfoTypeID, value FROM InfoType_old;''')

        cursor.execute('''INSERT INTO InfoClass (InfoClassID, value)
            SELECT InfoClassID, value FROM InfoClass_old;''')

        cursor.execute('COMMIT;')
        cursor.execute('DROP TABLE InfoDifficulty_old;')
        cursor.execute('DROP TABLE InfoDuration_old;')
        cursor.execute('DROP TABLE InfoType_old;')
        cursor.execute('DROP TABLE InfoClass_old;')

        cursor.execute('BEGIN TRANSACTION;')
        cursor.execute('ALTER TABLE Info RENAME TO Info_old;')
        cursor.execute('''CREATE TABLE Info (
            InfoID INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
            title TEXT NOT NULL,
            release DATE NOT NULL,
            difficulty INT,
            duration INT,
            type INT NOT NULL,
            class INT,
            trleID INT,
            trcustomsID INT,
            FOREIGN KEY (difficulty) REFERENCES InfoDifficulty(InfoDifficultyID),
            FOREIGN KEY (duration) REFERENCES InfoDuration(InfoDurationID),
            FOREIGN KEY (type) REFERENCES InfoType(InfoTypeID),
            FOREIGN KEY (class) REFERENCES InfoClass(InfoClassID)
        );''')

        cursor.execute('''INSERT INTO Info (InfoID, title, release, difficulty,
            duration, type, class, trleID, trcustomsID)
        SELECT InfoID, title, release, difficulty, duration, type,
            class, NULL AS trleID, NULL AS trcustomsID
        FROM Info_old;''')

        cursor.execute('''CREATE TABLE Author (
            AuthorID INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
            value TEXT NOT NULL UNIQUE
        );''')

        cursor.execute('''INSERT OR IGNORE INTO Author (value)
            SELECT DISTINCT author
            FROM Info_old
        ;''')

        cursor.execute('''CREATE TABLE AuthorList (
            authorID INTEGER NOT NULL,
            levelID INTEGER NOT NULL,
            PRIMARY KEY (authorID, levelID),
            FOREIGN KEY (authorID) REFERENCES Author(AuthorID),
            FOREIGN KEY (levelID) REFERENCES Level(LevelID)
        );''')

        cursor.execute('''INSERT INTO AuthorList (authorID, levelID)
        SELECT Author.AuthorID, Info.InfoID
        FROM Info_old AS Info
        JOIN Author ON Info.author = Author.value;
                       ''')
        cursor.execute('''COMMIT;''')
        cursor.execute('''DROP TABLE Info_old;''')
        cursor.execute('''CREATE TABLE Genre (
            GenreID INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
            value TEXT NOT NULL UNIQUE
        );''')

        cursor.execute('''CREATE TABLE GenreList (
            genreID INTEGER NOT NULL,
            levelID INTEGER NOT NULL,
            PRIMARY KEY (genreID, levelID),
            FOREIGN KEY (genreID) REFERENCES Genre(GenreID),
            FOREIGN KEY (levelID) REFERENCES Level(LevelID)
        );''')

        cursor.execute('''CREATE TABLE Tag (
            TagID INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
            value TEXT NOT NULL UNIQUE
        );''')

        cursor.execute('''CREATE TABLE TagList (
            tagID INTEGER NOT NULL,
            levelID INTEGER NOT NULL,
            PRIMARY KEY (tagID, levelID),
            FOREIGN KEY (tagID) REFERENCES Tag(TagID),
            FOREIGN KEY (levelID) REFERENCES Level(LevelID)
        );''')

        cursor.execute('''BEGIN TRANSACTION;''')
        cursor.execute('''ALTER TABLE Zip RENAME TO Zip_old;''')
        cursor.execute('''CREATE TABLE Zip (
            ZipID INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
            name TEXT NOT NULL UNIQUE,
            size FLOAT NOT NULL,
            md5sum TEXT NOT NULL,
            url TEXT,
            version INT,
            release DATE
        );''')
        cursor.execute('''INSERT INTO Zip (ZipID, name, size, md5sum, url)
        SELECT ZipID, name, size, md5sum, url FROM Zip_old;''')
        cursor.execute('''COMMIT;''')
        cursor.execute('''DROP TABLE Zip_old;''')
        cursor.execute('''BEGIN TRANSACTION;''')
        cursor.execute('''CREATE TABLE File (
            FileID INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
            md5sum TEXT NOT NULL,
            path TEXT NOT NULL,
            UNIQUE (md5sum, path)
        );''')
        cursor.execute('''INSERT INTO File (FileID, md5sum, path)
        SELECT FileID, md5sum, path FROM Files;''')
        cursor.execute('''COMMIT;''')
        cursor.execute('''DROP TABLE Files;''')
        cursor.execute('''BEGIN TRANSACTION;''')
        cursor.execute('''ALTER TABLE LevelFileList RENAME TO LevelFileList_old;''')
        cursor.execute('''CREATE TABLE LevelFileList (
            fileID INTEGER NOT NULL,
            levelID INTEGER NOT NULL,
            PRIMARY KEY (fileID, levelID),
            FOREIGN KEY (fileID) REFERENCES File(FileID),
            FOREIGN KEY (levelID) REFERENCES Level(LevelID)
        );''')

        cursor.execute('''INSERT INTO LevelFileList (fileID, levelID )
        SELECT fileID, levelID FROM LevelFileList_old;''')
        cursor.execute('''COMMIT;''')
        cursor.execute('''DROP TABLE LevelFileList_old;''')
        cursor.execute('''
        BEGIN TRANSACTION;
                       ''')
        cursor.execute('''
        ALTER TABLE GameFileList RENAME TO GameFileList_old;
                       ''')
        cursor.execute('''
        CREATE TABLE GameFileList (
            fileID INTEGER NOT NULL,
            gameID INTEGER NOT NULL,
            PRIMARY KEY (fileID, gameID),
            FOREIGN KEY (fileID) REFERENCES File(FileID),
            FOREIGN KEY (gameID) REFERENCES Game(GameID)
        );''')

        cursor.execute('''INSERT INTO GameFileList (fileID, gameID)
        SELECT fileID, gameID FROM GameFileList_old;''')

        cursor.execute('''COMMIT;''')
        cursor.execute('''DROP TABLE GameFileList_old;''')
        cursor.execute('''BEGIN TRANSACTION;''')
        cursor.execute('''ALTER TABLE Level RENAME TO Level_old;''')
        cursor.execute('''CREATE TABLE Level (
            LevelID INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
            body TEXT NOT NULL,
            walkthrough  TEXT NOT NULL,
            infoID INTEGER NOT NULL,
            FOREIGN KEY (infoID) REFERENCES Info(InfoID)
        );''')

        cursor.execute('''INSERT INTO Level (LevelID, body, walkthrough, infoID)
        SELECT LevelID, body, walkthrough, infoID FROM Level_old;''')

        cursor.execute('''CREATE TABLE ZipList (
            zipID INTEGER NOT NULL,
            levelID INTEGER NOT NULL,
            PRIMARY KEY (zipID, levelID),
            FOREIGN KEY (zipID) REFERENCES Zip(ZipID),
            FOREIGN KEY (levelID) REFERENCES Level(LevelID)
        );''')

        cursor.execute('''INSERT INTO ZipList (zipID, levelID)
        SELECT Level.zipID, Level.InfoID
        FROM Level_old AS Level;''')
        cursor.execute('''COMMIT;''')
        cursor.execute('''DROP TABLE Level_old;''')

        cursor.execute('SELECT PictureID, data FROM Picture')
        pictures = cursor.fetchall()

        for picture_id, jpg_data in pictures:
            img = Image.open(BytesIO(jpg_data))

            webp_image = BytesIO()
            img.save(webp_image, format='WEBP')
            webp_image_data = webp_image.getvalue()

            try:
                cursor.execute('''
                    UPDATE Picture SET data = ? WHERE PictureID = ?
                ''', (webp_image_data, picture_id))

            except OSError as img_error:
                # Handle errors related to image processing
                print(f"Error processing image for PictureID {picture_id}: {img_error}")

        cursor.execute('SELECT fileID FROM LevelFileList')
        file_ids = cursor.fetchall()

        for (file_id,) in file_ids:
            try:
                cursor.execute('DELETE FROM File WHERE FileID = ?', (file_id,))

                cursor.execute('DELETE FROM LevelFileList WHERE fileID = ?', (file_id,))

            except sqlite3.DatabaseError as e:
                print(f"Error deleting fileID {file_id}: {e}")

    except sqlite3.DatabaseError as db_error:
        print(f"Error fetching data from database: {db_error}")

    finally:
        conn.commit()
        cursor.execute('VACUUM')
        conn.commit()
        conn.close()

if __name__ == "__main__":
    if len(sys.argv) != 2:
        logging.error("Usage: python3 updateDB_1.1.0.py path/to/tombll.db")
        sys.exit(1)
    else:
        arg_path = sys.argv[1]
        update_table_schema_and_data(arg_path)
