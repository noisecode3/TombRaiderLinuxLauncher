import sys
import logging
import sqlite3

def update_table_schema_and_data(db_path):
    """Update from 1.1.0 database to 1.1.1"""
    conn = sqlite3.connect(db_path)
    cursor = conn.cursor()

    try:
        #cursor.execute('BEGIN TRANSACTION;')


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
