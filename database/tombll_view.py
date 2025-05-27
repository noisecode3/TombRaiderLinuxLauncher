"""terminal menus, prompts for data, tests and setting up the database."""

import os
import subprocess  # nosec
import json

import tempfile
import getpass

import sys
import shutil
import gc
import sqlite3

import scrape_trle
import tombll_read

gc.collect()
os.chdir(os.path.dirname(os.path.abspath(__file__)))


def scrape_trle_index():
    """Browse TRLE data by using normal https requests."""
    offset = 0
    while True:
        _print_trle_page(scrape_trle.get_trle_page(offset, True))
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
        page = tombll_read.trle_page(offset, con, sort_latest_first=True)
        _print_trle_page(page)
        if len(page['levels']) < 20:
            break
        user_input = input("Press Enter for the next page (or type 'q' to quit: ")
        if user_input.lower() == 'q':
            print("Exiting...")
            break
        offset += 20
    con.close()


def local_trle_pictures_index():
    """Browse local TRLE data with pictures."""
    con = sqlite3.connect(os.path.dirname(os.path.abspath(__file__)) + '/tombll.db')
    if _check_ueberzugpp():
        offset = 1
        while True:
            page = tombll_read.trle_page(offset, con, sort_latest_first=True)
            levels = page['levels']
            covers = []
            for level in levels:
                data = tombll_read.trle_cover_picture(level['trle_id'], con)
                covers.append(scrape_trle.scrape_common.cover_to_tempfile(data))

            _display_menu(levels, covers)
            for file in covers:
                try:
                    os.remove(file)
                except FileNotFoundError:
                    print(f"{file} not found, skipping.")
            offset += 1
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


def _check_ueberzugpp():
    if shutil.which("ueberzugpp") is None:
        print("ueberzugpp is not installed or not in PATH.")
        return False

    try:
        result = subprocess.run(  # nosec
            ["ueberzugpp", "--version"],
            capture_output=True,
            text=True,
            check=True
        )
        version_str = result.stdout.strip()
        print(f"Found ueberzugpp version: {version_str}")
        # Optionally parse and check version here
        return True

    except FileNotFoundError:
        print("ueberzugpp binary not found.")
        return False

    except subprocess.CalledProcessError as e:
        print(f"ueberzugpp returned a non-zero exit code: {e}")
        return False

    except OSError as e:
        print(f"OS error occurred while running ueberzugpp: {e}")
        return False


# Function to display the menu for multiple items, with pictures in the terminal
# This works with Xterm/UXterm and Alacritty
# Tested with xfce4-terminal, Terminator and other with fancy tabs and menus wont work
# Developed with on x11 and python3 ueberzug with pip from
# https://github.com/ueber-devel/ueberzug/
# xterm is supported 100% with TrueType
# uxterm -fa 'TrueType' -fs 12
# will calculate "space" correctly for all font sizes -fs N
# Alacritty is 100% working with fonts that have the same spacing as

# [font]
# size = 15.0

# [font.bold]
# family = "Hack"
# style = "Bold"

# [font.bold_italic]
# family = "Hack"
# style = "Bold Italic"

# [font.italic]
# family = "Hack"
# style = "Italic"

# [font.normal]
# family = "Hack"
# style = "Regular"

def _display_menu(items, image_paths):
    """Display a list of items with images next to them."""
    supported_terminals = ['alacritty', 'xterm']
    term = os.getenv('TERM', '').lower()
    identifiers = []
    if len(items) != len(image_paths):
        print("Fucked up in previous functions.")
        sys.exit(1)
    if term not in supported_terminals:
        print("Terminal not supported.")
        sys.exit(1)
    if not shutil.which('ueberzug'):
        print("ueberzug not found in $PATH.")
        sys.exit(1)
    print("\033c", end="")
    print("")

    terminal_size = shutil.get_terminal_size()
    max_rows = (terminal_size.lines - 2) // 7
    max_columns = terminal_size.columns // 79

    if max_columns < 1:
        print("Screen width too small")
        sys.exit(1)

    proc = subprocess.Popen(  # nosec
        ["ueberzugpp", "layer", "-o", "wayland"],
        stdin=subprocess.PIPE,
        text=True
    )

    if proc is None:
        sys.exit(1)

    full_board = min(len(items), max_rows * max_columns)
    display_items = items[:full_board]
    remaining_items = items[full_board:]
    display_image_paths = image_paths[:full_board]
    remaining_image_paths = image_paths[full_board:]
    for i in range(len(display_items)):
        if i == len(identifiers):
            command = {
                "action": "add",
                "identifier": f"cover_{i}",
                "x": (i % max_columns) * 79 + 2,
                "y": (i // max_columns) * 7 + 1,
                "path": display_image_paths[i],
                "scaler": "stretch",
                "width": 16,
                "height": 6
            }
            proc.stdin.write(json.dumps(command) + '\n')
            proc.stdin.flush()
            identifiers.append(f"cover_{i}")

        # Calculate full rows and handle the last row separately
        full_rows = len(display_items) // max_columns
        last_row_items = len(display_items) % max_columns

        # Display full rows
        for row in range(full_rows):
            row_offset = row * max_columns
            _print_row(display_items, row_offset, max_columns)

        # Display last row if there are any remaining items
        if last_row_items > 0:
            row_offset = full_rows * max_columns
            _print_row(display_items, row_offset, last_row_items)

        awn = input(f"  {len(display_items)} of {len(items)} results, " +
                    "Press 'q' and Enter to exit, else press Enter for next page...")

        print("\033c", end="")
        if awn == 'q':
            # TODO clean upp files in tmp
            sys.exit(0)

        # Remove all previous images
        for identifier in identifiers:
            command = {
                "action": "remove",
                "identifier": identifier,
            }
            proc.stdin.write(json.dumps(command) + '\n')
            proc.stdin.flush()

        # Recursive call with remaining items
        if remaining_items:
            _display_menu(remaining_items, remaining_image_paths)


def _print_row(items, row_offset, columns):
    """Print a single row."""
    # Title row
    for column in range(columns):
        print(f"{' '*19}{items[row_offset + column]['title'][:60]:<60}", end="")
    print("")

    # Duration and difficulty row
    for column in range(columns):
        field = (
            f"{' '*19}Duration: {items[row_offset + column]['duration']}"
            f" Difficulty: {items[row_offset + column]['difficulty']}"
        )
        print(field[:79].ljust(79), end="")
    print("")

    # Release date and type row
    for column in range(columns):
        field = (
            f"{' '*19}Release Date: {items[row_offset + column]['release']}"
            f" Type: {items[row_offset + column]['type']}"
        )
        print(field[:79].ljust(79), end="")
    print("")

    for column in range(columns):
        print(
            f"{' '*19}Author: " +
            f"{', '.join(map(str, items[row_offset + column]['authors']))[:52]:<52}",
            end=""
        )
    print("")

    for column in range(columns):
        print(
            f"{' '*19}Genre: " +
            f"{', '.join(map(str, items[row_offset + column]['genres']))[:53]:<53}",
            end=""
        )
    print("")

    for column in range(columns):
        print(
            f"{' '*19}Tag: " +
            f"{', '.join(map(str, items[row_offset + column]['tags']))[:55]:<55}",
            end=""
        )
    print("\n")
