"""Common tombll database calls."""
import sys
import sqlite3
import json
import logging


def get_tombll_json(path):
    """Load and parse a JSON file from a specified path.

    Attempts to open and read a JSON file, parsing its content into a dictionary.
    Exit on errors for file not found, JSON decoding issues, and other I/O problems.

    Args:
        path (str): The path to the JSON file.

    Returns:
        dict: Parsed content of the JSON file.

    Exits:
        Logs an error and exits the program if the file cannot be read or parsed.
    """
    try:
        # Open the file with UTF-8 encoding
        with open(path, mode='r', encoding='utf-8') as json_file:
            try:
                # Parse and return JSON content
                return json.load(json_file)
            except json.JSONDecodeError as json_error:
                # Log and exit if JSON content is invalid
                logging.error("Error decoding JSON from file '%s': %s", path, json_error)
                sys.exit(1)
    except FileNotFoundError:
        # Log and exit if file is not found
        logging.error("File not found: '%s'", path)
        sys.exit(1)
    except IOError as file_error:
        # Log and exit if any other I/O error occurs
        logging.error("I/O error occurred while opening file '%s': %s", path, file_error)
        sys.exit(1)


def query_return_everything(query, params, con):
    """
    Execute a SQL query using the provided connection and returns all resulting rows.

    This function is primarily intended for SELECT or other queries that produce a result set.
    It uses `fetchall()` to retrieve all results from the executed query.

    If a database error occurs, the error is logged, any open transaction is rolled back,
    and the program exits with an error status.

    Args:
        query (str): The SQL query to execute. Expected to be a SELECT or similar.
        params (tuple or None): A tuple of parameters to safely bind to the query.
        con (sqlite3.Connection): An open SQLite database connection.

    Returns:
        list: A list of rows resulting from the query. Each row is a tuple.
              Returns an empty list if no rows match.

    Exits:
        Logs an error and exits the program if a sqlite3.DatabaseError occurs.

    Example:
        result = query_return_everything("SELECT * FROM users WHERE age > ?", (18,), con)
    """
    cursor = con.cursor()

    try:
        if params:
            # Execute the query with the given parameters
            cursor.execute(query, params)
        else:
            # Execute the query without parameters
            cursor.execute(query)

        # Return all rows from the query result
        return cursor.fetchall()

    except sqlite3.DatabaseError as db_error:
        # Log the error, rollback any changes, and exit the program
        logging.error("Database error occurred during query execution: %s", db_error)
        con.rollback()
        sys.exit(1)


def query_return_id(query, params, con):
    """Execute a SQL query and return an ID.

    If the query is an INSERT, this function returns the last inserted row ID.
    For other queries, it fetches and returns the first integer result, if it exists and is
    non-negative.

    If a database error occurs, the error is logged, any open transaction is rolled back,
    and the program exits with an error status.

    Args:
        query (str): SQL query to execute.
        params (tuple or None): Parameters for the query.
        con (sqlite3.Connection): SQLite database connection.

    Returns:
        int or None: The ID from the query result, or None if not found.

    Exits:
        Logs an error and exits if a database error occurs.
    """
    cursor = con.cursor()
    try:
        if params:
            # Execute the query with the given parameters
            cursor.execute(query, params)
        else:
            # Execute the query without parameters
            cursor.execute(query)

        # Check if it's an INSERT query to return the last inserted row ID
        if query.strip().upper().startswith("INSERT"):
            return cursor.lastrowid

        # For non-INSERT queries, fetch and validate the first result
        result = cursor.fetchone()
        if result and isinstance(result[0], int) and result[0] >= 0:
            return result[0]

        return None  # Return None if no valid ID is found

    except sqlite3.DatabaseError as db_error:
        # Log the database error and exit
        logging.error("Database error occurred: %s", db_error)
        con.rollback()
        sys.exit(1)


def query_run(query, params, con):
    """Execute a SQL query with the provided parameters.

    If a database error occurs, the error is logged, any open transaction is rolled back,
    and the program exits with an error status.

    Args:
        query (str): The SQL query to execute.
        params (tuple or None): Parameters to substitute in the SQL query.
        con (sqlite3.Connection): SQLite database connection.

    Exits:
        Logs an error and exits if a database error occurs.
    """
    cursor = con.cursor()
    try:
        if params:
            # Execute the query with the given parameters
            cursor.execute(query, params)
        else:
            # Execute the query without parameters
            cursor.execute(query)

    except sqlite3.DatabaseError as db_error:
        # Log the database error and exit the program
        logging.error("Database error occurred: %s", db_error)
        con.rollback()
        sys.exit(1)


def make_empty_null(value):
    """Convert empty strings or zero float values to None.

    Args:
        value: The value to check, which can be a string, float, or other types.

    Returns:
        None if the value is an empty string or exactly 0.0; otherwise, returns the original value.
    """
    if value in ("", 0.0, 0):
        return None
    return value
