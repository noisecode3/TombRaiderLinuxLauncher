"""Manage data for the tombll database."""
import os
import sys
import re
import sqlite3
import json
import time
import logging

import scrape_trle
import data_factory

import tombll_common
import tombll_create
import tombll_delete
import tombll_read
import tombll_update

# Set up logging
logging.basicConfig(level=logging.DEBUG, format='%(asctime)s %(levelname)s:%(message)s')
logging.getLogger("requests").setLevel(logging.DEBUG)


def print_info():
    """Print help info."""
    help_text = """
Usage: python3 tombll_manage_data.py [options]
  Options:
      -l    List level records
      -a    [lid] Add a level record
      -sj   [lid path] Save a level record to json file
      -aj   [path] Add a level record from a json file
      -ac   [lid] Add a level card record without info and walkthrough
      -acr  [lid lid] Add a range of level card records
      -rm   [lid] Remove one level record
      -sc   Sync cards
      -u    [lid] Update a level record

      -ld   [lid] List download files records
      -ad   [lid Zip.name Zip.size Zip.md5sum]
                Add a local zip file to a level without a file
"""
    print(help_text.strip())


def list_levels():
    """Retrieve and prints a list of level information."""
    # Fetch all rows
    con = database_make_connection()
    results = tombll_read.database_level_list(con)
    con.close()

    # Iterate over the results and print each row
    for row in results:
        print(row)


def add_level(lid):
    """Add a level by taking the lid number."""
    data = data_factory.make_trle_tombll_data()

    soup = scrape_trle.scrape_common.get_soup(trle_level_url(lid))
    scrape_trle.get_trle_level(soup, data)

    if data['title']:
        con = database_make_connection()
        database_begin_write(con)
        add_tombll_json_to_database(data, con)
        database_commit_and_close(con)
        print(f"lid {lid} with title \"{data['title']}\" added successfully.")
    else:
        print(f"lid {lid} was an empty page.")


def save_json_level(lid, path):
    """Save a level to a json file on disk by taking the lid number."""
    data = data_factory.make_trle_tombll_data()

    soup = scrape_trle.scrape_common.get_soup(trle_level_url(lid))
    scrape_trle.get_trle_level(soup, data)

    if data['title']:
        with open(path, mode='w', encoding='utf-8') as json_file:
            json.dump(data, json_file)
        print(
            f"lid {lid} with title \"{data['title']}\" "
            f"saved to {path} successfully."
        )
    else:
        print(f"lid {lid} was an empty page.")


def add_file(path):
    """Add a level from a json file on disk."""
    data = tombll_common.get_tombll_json(path)

    con = database_make_connection()
    database_begin_write(con)
    add_tombll_json_to_database(data, con)
    database_commit_and_close(con)
    print(f"File {path} with title \"{data['title']}\" added successfully.")


def add_level_card(lid):
    """Add a level card by taking the lid number."""
    data = data_factory.make_trle_tombll_data()

    soup = scrape_trle.scrape_common.get_soup(trle_level_url(lid))
    scrape_trle.get_trle_level_card(soup, data)

    if data['title']:
        con = database_make_connection()
        database_begin_write(con)
        add_tombll_json_to_database(data, con)
        database_commit_and_close(con)
        print(f"lid {lid} card with title \"{data['title']}\" added successfully.")
    else:
        print(f"lid {lid} was an empty page.")


def add_level_card_range(range_a, range_b):
    """Add a level card by taking a lid range."""
    for i in range(min(int(range_a), int(range_b)), max(int(range_a), int(range_b)) + 1):
        data = data_factory.make_trle_tombll_data()
        soup = scrape_trle.scrape_common.get_soup(trle_level_url(i))
        scrape_trle.get_trle_level_card(soup, data)

        if data['title']:
            con = database_make_connection()
            database_begin_write(con)
            add_tombll_json_to_database(data, con)
            database_commit_and_close(con)
            print(f"lid {i} card with title \"{data['title']}\" added successfully.")
        else:
            print(f"lid {i} was an empty page.")

        time.sleep(5)


def remove_level(lid):
    """Remove a level from the database by taking the lid number."""
    con = database_make_connection()
    database_level_id = tombll_read.database_level_id(lid, con)

    database_begin_write(con)
    tombll_delete.database_level(database_level_id, con)
    database_commit_and_close(con)
    print(f"lid {lid} removed successfully.")


def update_level(lid):
    """Update a level by taking the lid number."""
    data = data_factory.make_trle_tombll_data()

    soup = scrape_trle.scrape_common.get_soup(trle_level_url(lid))
    scrape_trle.get_trle_level(soup, data)

    if data['title']:
        con = database_make_connection()
        level_id = tombll_read.database_level_id(lid, con)

        database_begin_write(con)
        update_tombll_json_to_database(data, level_id, con)
        database_commit_and_close(con)
        print(f"lid {lid} with title \"{data['title']}\" updated successfully.")
    else:
        print(f"lid {lid} was an empty page.")


def add_download(lid, name, size, md5):
    """Add a download file to a level by taking the lid number."""
    data = data_factory.make_zip_file()
    data["name"] = name
    data["size"] = size
    data["md5"] = md5
    data["url"] = None
    data["release"] = None
    data["version"] = None

    con = database_make_connection()
    database_begin_write(con)
    level_id = tombll_read.database_level_id(lid, con)
    tombll_create.database_zip_file(data, level_id, con)
    database_commit_and_close(con)


def list_downloads(lid):
    """Print a list of ZIP file entries associated with a specific level."""
    # Fetch all rows
    con = database_make_connection()
    level_id = tombll_read.database_level_id(lid, con)
    results = tombll_read.database_zip_list(level_id, con)
    con.close()

    # Print each row in the result
    for row in results:
        print(row)


class TailSync:
    """Match sync record rows and add new records or delete old."""

    def __init__(self):
        """Set default member variables."""
        self.local = []
        self.trle = []

        self.local_page_offset = 0
        self.trle_page_offset = 0

    def match_record(self, a, b):
        """trle.net record data matching."""
        keys = [
            'trle_id',
            #  'author',
            'title',
            'difficulty',
            'duration',
            'class',
            'type',
            'release'
        ]
        return all(a.get(k) == b.get(k) for k in keys)

    def match_tails(self, match_size=4, max_pages=20):
        """
        Find a tail match between local and trle databases based ids.

        This checks every level id in orderd steps aginst a batch of local ids.
        When it finds ONE of at least match_size*5 known local ids then it
        jumps forward that number of ids, but now there is a problem, trle.net
        can have deleted previous known level ids. So we have to go back
        a number of steps to match the tails. This is true becouse trle will
        not add records in between old records, only remove or update.
        The end of the tail is the point where some records needs to be chacked
        again, updated, deleted or added to the local database. We match aginst
        a batch of known ids not in a specific sequence but based on a jump,
        not order.

        match_size is the tail=n*5 and overshot=n*2+1
        tail is the number of most recent ids from the local databases
        overshot is the steps the alogorithm look backwards after the jump
        """

        def get_local_page_and_extend(offset):
            con = database_make_connection()
            page = get_local_page(offset, con)['levels']
            con.close()
            self.local.extend(page)
            return len(page)

        def get_trle_page_and_extend(offset):
            page = get_trle_page(offset)['levels']
            self.trle.extend(page)
            return len(page)

        def batch_ids_match(match_size, offset_trle):
            """Compare a batch of local trle ids to one online trle id."""
            for i in range(match_size*5):
                try:
                    local_id = int(self.local[i]['trle_id'])
                    trle_id = int(self.trle[offset_trle]['trle_id'])
                    if local_id == trle_id:
                        return True
                except IndexError:
                    print(f"IndexError at local[{i}] and trle[{offset_trle}]")
                    return False
            return False

        def overshot_ids_match(match_size, offset_trle):
            """Compare one to one backwards from local trle ids to one online trle id."""
            for i in range(match_size*2+1):
                for j in range(match_size*5):
                    try:
                        overshot_index = offset_trle + match_size*5 - i - 1
                        trle_id = int(self.trle[overshot_index]['trle_id'])
                        local_id = int(self.local[match_size*5 - j - 1]['trle_id'])
                        if local_id == trle_id:
                            print(f"Match found at local[{match_size*5 - j - 1}] and trle[{overshot_index}]")
                            return (match_size*5 - j - 1, overshot_index)
                    except IndexError:
                        print(f"IndexError at local[{match_size*5 - j}]"
                              " and trle[{offset_trle + match_size*5 - i}]")
                        return None
            return None
        # Load initial data
        get_local_page_and_extend(self.local_page_offset*20)

        # Load local pages if needed
        while match_size*5 > len(self.local):
            self.local_page_offset += 1
            print(f"Loading local page at offset {self.local_page_offset}")
            if self.local_page_offset >= max_pages or \
                    get_local_page_and_extend(self.local_page_offset*20) == 0:
                break

        get_trle_page_and_extend(self.trle_page_offset*20)

        i = 0
        while True:
            # If we run out of trle data, load more
            if match_size*5 + i > len(self.trle):
                self.trle_page_offset += 1
                print(f"Loading trle page at offset {self.trle_page_offset}")
                if self.trle_page_offset >= max_pages or \
                        get_trle_page_and_extend(self.trle_page_offset*20) == 0:
                    break

            if batch_ids_match(match_size, i):
                id_match = overshot_ids_match(match_size, i)
                if id_match is not None:
                    return id_match

            i += 1

        print("❌ No match found after paging.")
        sys.exit(1)

    def run(self):
        """
        Tail-sync in 3 simple stages.

        1. Update the last 5 matching records if their attributes differ.
        2. Add new records from remote page after the match.
        3. Update or delete unmatched local records after the match.
        """
        con = database_make_connection()

        def check_local_trle_id(trle_id):
            result = tombll_read.database_level_id(trle_id, con)
            if isinstance(result, int):
                return True
            return False

        def has_multivalued_field(trle_id):
            result = tombll_read.has_multivalued_field(trle_id, con)
            if isinstance(result, int):
                return result
            return 0

        def go_to_next_id(index, array):
            current_id = array[index]['trle_id']
            i = index

            while i < len(array) and array[i]['trle_id'] == current_id:
                i -= 1

            return i

        # get a tail of local records matching a point on trle records
        # from old known records to new, when local_start == 0 we have
        # only new records left.
        local_start, remote_start = self.match_tails()

        # update mismatched and remove missing records from trle to local
        i = local_start
        j = remote_start
        while 0 < i:
            try:
                local = self.local[i]
                remote = self.trle[j]
            except IndexError:
                print(f"IndexError self.local[{i}] self.trle[{j}]")
                sys.exit(1)

            if not self.match_record(local, remote):
                if local['trle_id'] == remote['trle_id']:
                    # some attributes was chagned we update
                    # this usally happens when a level is new
                    update_level(local['trle_id'])
                else:
                    # we asume the level was deleted but it might
                    # apear as a new level later (moved record)
                    remove_level(local['trle_id'])

            # we can hit a multivalued row usually its just different authors
            # but could be class also, but its rare. or we go just one record step
            i = go_to_next_id(i, self.local)
            j = go_to_next_id(j, self.trle)

        while 0 < j:
            j -= 1
            try:
                remote = self.trle[j]
            except IndexError:
                print(f"IndexError self.trle[{j}]")
                sys.exit(1)

            if check_local_trle_id(remote['trle_id']):
                update_level(remote['trle_id'])
            else:
                add_level_card(remote['trle_id'])

        con.close()


def sync_cards():
    """Lazy tail sync of cards."""
    tailsync = TailSync()
    tailsync.run()


def get_local_page(offset, con):
    """Pass down API to get a TRLE page from the database."""
    return tombll_read.trle_page(offset, con, sort_latest_first=True)


def get_trle_page(offset):
    """Pass down API to scrape a TRLE page from "trle.net"."""
    return scrape_trle.get_trle_page(offset, sort_created_first=True)


def database_make_connection():
    """
    Get a connection to the database.

    Returns:
        (sqlite3.Connection): An open SQLite database connection.

    """
    return sqlite3.connect(os.path.dirname(os.path.abspath(__file__)) + '/tombll.db', timeout=10)


def database_begin_write(con):
    """
    Run this before you call commit() when you write data as a starting point.

    If there is an data base error it will role back and exit the program.
    If it comes back sane you should run con.commit()

    Args:
        con (sqlite3.Connection): An open SQLite database connection.

    """
    cur = con.cursor()
    cur.execute("BEGIN;")


def database_commit_and_close(con):
    """
    Commit and close in one line or just run close if you only read data.

    Args:
        con (sqlite3.Connection): An open SQLite database connection.

    """
    con.commit()
    con.close()


def trle_level_url(lid):
    """
    Get level TRLE url from lid number.

    Args:
        con (sqlite3.Connection): An open SQLite database connection.
    """
    return f"https://www.trle.net/sc/levelfeatures.php?lid={lid}"


def parse_position(s):
    """Pares the letter to int from an jpg name."""
    match = re.match(r'^(\d+)([a-z]?)$', s)
    if not match:
        raise ValueError(f"Invalid format: {s}")
    suffix = match.group(2)
    return ord(suffix) - ord('a') + 1 if suffix else 0


def add_screen_from_url(screen, level_id, con):
    """
    Scrape the url picture and add to database.

    Returns:
        INT: Picture ID.

    """
    if screen.startswith("https://www.trle.net/screens/"):
        picture = data_factory.make_picture()
        # Fetch the .webp image data
        relative_path = screen.replace("https://www.trle.net/screens/", "")
        picture['data'] = scrape_trle.get_trle_cover(relative_path)
        picture['position'] = parse_position(scrape_trle.scrape_common.url_basename_prefix(screen))
        picture['md5sum'] = scrape_trle.scrape_common.calculate_data_md5(picture['data'])
        return tombll_create.database_screen(picture, level_id, con)

    print("Cover did not start with https://www.trle.net/screens/ ")
    sys.exit(1)


def add_tombll_json_to_database(data, con):
    """Insert level data and related details into the database.

    This function inserts a level record into the database and subsequently
    adds associated authors, genres, tags, zip files, screen, and large screens.

    Args:
        data (dict): A dictionary containing level data and related details.
        con (sqlite3.Connection): SQLite database connection.
    """
    # Insert level information and obtain the generated level ID
    level_id = tombll_create.database_level(data, con)

    # Add related information only if corresponding data is present
    if data.get('authors'):
        tombll_create.database_authors(data['authors'], level_id, con)
    if data.get('genres'):
        tombll_create.database_genres(data['genres'], level_id, con)
    if data.get('tags'):
        tombll_create.database_tags(data['tags'], level_id, con)
    if data.get('zip_files'):
        tombll_create.database_zip_files(data['zip_files'], level_id, con)

    # Single screen image, checked for None in the add_screen_to_database function
    add_screen_from_url(data.get('screen'), level_id, con)

    # Add large screens if they are provided
    large_screens = data.get('large_screens')
    if large_screens:
        for screen in large_screens:
            add_screen_from_url(screen, level_id, con)


def update_tombll_authors_to_database(authors, level_id, con):
    """Update authors data by adding and deleting.

    Args:
        authors (list of str): A list containing author names.
        level_id (int): Level.LevelID number.
        con (sqlite3.Connection): SQLite database connection.
    """
    database_authors = tombll_read.database_author_ids(level_id, con)
    new_set = set()
    for name in authors:
        author_id = tombll_create.database_author(name, level_id, con)
        new_set.add(author_id)

    if database_authors:
        current_set = set(a[0] for a in database_authors)
        to_remove = current_set - new_set
        for author_id in to_remove:
            tombll_delete.database_author(author_id, level_id, con)


def update_tombll_genres_to_database(genres, level_id, con):
    """Update genres data by adding and deleting.

    Args:
        genres (list of str): A list containing genres.
        level_id (int): Level.LevelID number.
        con (sqlite3.Connection): SQLite database connection.
    """
    database_genres = tombll_read.database_genre_ids(level_id, con)
    new_set = set()
    for genre in genres:
        genre_id = tombll_create.database_genre(genre, level_id, con)
        new_set.add(genre_id)

    if database_genres:
        current_set = set(a[0] for a in database_genres)
        to_remove = current_set - new_set
        for genre_id in to_remove:
            tombll_delete.database_genre(genre_id, level_id, con)


def update_tombll_tags_to_database(tags, level_id, con):
    """Update tags data by adding and deleting.

    Args:
        tags (list of str): A list containing tags.
        level_id (int): Level.LevelID number.
        con (sqlite3.Connection): SQLite database connection.
    """
    database_tags = tombll_read.database_tag_ids(level_id, con)
    new_set = set()
    for tag in tags:
        tag_id = tombll_create.database_tag(tag, level_id, con)
        new_set.add(tag_id)

    if database_tags:
        current_set = set(a[0] for a in database_tags)
        to_remove = current_set - new_set
        for tag_id in to_remove:
            tombll_delete.database_tag(tag_id, level_id, con)


def update_tombll_zip_files_to_database(zip_files, level_id, con):
    """Update zip_files data by adding and deleting.

    Args:
        zip_files (list of dict): A list containing zip_files.
        level_id (int): Level.LevelID number.
        con (sqlite3.Connection): SQLite database connection.
    """
    database_zip_files = tombll_read.database_zip_list(level_id, con)

    # Build sets for fast comparison
    current_set = set((z[0], z[3]) for z in database_zip_files)  # (name, url)
    new_set = set((z['name'], z['url']) for z in zip_files)

    # REMOVE
    to_remove = current_set - new_set
    for name, url in to_remove:
        for d in database_zip_files:
            if (d[0], d[3]) == (name, url):
                list = tombll_read.database_zip_id_list(level_id, name, url, con)
                for item in list:
                    tombll_delete.database_zip_file(item[0], level_id, con)
                break

    # ADD
    to_add = new_set - current_set
    for name, url in to_add:
        zip_file = next(z for z in zip_files if (z['name'], z['url']) == (name, url))
        tombll_create.database_zip_file(zip_file, level_id, con)


def update_tombll_json_to_database(data, level_id, con):
    """Update level data and related attributes into the database.

    This function update a level record into the database and subsequently
    updates associated authors, genres, tags, zip files, screen, and large screens.

    Args:
        data (dict): A dictionary containing level data and related details.
        level_id (int): Level.LevelID number.
        con (sqlite3.Connection): SQLite database connection.
    """
    info_id = tombll_update.database_level(data, level_id, con)
    tombll_update.database_info(data, info_id, con)

    authors = data.get('authors')
    if isinstance(authors, list):
        update_tombll_authors_to_database(authors, level_id, con)

    genres = data.get('genres')
    if isinstance(genres, list):
        update_tombll_genres_to_database(genres, level_id, con)

    tags = data.get('tags')
    if isinstance(tags, list):
        update_tombll_tags_to_database(tags, level_id, con)

    zip_files = data.get('zip_files')
    if zip_files:
        update_tombll_zip_files_to_database(zip_files, level_id, con)

    # Single screen image, checked for None in the add_screen_to_database function
    new_set = set()
    new_set.add(add_screen_from_url(data.get('screen'), level_id, con))

    # Add large screens if they are provided
    large_screens = data.get('large_screens')
    if large_screens:
        for screen in large_screens:
            new_set.add(add_screen_from_url(screen, level_id, con))

    database_pictures = tombll_read.database_picture_ids(level_id, con)
    if database_pictures:
        current_set = set(a[0] for a in database_pictures)
        to_remove = current_set - new_set
        for picture_id in to_remove:
            tombll_delete.database_picture(picture_id, level_id, con)


if __name__ == "__main__":
    number_of_argument = len(sys.argv)
    if (number_of_argument == 1 or number_of_argument >= 10):
        print_info()
        sys.exit(1)

    if (sys.argv[1] == "-h" and number_of_argument == 2):
        print_info()
        sys.exit(1)

    if (sys.argv[1] == "-l" and number_of_argument == 2):
        list_levels()

    elif (sys.argv[1] == "-a" and number_of_argument == 3):
        add_level(sys.argv[2])

    elif (sys.argv[1] == "-sj" and number_of_argument == 4):
        save_json_level(sys.argv[2], sys.argv[3])

    elif (sys.argv[1] == "-aj" and number_of_argument == 3):
        add_file(sys.argv[2])

    elif (sys.argv[1] == "-ac" and number_of_argument == 3):
        add_level_card(sys.argv[2])

    elif (sys.argv[1] == "-acr" and number_of_argument == 4):
        add_level_card_range(sys.argv[2], sys.argv[3])

    elif (sys.argv[1] == "-rm" and number_of_argument == 3):
        remove_level(sys.argv[2])

    elif (sys.argv[1] == "-u" and number_of_argument == 3):
        update_level(sys.argv[2])

    elif (sys.argv[1] == "-sc" and number_of_argument == 2):
        sync_cards()

    elif (sys.argv[1] == "-ld" and number_of_argument == 3):
        list_downloads(sys.argv[2])

    elif (sys.argv[1] == "-ad" and number_of_argument == 6):
        add_download(sys.argv[2], sys.argv[3], sys.argv[4], sys.argv[5])

    else:
        print_info()
