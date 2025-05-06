"""
All Insert queries.

Only pure queries and sometimes json data here.
If we can access the json array directly or use all of the data
equal to the make_trle_tombll_data() from the data_factory to access data, we do it here.
"""
import logging
import tombll_common


def database_author(author, level_id, con):
    """Add a author to the database, linking with a specific level.

    This function checks if a author already exists in the database.
    If it does it will use this ID number.
    If not, it inserts the author and creates an entry in the AuthorList table
    to link the author to the specified level.

    Args:
        author (str): Author names.
        level_id (int): The ID of the level to link author with.
        con (sqlite3.Connection): SQLite database connection.
    """
    # SQL queries for selecting, inserting, and linking author
    query_select_id = "SELECT AuthorID FROM Author WHERE value = ?"
    query_insert = "INSERT INTO Author (value) VALUES (?)"
    query_insert_middle = "INSERT INTO AuthorList (authorID, levelID) VALUES (?, ?)"

    # Try to get the existing author ID; if none, insert a new author and get its ID
    author_id = tombll_common.query_return_id(query_select_id, (author,), con)
    if author_id is None:
        author_id = tombll_common.query_return_id(query_insert, (author,), con)

    # Link the author with the level in AuthorList table
    tombll_common.query_run(query_insert_middle, (author_id, level_id), con)


def database_authors(authors_array, level_id, con):
    """Add authors array to the database, linking them with a specific level.

    Args:
        authors_array (list of str): List of author names.
        level_id (int): The ID of the level to link authors with.
        con (sqlite3.Connection): SQLite database connection.
    """
    for author in authors_array:
        database_author(author, level_id, con)


def database_genre(genre, level_id, con):
    """Add a genre to the database, linking them with a specific level.

    This function checks if the genre already exists in the database.
    If it does it will use this ID number.
    If not, it inserts the genre and creates an entry in the GenreList table
    to link the genre to the specified level.

    Args:
        genre (str): Genre names.
        level_id (int): The ID of the level to link genre with.
        con (sqlite3.Connection): SQLite database connection.
    """
    # SQL queries for selecting, inserting, and linking genre
    query_select_id = "SELECT GenreID FROM Genre WHERE value = ?"
    query_insert = "INSERT INTO Genre (value) VALUES (?)"
    query_insert_middle = "INSERT INTO GenreList (genreID, levelID) VALUES (?, ?)"

    # Try to get the existing genre ID; if none, insert a new genre and get its ID
    genre_id = tombll_common.query_return_id(query_select_id, (genre,), con)
    if genre_id is None:
        genre_id = tombll_common.query_return_id(query_insert, (genre,), con)

    # Link the genre with the level in GenreList table
    tombll_common.query_run(query_insert_middle, (genre_id, level_id), con)


def database_genres(genres_array, level_id, con):
    """Add genres to the database, linking them with a specific level.

    Args:
        genres_array (list of str): List of genre names.
        level_id (int): The ID of the level to link genres with.
        con (sqlite3.Connection): SQLite database connection.
    """
    for genre in genres_array:
        database_genre(genre, level_id, con)


def database_tag(tag, level_id, con):
    """Add a tag to the database, linking them with a specific level.

    This function checks if the tag already exists in the database.
    If it does it will use this ID number.
    If not, it inserts the tag and creates an entry in the TagList table
    to link the tag to the specified level.

    Args:
        tags_array (list of str): List of tag names.
        level_id (int): The ID of the level to link tags with.
        con (sqlite3.Connection): SQLite database connection.
    """
    # SQL queries for selecting, inserting, and linking tags
    query_select_id = "SELECT TagID FROM Tag WHERE value = ?"
    query_insert = "INSERT INTO Tag (value) VALUES (?)"
    query_insert_middle = "INSERT INTO TagList (tagID, levelID) VALUES (?, ?)"

    # Try to get the existing tag ID; if not found, insert a new tag and get its ID
    tag_id = tombll_common.query_return_id(query_select_id, (tag,), con)
    if tag_id is None:
        tag_id = tombll_common.query_return_id(query_insert, (tag,), con)

    # Link the tag with the level in TagList table
    tombll_common.query_run(query_insert_middle, (tag_id, level_id), con)


def database_tags(tags_array, level_id, con):
    """Add tags to the database, linking them with a specific level.

    Args:
        tags_array (list of str): List of tag names.
        level_id (int): The ID of the level to link tags with.
        con (sqlite3.Connection): SQLite database connection.
    """
    for tag in tags_array:
        database_tag(tag, level_id, con)


def database_zip_file(zip_file, level_id, con):
    """Add a ZIP file to the database, linking them with a specific level.

    This function inserts a ZIP file into the Zip table if the file's attributes
    are provided, setting any empty or missing values to `None`. It then links
    the inserted ZIP file to the specified level in the ZipList table.

    Args:
        zip_files (dict): List of ZIP file details, represented as a dictionary
        with keys 'name', 'size', 'md5', 'url', 'version', 'release'.
        level_id (int): The ID of the level to link ZIP files with.
        con (sqlite3.Connection): SQLite database connection.
    """
    # SQL queries for inserting ZIP file data and linking ZIP files to a level
    query_insert = '''
        INSERT INTO Zip (name, size, md5sum, url, version, release)
        VALUES (?, ?, ?, ?, ?, ?)
    '''

    # Prepare arguments for inserting the ZIP file, converting empty values to None
    insert_args = (
        tombll_common.make_empty_null(zip_file.get('name')),
        tombll_common.make_empty_null(zip_file.get('size')),
        tombll_common.make_empty_null(zip_file.get('md5')),
        tombll_common.make_empty_null(zip_file.get('url')),
        tombll_common.make_empty_null(zip_file.get('version')),
        tombll_common.make_empty_null(zip_file.get('release'))
    )

    # Insert the ZIP file and get its ID
    zip_id = tombll_common.query_return_id(query_insert, insert_args, con)

    # Link the ZIP file to the level in ZipList table
    query_insert_middle = "INSERT INTO ZipList (zipID, levelID) VALUES (?, ?)"
    middle_args = (zip_id, level_id)
    tombll_common.query_run(query_insert_middle, middle_args, con)


def database_zip_files(zip_files_array, level_id, con):
    """Add ZIP files to the database, linking them with a specific level.

    Args:
        zip_files_array (list of dict): List of ZIP file details, each represented as a dictionary
            with keys 'name', 'size', 'md5', 'url', 'version', 'release'.
        level_id (int): The ID of the level to link ZIP files with.
        con (sqlite3.Connection): SQLite database connection.
    """
    for zip_file in zip_files_array:
        database_zip_file(zip_file, level_id, con)


def database_screen(webp_image_data, level_id, con):
    """Add a screen image to the database and link it to a specific level.

    This function takes the corresponding image as a .webp file, and inserts
    it into the Picture table. It also creates an association with the specified
    level in the Screens table.

    Args:
        webp_image_data (byte): URL of the screen image.
        level_id (int): The ID of the level to link the screen image with.
        con (sqlite3.Connection): SQLite database connection.
    """
    # Insert the .webp image data into the Picture table and retrieve its ID
    query_insert_picture = "INSERT INTO Picture (data) VALUES (?)"
    picture_id = tombll_common.query_return_id(query_insert_picture, (webp_image_data,), con)

    # Link the inserted picture to the specified level in the Screens table
    query_insert_screen = "INSERT INTO Screens (pictureID, levelID) VALUES (?, ?)"
    tombll_common.query_run(query_insert_screen, (picture_id, level_id), con)


def database_screens(webp_image_data_array, level_id, con):
    """Add multiple screen images to the database and associate them with a specific level.

    This function iterates over an array of large screens, adding each to the database
    and linking it to the specified level using the `create_screen_to_database` helper function.

    Args:
        large_screens_array (list): List of byte of screen images.
        level_id (int): The ID of the level to associate the screen images with.
        con (sqlite3.Connection): SQLite database connection.
    """
    # Iterate over each screen in the provided array and add it to the database
    for screen in webp_image_data_array:
        database_screen(screen, level_id, con)


def database_level(data, con):
    """Insert a level entry into the Level table and return its ID.

    This function inserts a new level into the database, using information provided in the
    `data` dictionary. It retrieves or creates an associated `infoID` using the
    `add_info_to_database` helper function, then logs and returns the `level_id` of
    the inserted level.

    Args:
        data (dict): A dictionary containing level data, including 'body' and 'walkthrough' fields.
        con (sqlite3.Connection): SQLite database connection.

    Returns:
        int: The ID of the newly inserted level.
    """
    # SQL query to insert a new level record
    query = "INSERT INTO Level (body, walkthrough, infoID) VALUES (?, ?, ?)"

    # Prepare arguments for the insertion, including the infoID from `add_info_to_database`
    arg = (
        data.get('body'),                   # Level body content
        data.get('walkthrough'),            # Level walkthrough content
        database_info(data, con)  # Retrieve and create infoID
    )

    # Execute the query and get the ID of the inserted level
    level_id = tombll_common.query_return_id(query, arg, con)

    # Log the current level ID for debugging or tracking purposes
    logging.info("Current tombll level_id: %s", level_id)

    return level_id


def database_info(data, con):
    """Insert or retrieve IDs for level information attributes and add an Info record.

    This function retrieves or sets IDs for various level attributes (difficulty, duration,
    type, class) and uses them to insert a new record into the Info table.

    Args:
        data (dict): A dictionary containing level information attributes.
        con (sqlite3.Connection): SQLite database connection.

    Returns:
        int: The ID of the newly inserted or existing Info record.
    """
    # Retrieve or assign InfoDifficultyID, or set to None if not specified
    info_difficulty = data.get('difficulty')
    info_difficulty_id = None
    if info_difficulty:
        query = "SELECT InfoDifficultyID FROM InfoDifficulty WHERE value = ?"
        info_difficulty_id = tombll_common.query_return_id(query, (info_difficulty,), con)

    # Retrieve or assign InfoDurationID, or set to None if not specified
    info_duration = data.get('duration')
    info_duration_id = None
    if info_duration:
        query = "SELECT InfoDurationID FROM InfoDuration WHERE value = ?"
        info_duration_id = tombll_common.query_return_id(query, (info_duration,), con)

    # Retrieve or assign InfoTypeID, allowing None if not specified
    info_type = data.get('type') or None
    query = "SELECT InfoTypeID FROM InfoType WHERE value = ?"
    info_type_id = tombll_common.query_return_id(query, (info_type,), con)

    # Retrieve or assign InfoClassID, allowing None if not specified
    info_class = data.get('class') or None
    query = "SELECT InfoClassID FROM InfoClass WHERE value = ?"
    info_class_id = tombll_common.query_return_id(query, (info_class,), con)

    # Insert a new Info record with the retrieved or default IDs
    query = '''
        INSERT INTO Info (title, release, difficulty, duration, type, class, trleID, trcustomsID)
        VALUES (?, ?, ?, ?, ?, ?, ?, ?)
    '''
    arg = (
        data.get('title'),      # Title of the level
        data.get('release'),    # Release date
        info_difficulty_id,     # Retrieved or NULL difficulty ID
        info_duration_id,       # Retrieved or NULL duration ID
        info_type_id,           # Retrieved or NULL type ID
        info_class_id,          # Retrieved or NULL class ID
        tombll_common.make_empty_null(data.get('trle_id')),        # TRLE ID if available
        tombll_common.make_empty_null(data.get('trcustoms_id'))    # TRCustoms ID if available
    )

    return tombll_common.query_return_id(query, arg, con)
