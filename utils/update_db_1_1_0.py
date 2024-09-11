"""
Update database remove "NOT NULL" from difficulty, duration and fix a typo
"""
import sys
import logging
import sqlite3

def update_table_schema_and_data(db_path):
    """Update from 1.0.0 database to 1.1.0"""
    conn = sqlite3.connect(db_path)
    cursor = conn.cursor()



if __name__ == "__main__":
    if len(sys.argv) != 2:
        logging.error("Usage: python3 updateDB_1.1.0.py path/to/tombll.db")
        sys.exit(1)
    else:
        logging.error("updateDB_1.1.0.py not yet implemented")
        #arg_path = sys.argv[1]
        #update_table_schema_and_data(arg_path)
