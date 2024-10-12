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
import os
import sqlite3

os.chdir(os.path.dirname(os.path.abspath(__file__)))


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
        id INTEGER PRIMARY KEY CHECK (id = 1),
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


if __name__ == '__main__':
    make_index_database()
    add_static_data()
