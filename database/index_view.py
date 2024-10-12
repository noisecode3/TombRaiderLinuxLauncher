"""terminal menus, prompts for data, tests and setting up the database"""
import os
import sys
import shutil
import gc
import ueberzug as ueberzug_root
import ueberzug.lib.v0 as ueberzug

gc.collect()
os.chdir(os.path.dirname(os.path.abspath(__file__)))


def print_trle_page(page):
    print(f"{page['offset'] + 20} of {page['records_total']} records")
    levels = page['levels']

    # Column widths for even spacing
    column_widths = [20, 20, 70, 20, 15, 15, 10, 20]

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
            truncated_text = cell[:width].ljust(width)  # Truncate and pad the text
            cell_data.append(truncated_text)
        print("".join(cell_data))  # Print the row in one line


def print_trcustoms_page(page):
    for level in page['levels']:
        print(f"\nid: {level['trcustoms_id']} tile: {level['title']}")
        print(f"authors: {level['authors']}")
        print(f"tags: {level['tags']}")
        print(f"created: {level['release']}")
        print(f"duration: {level['duration']} difficulty: {level['difficulty']}")
        print(f"type: {level['type']}")
        print(f"genres: {level['genres']}")
        print(f"picture_url: {level['cover']}")
        print(f"picture_md5sum: {level['cover_md5sum']}")

    print(f"\nPage number:{page['current_page']} of {page['total_pages']}")
    print(f"Total records: {page['records_total']}")


def check_ueberzug():
    try:
        version_str = ueberzug_root.__version__
        version_parts = [int(v) for v in version_str.split('.')]  # Split and convert to integers
        # Check if version is at least 18.2.3
        if version_parts < [18, 2, 3]:
            print("Your version of ueberzug is too old.")
            print("Please upgrade by running the following:")
            print("pip install git+https://github.com/ueber-devel/ueberzug.git")
            print("")
            print("ueberzug is packaged on most popular distros debian, arch, fedora, gentoo")
            print("and void, please request the package maintainer of your favourite distro to")
            print("package the latest release of ueberzug.")
            return False
        return True
    except AttributeError:
        print("Could not determine the version of ueberzug.")
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

def display_menu(items, image_paths):
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

    with ueberzug.Canvas() as canvas:
        full_board = min(len(items), max_rows * max_columns)
        display_items = items[:full_board]
        remaining_items = items[full_board:]
        display_image_paths = image_paths[:full_board]
        remaining_image_paths = image_paths[full_board:]

        for i in range(len(display_items)):
            if i == len(identifiers):
                cover_canvas = canvas.create_placement(
                    f'cover_{i}',
                    x=(i % max_columns) * 79 + 2,
                    y=(i // max_columns) * 7 + 1,
                    scaler=ueberzug.ScalerOption.COVER.value,
                    width=16, height=6
                )
                cover_canvas.path = display_image_paths[i]
                cover_canvas.visibility = ueberzug.Visibility.VISIBLE
                identifiers.append(cover_canvas)

        # Calculate full rows and handle the last row separately
        full_rows = len(display_items) // max_columns
        last_row_items = len(display_items) % max_columns

        # Display full rows
        for row in range(full_rows):
            row_offset = row * max_columns
            print_row(display_items, row_offset, max_columns)

        # Display last row if there are any remaining items
        if last_row_items > 0:
            row_offset = full_rows * max_columns
            print_row(display_items, row_offset, last_row_items)

        awn = input(f"  {len(display_items)} of {len(items)} results, " +\
                "Press 'q' and Enter to exit, else press Enter for next page...")

        print("\033c", end="")
        if awn == 'q':
            #TODO clean upp files in tmp
            sys.exit(0)

        # Remove all previous images
        for identifier in identifiers:
            # Create the remove command
            identifier.visibility = ueberzug.Visibility.INVISIBLE

        # Recursive call with remaining items
        if remaining_items:
            display_menu(remaining_items, remaining_image_paths)


def print_row(items, row_offset, columns):
    """Helper function to print a single row."""
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
        print( \
            f"{' '*19}Author: " +\
            f"{', '.join(map(str, items[row_offset + column]['authors']))[:52]:<52}",
            end=""
        )
    print("")

    for column in range(columns):
        print( \
            f"{' '*19}Genre: " +\
            f"{', '.join(map(str, items[row_offset + column]['genres']))[:53]:<53}",
            end=""
        )
    print("")

    for column in range(columns):
        print(
            f"{' '*19}Tag: " +\
            f"{', '.join(map(str, items[row_offset + column]['tags']))[:55]:<55}",
            end=""
        )
    print("\n")


