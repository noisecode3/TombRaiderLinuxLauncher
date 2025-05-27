"""Main loop for CLI menu like interfaces."""
import sys
import tombll_view


def show_menu():
    """Print main menu."""
    print("\nPlease select a command:")
    print("1. new - Create a new empty index database and add static data")
    print("2. new - Create a new full index database, without info and walkthrough data")
    print("3. new - Create a new full index database, with info and walkthrough data")
    print("4. update - Insert missing TRLE book basic records")
    print("5. update - Insert missing TRLE book records with info and walkthrough")
    print("6. trle - Run http TRLE")
    print("7. trle_local - Run TRLE local")
    print("8. trle_local_pic - Run TRLE local picture cards")
    print("0. Exit")


def main_menu():
    """Display the start options."""
    while True:
        show_menu()
        choice = input("Enter your choice: ").strip()

        if choice == "1":
            pass
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
            tombll_view.local_trle_index()
        elif choice == "8":
            tombll_view.local_trle_pictures_index()
        elif choice == "0":
            print("Exiting...")
            sys.exit(0)
        else:
            print("Invalid choice, please try again.")


if __name__ == '__main__':
    main_menu()
