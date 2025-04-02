"""Main loop for CLI menu like interfaces"""
import sys
import os
import time
import index_view
import scrape_trle
import index_query
import make_index_database


def test_trle():
    """Browse TRLE data by using normal https requests."""
    index_view.print_trle_page(scrape_trle.get_trle_page(0, True))
    offset = 0
    while True:
        user_input = input("Press Enter for the next page (or type 'q' to quit: ")
        if user_input.lower() == 'q':
            print("Exiting...")
            break
        offset += 20
        index_view.print_trle_page(scrape_trle.get_trle_page(offset, True))


def test_trle_local():
    """Browse local TRLE data."""
    offset = 0
    index_view.print_trle_page(index_query.get_trle_page_local(offset, True))
    while True:
        user_input = input("Press Enter for the next page (or type 'q' to quit: ")
        if user_input.lower() == 'q':
            print("Exiting...")
            break
        offset += 20
        index_view.print_trle_page(index_query.get_trle_page_local(offset, True))


def test_trle_pic_local():
    """Browse local TRLE data with pictures."""
    if not index_view.check_ueberzug():
        sys.exit(1)
    offset = 1
    while True:
        page = index_query.get_trle_page_local(offset, True)
        levels = page['levels']
        covers = scrape_trle.get_trle_cover_list(levels, True)
        index_view.display_menu(levels, covers)
        for file in covers:
            try:
                os.remove(file)
            except FileNotFoundError:
                print(f"{file} not found, skipping.")
        offset += 1


def test_insert_trle_book():
    """This must not bother the server but at the same time its not
       meant for normal users but the database admin to run this,
       it could take 30-45 min. there is a problem about how TRLE sort result
       so this method might not work, cus it will miss results, there is another
       method that is much slower but will work and accurate"""

    # Get the first page to determine the total number of records
    page = scrape_trle.get_trle_page(0)
    total_records = page['records_total']

    # Insert the first page of data
    index_query.insert_trle_page(page)
    print(f"Records number:20 of {total_records}")
    time.sleep(15)

    # Start offset at 20 and loop through all records in steps of 20
    offset = 20
    while offset < total_records:
        # Fetch the next page of data
        page = scrape_trle.get_trle_page(offset)
        index_query.insert_trle_page(page)

        # Increment offset by 20 for the next batch
        if offset + 20 > total_records:
            offset = total_records
        else:
            offset += 20
        print(f"Records number:{offset} of {total_records}")

        # Introduce a random delay of 15 seconds
        time.sleep(15)


def show_menu():
    """Main menu"""
    print("\nPlease select a command:")
    print("1. new - Create a new empty index database and add static data")
    print("2. new - Create a new full index database, without info and walkthrough data")
    print("3. new - Create a new full index database, with info and walkthrough data")
    print("4. update - Insert TRLE book basic records")
    print("5. update - Insert TRLE book records with info and walkthrough")
    print("6. trle - Run http TRLE test")
    print("7. trle_local - Run TRLE local test")
    print("8. trle_local_pic - Run TRCustoms local picture cards test")
    print("0. Exit")


def main_menu():
    """Display the start options."""
    while True:
        show_menu()
        choice = input("Enter your choice: ").strip()

        if choice == "1":
            make_index_database.make_index_database()
            make_index_database.add_static_data()
        elif choice == "2":
            pass
        elif choice == "3":
            pass
        elif choice == "4":
            pass
        elif choice == "5":
            pass
        elif choice == "6":
            pass
        elif choice == "7":
            pass
        elif choice == "8":
            pass
        elif choice == "0":
            print("Exiting...")
            sys.exit(0)
        else:
            print("Invalid choice, please try again.")


if __name__ == '__main__':
    main_menu()
