"""Terminal menus, prompts for data, tests and setting up the database."""

import os
import sqlite3

import tombll_manage_data

os.chdir(os.path.dirname(os.path.abspath(__file__)))


def compare_pages(estimate):
    """Get the range from database and server pages. Its a bit stupid but lets do this at first."""
    db_path = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'tombll.db')
    con = sqlite3.connect(db_path)
    database_page = tombll_manage_data.get_local_page(0, con)
    con.close()

    database_id = 1
    if database_page['levels']:
        database_id = database_page['levels'][0]['trle_id']

    trle_id = int(tombll_manage_data.get_trle_page(0)['levels'][0]['trle_id'])
    total_records = trle_id - database_id + 1
    about_time_sec = total_records * estimate

    hours, remainder = divmod(about_time_sec, 3600)
    minutes, seconds = divmod(remainder, 60)

    print(f"There are {total_records} level records (or fewer) to update.")
    print(f"Estimated time: {hours:02d}:{minutes:02d}:{seconds:02d}")

    return (database_id, trle_id)


def update_level_cards():
    """
    Update the database by getting the missing last card records.

    Get a local page from the database and compare it to a scraped page from trle.net.
    Then add all the level cards in the range of ID numbers from the last date ID in the database
    to the last date ID on trle.net.
    """
    trle_range = compare_pages(6)
    print("I will upload a database on MEGA to offload the server.")
    print("The program can't even handle that many records right now.")
    print("There is no reason for you to test this.")
    print("Do you want to continue?")
    awn = input("y or n: ")
    if awn == 'y':
        tombll_manage_data.add_level_card_range(trle_range[0], trle_range[1])


def update_levels():
    """
    Update the databse by getting the missing last level records.

    Get a local page from the database and then compare it by a scraped page form trle.net.
    Then add all the level in the range of id numbers from the last date id on the database
    to the last date id on trle.net.
    """
    '''
    trle_range = compare_pages(7)
    print("Do you want to continue?")
    awn = input("y or n: ")
    if awn == 'y':
        tombll_manage_data.add_level_range(trle_range[0], trle_range[1])
    '''


def scrape_trle_index():
    """Browse TRLE data by using normal https requests."""
    offset = 0
    while True:
        _print_trle_page(tombll_manage_data.get_trle_page(offset))
        user_input = input("Press Enter for the next page (or type 'q' to quit: ")
        if user_input.lower() == 'q':
            print("Exiting...")
            break
        offset += 20


def local_trle_index():
    """Browse local TRLE data."""
    con = sqlite3.connect(os.path.dirname(os.path.abspath(__file__)) + '/tombll.db')
    offset = 0
    while True:
        page = tombll_manage_data.get_local_page(offset, con)
        _print_trle_page(page)
        if len(page['levels']) < 20:
            break
        user_input = input("Press Enter for the next page (or type 'q' to quit: ")
        if user_input.lower() == 'q':
            print("Exiting...")
            break
        offset += 20
    con.close()


def _print_trle_page(page):
    print(f"{page['offset'] + len(page['levels'])} of {page['records_total']} records")
    levels = page['levels']

    # Column widths for even spacing
    column_widths = [6, 20, 70, 17, 11, 16, 6, 10]

    headers = ["ID", "Author", "Level Name", "Difficulty",
               "Duration", "Class", "Type", "Released"]
    # Print the headers
    for i, header in enumerate(headers):
        print(header.ljust(column_widths[i]), end="")
    print("")  # Move to the next line after printing headers

    # Print the level data
    for row in levels:
        cell_data = []  # Collect the cell data to print in one line
        for idx, k in enumerate(row.keys()):
            cell = str(row[k])  # Convert each cell value to string
            width = column_widths[idx]  # Get the correct column width
            truncated_text = cell[:width-1].ljust(width-1)  # Truncate and pad the text
            cell_data.append(truncated_text + ' ')
        print("".join(cell_data))  # Print the row in one line
    print("")
