"""terminal menus, prompts for data, tests and setting up the database."""

import os
import sqlite3

import tombll_manage_data

os.chdir(os.path.dirname(os.path.abspath(__file__)))


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
