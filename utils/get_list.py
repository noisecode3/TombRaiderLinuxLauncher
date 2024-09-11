import sqlite3
import os

os.chdir(os.path.dirname(os.path.abspath(__file__)))


CONNECTION = sqlite3.connect('tombll.db')
C = CONNECTION.cursor()

C.execute('''
    SELECT Level.*, Info.*, Picture.*, Author.* 
    FROM Level 
    JOIN Info ON Level.infoID = Info.InfoID 
    JOIN Screens ON Level.LevelID = Screens.levelID 
    JOIN AuthorList ON Level.LevelID = AuthorList.levelID 
    JOIN Picture ON Screens.pictureID = Picture.PictureID 
    JOIN Author ON AuthorList.authorID = Author.AuthorID 
    WHERE Level.LevelID = 1
    GROUP BY Level.LevelID 
    ORDER BY MIN(Picture.PictureID) ASC
''')

# Fetch all rows from the executed query
results = C.fetchall()

# Iterate over the results and print each row
for row in results:
    print(row)

CONNECTION.commit()
CONNECTION.close()
