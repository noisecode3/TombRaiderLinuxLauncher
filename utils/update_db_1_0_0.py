import sys
import logging
import sqlite3

if __name__ == "__main__":
    if len(sys.argv) != 2:
        logging.error("Usage: python3 updateDB_1.0.0.py path/to/tombll.db")
        sys.exit(1)
    else:
        arg_path = sys.argv[1]

def update_table_schema_and_data(db_path):
    conn = sqlite3.connect(db_path)
    cursor = conn.cursor()

    # Begin a transaction
    cursor.execute('BEGIN TRANSACTION;')

    try:
        # Step 1: Rename the existing Info table
        cursor.execute('ALTER TABLE Info RENAME TO Info_old;')

        # Step 2: Create the new Info table with the updated schema
        cursor.execute('''
            CREATE TABLE Info (
                InfoID INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
                title TEXT NOT NULL,
                author TEXT NOT NULL,
                release DATE NOT NULL,
                difficulty INT,
                duration INT,
                type INT NOT NULL,
                class INT NOT NULL,
                FOREIGN KEY (difficulty) REFERENCES InfoDifficulty(InfoDifficultyID),
                FOREIGN KEY (duration) REFERENCES InfoDuration(InfoDurationID),
                FOREIGN KEY (type) REFERENCES InfoType(InfoTypeID),
                FOREIGN KEY (class) REFERENCES InfoClass(InfoClassID)
            );
        ''')

        # Step 3: Copy data from the old Info table to the new Info table
        cursor.execute('''
            INSERT INTO Info (InfoID, title, author, release, difficulty, duration, type, class)
            SELECT InfoID, title, author, release, difficulty, duration, type, class FROM Info_old;
        ''')

        # Step 4: Drop the old Info table
        cursor.execute('DROP TABLE Info_old;')

        # Step 5: Update the value in the InfoClass table
        cursor.execute('''
            UPDATE InfoClass
            SET value = 'Alien/Space'
            WHERE value = 'alien/space';
        ''')

        # Commit the transaction
        conn.commit()

    except Exception as e:
        # If there is any error, rollback the transaction
        conn.rollback()
        print(f"An error occurred: {e}")

    finally:
        # Close the connection
        conn.close()

# Usage
update_table_schema_and_data(arg_path)

