
import os
import sys
import sqlite3

def print_info():
    print("Usage: python3 tombll_manage_data.py [options]")
    print("  Options:")
    print("      -l List database records")
    print("      -r [Level.LevelID] Remove one level")

def print_list(con):
    cur = con.cursor()
    cur.execute('''
        SELECT Level.LevelID, Info.Title, Author.value
        FROM Level
        JOIN Info ON Level.infoID = Info.InfoID
        JOIN AuthorList ON Level.LevelID = AuthorList.levelID
        JOIN Author ON AuthorList.authorID = Author.AuthorID
        GROUP BY Level.LevelID
    ''')

    # Fetch all rows from the executed query
    results = cur.fetchall()

    # Iterate over the results and print each row
    for row in results:
        print(row)

def remove_level(level_id, con):
    cur = con.cursor()
    try:
        # Start transaction
        cur.execute("BEGIN;")

        cur.execute('''
            DELETE FROM Picture
            WHERE PictureID IN (
                SELECT pictureID
                FROM Screens
                GROUP BY pictureID
                HAVING COUNT(DISTINCT levelID) = 1 AND MAX(levelID) = ?
            );
        ''', (level_id,))

        cur.execute('DELETE FROM Screens WHERE levelID = ?;', (level_id,))

        cur.execute('''
            DELETE FROM Zip
            WHERE ZipID IN (
                SELECT zipID
                FROM ZipList
                GROUP BY zipID
                HAVING COUNT(DISTINCT levelID) = 1 AND MAX(levelID) = ?
            );
        ''', (level_id,))

        cur.execute('DELETE FROM ZipList WHERE levelID = ?;', (level_id,))

        cur.execute('''
            DELETE FROM Tag
            WHERE TagID IN (
                SELECT tagID
                FROM TagList
                GROUP BY tagID
                HAVING COUNT(DISTINCT levelID) = 1 AND MAX(levelID) = ?
            );
        ''', (level_id,))

        cur.execute('DELETE FROM TagList WHERE levelID = ?;', (level_id,))

        cur.execute('''
            DELETE FROM Genre
            WHERE genreID IN (
                SELECT genreID
                FROM GenreList
                GROUP BY genreID
                HAVING COUNT(DISTINCT levelID) = 1 AND MAX(levelID) = ?
            );
        ''', (level_id,))

        cur.execute('DELETE FROM GenreList WHERE levelID = ?;', (level_id,))

        # Delete authors only used by this level, if its the last one
        cur.execute('''
            DELETE FROM Author
            WHERE AuthorID IN (
                SELECT authorID
                FROM AuthorList
                GROUP BY authorID
                HAVING COUNT(DISTINCT levelID) = 1 AND MAX(levelID) = ?
            );
        ''', (level_id,))

        # Delete from AuthorList middle table
        cur.execute('DELETE FROM AuthorList WHERE levelID = ?;', (level_id,))

        cur.execute('''
            DELETE FROM Info
            WHERE InfoID IN (
                SELECT infoID
                FROM Level WHERE LevelID = ?
            );
        ''', (level_id,))

        cur.execute('DELETE FROM Level WHERE LevelID = ?;', (level_id,))

        # Commit if all went well
        con.commit()
        print(f"Level {level_id} and related authors removed successfully.")

    except Exception as any_error:
        # Rollback in case of any error
        con.rollback()
        print(f"Error while deleting level {level_id}: {any_error}")

    finally:
        pass

if __name__ == "__main__":
    number_of_argument = len(sys.argv)
    if (number_of_argument == 1 or number_of_argument >= 4):
        print_info()
        sys.exit(1)
    CON = sqlite3.connect(os.path.dirname(os.path.abspath(__file__)) + '/tombll.db')
    if (sys.argv[1] == "-h" and number_of_argument == 2):
        print_info()
    elif (sys.argv[1] == "-l" and number_of_argument == 2):
        print_list(CON)
    elif (sys.argv[1] == "-r" and number_of_argument == 3):
        remove_level(sys.argv[2], CON)
    else:
        print_info()
    CON.close()
