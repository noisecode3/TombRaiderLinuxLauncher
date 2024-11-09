"""Main loop for CLI menu like interfaces"""
import sys
import os
import time
import index_view
import scrape
import index_query
import make_index_database

def test_trle():
    """Browse TRLE data"""
    index_view.print_trle_page(scrape.get_trle_page(0, True))
    offset = 0
    while True:
        user_input = input("Press Enter for the next page (or type 'q' to quit: ")
        if user_input.lower() == 'q':
            print("Exiting...")
            break
        offset += 20
        index_view.print_trle_page(scrape.get_trle_page(offset, True))


def test_trcustoms():
    """Browse Trcustom data"""
    page = scrape.get_trcustoms_page(1, True)
    index_view.print_trcustoms_page(page)
    offset = 1
    while True:
        user_input = input("Press Enter for the next page (or type 'q' to quit: ")
        if user_input.lower() == 'q':
            print("Exiting...")
            break
        offset += 1
        page = scrape.get_trcustoms_page(offset, True)
        index_view.print_trcustoms_page(page)


def test_trcustoms_local():
    """Browse local Trcustom data"""
    page = index_query.get_trcustoms_page_local(1, True)
    index_view.print_trcustoms_page(page)
    offset = 1
    while True:
        user_input = input("Press Enter for the next page (or type 'q' to quit: ")
        if user_input.lower() == 'q':
            print("Exiting...")
            break
        offset += 1
        page = index_query.get_trcustoms_page_local(offset, True)
        index_view.print_trcustoms_page(page)


def test_trcustoms_pic_local():
    """Browse local Trcustom data"""
    if not index_view.check_ueberzug():
        sys.exit(1)
    offset = 1
    while True:
        page = index_query.get_trcustoms_page_local(offset, True)
        levels = page['levels']
        covers = scrape.get_trcustoms_cover_list(levels, True)
        index_view.display_menu(levels, covers)
        for file in covers:
            try:
                os.remove(file)
            except FileNotFoundError:
                print(f"{file} not found, skipping.")
        offset += 1


def test_trle_local():
    """Browse local data"""
    offset = 0
    index_view.print_trle_page(index_query.get_trle_page_local(offset, True))
    while True:
        user_input = input("Press Enter for the next page (or type 'q' to quit: ")
        if user_input.lower() == 'q':
            print("Exiting...")
            break
        offset += 20
        index_view.print_trle_page(index_query.get_trle_page_local(offset, True))


def test_insert_trle_book():
    """This must not bother the server but at the same time its not
       meant for normal users but the database admin to run this,
       it could take 30-45 min. there is a problem about how TRLE sort result
       so this method might not work, cus it will miss results, there is another
       method that is much slower but will work and accurate"""

    # Get the first page to determine the total number of records
    page = scrape.get_trle_page(0)
    total_records = page['records_total']

    # Insert the first page of data
    index_query.insert_trle_page(page)
    print(f"Records number:20 of {total_records}")
    time.sleep(15)

    # Start offset at 20 and loop through all records in steps of 20
    offset = 20
    while offset < total_records:
        # Fetch the next page of data
        page = scrape.get_trle_page(offset)
        index_query.insert_trle_page(page)

        # Increment offset by 20 for the next batch
        if offset + 20 > total_records:
            offset = total_records
        else:
            offset += 20
        print(f"Records number:{offset} of {total_records}")

        # Introduce a random delay of 15 seconds
        time.sleep(15)


def test_insert_trcustoms_book():
    """Get index data"""
    # Get the first page to determine the total number of records
    page = scrape.get_trcustoms_page(1)
    total_pages = page['total_pages']

    # Insert the first page of data
    index_query.insert_trcustoms_page(page)
    print(f"Page number:1 of {total_pages}")
    time.sleep(15)

    # Start at page 2 and loop through all records in steps of 20
    page_number = 2
    while page_number <= total_pages:
        # Fetch the next page of data
        page = scrape.get_trcustoms_page(page_number)
        index_query.insert_trcustoms_page(page)
        print(f"Page number:{page_number} of {total_pages}")

        # Increment page number by 1 for the next batch
        page_number += 1

        # Introduce a random delay of 15 seconds
        time.sleep(15)


def show_menu():
    """Main menu"""
    print("\nPlease select a command:")
    print("1. new - Create a new index database and add static data")
    print("2. check_trle_doubles - Check for duplicate entries in TRLE")
    print("3. insert_trle_book - Insert TRLE book")
    print("4. insert_trcustoms_book - Insert TRCustoms book")
    print("5. trle - Run TRLE test")
    print("6. trcustoms_local_pic - Run TRCustoms local picture test")
    print("7. trle_local - Run TRLE local test")
    print("8. trcustoms - Run TRCustoms test")
    print("9. trcustoms_local - Run TRCustoms local test")
    print("0. Exit")

def main_menu():
    while True:
        show_menu()
        choice = input("Enter your choice: ").strip()

        if choice == "1":
            make_index_database.make_index_database()
            make_index_database.add_static_data()
        elif choice == "2":
            index_query.check_trle_doubles()
        elif choice == "3":
            test_insert_trle_book()
        elif choice == "4":
            test_insert_trcustoms_book()
        elif choice == "5":
            test_trle()
        elif choice == "6":
            test_trcustoms_pic_local()
        elif choice == "7":
            test_trle_local()
        elif choice == "8":
            test_trcustoms()
        elif choice == "9":
            test_trcustoms_local()
        elif choice == "0":
            print("Exiting...")
            sys.exit(0)
        else:
            print("Invalid choice, please try again.")

if __name__ == '__main__':
    main_menu()
