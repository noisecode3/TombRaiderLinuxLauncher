"""
Make database file
"""
import sqlite3
import os

script_dir = os.path.dirname(os.path.abspath(__file__))
os.chdir(script_dir)

conn = sqlite3.connect('tombll.db')
c = conn.cursor()

c.execute('''
CREATE TABLE Info (
    InfoID INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
    title TEXT NOT NULL,
    author TEXT NOT NULL,
    release DATE NOT NULL,
    difficulty TEXT NOT NULL,
    duration TEXT NOT NULL,
    type TEXT NOT NULL,
    class TEXT NOT NULL
)''')

c.execute('''
CREATE TABLE Zip (
    ZipID INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
    name TEXT NOT NULL,
    size FLOAT NOT NULL,
    md5sum TEXT UNIQUE NOT NULL
)''')

c.execute('''
CREATE TABLE Files (
    FileID INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
    md5sum TEXT NOT NULL,
    path TEXT NOT NULL,
    url TEXT
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
    name TEXT NOT NULL,
    data BLOB NOT NULL
)''')

c.execute('''
CREATE TABLE Game (
    GameID INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
    name TEXT NOT NULL,
    pictureID INTEGER NOT NULL,
    FOREIGN KEY (pictureID) REFERENCES Picture(PictureID)
)''')

c.execute('''
CREATE TABLE Level (
    LevelID INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
    screenName TEXT,
    screenLargeNames TEXT,
    body TEXT NOT NULL,
    walkthrough  TEXT NOT NULL,
    pictureID INTEGER NOT NULL,
    zipID INTEGER NOT NULL,
    infoID INTEGER NOT NULL,
    FOREIGN KEY (pictureID) REFERENCES  Picture(PictureID),
    FOREIGN KEY (zipID) REFERENCES Zip(ZipID),
    FOREIGN KEY (infoID) REFERENCES Info(InfoID)
)''')

conn.commit()
conn.close()
