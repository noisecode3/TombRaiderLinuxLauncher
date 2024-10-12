




def test_trle():
    print_trle_page(get_trle_page(0, True))
    offset = 0
    while True:
        user_input = input("Press Enter for the next page (or type 'q' to quit: ")
        if user_input.lower() == 'q':
            print("Exiting...")
            break
        offset += 20
        print_trle_page(get_trle_page(offset, True))


def test_trcustoms():
    page = get_trcustoms_page(1, True)
    print_trcustoms_page(page)
    offset = 1
    while True:
        user_input = input("Press Enter for the next page (or type 'q' to quit: ")
        if user_input.lower() == 'q':
            print("Exiting...")
            break
        offset += 1
        page = get_trcustoms_page(offset, True)
        print_trcustoms_page(page)


def test_trcustoms_local():
    page = get_trcustoms_page_local(1, True)
    print_trcustoms_page(page)
    offset = 1
    while True:
        user_input = input("Press Enter for the next page (or type 'q' to quit: ")
        if user_input.lower() == 'q':
            print("Exiting...")
            break
        offset += 1
        page = get_trcustoms_page_local(offset, True)
        print_trcustoms_page(page)


def test_trcustoms_pic_local():
    if not check_ueberzug():
        sys.exit(1)
    offset = 1
    while True:
        page = get_trcustoms_page_local(offset, True)
        levels = page['levels']
        covers = get_cover_list(levels)
        display_menu(levels, covers)
        for file in covers:
            try:
                os.remove(file)
            except FileNotFoundError:
                print(f"{file} not found, skipping.")
        offset += 1


def test_trle_local():
    offset = 0
    print_trle_page(get_trle_page_local(offset, True))
    while True:
        user_input = input("Press Enter for the next page (or type 'q' to quit: ")
        if user_input.lower() == 'q':
            print("Exiting...")
            break
        offset += 20
        print_trle_page(get_trle_page_local(offset, True))


def test_insert_trle_book():
    """This must not bother the server but at the same time its not
       meant for normal users but the database admin to run this. Not
       bother here means to not be "detected" as a robot and limit request rate.
       Not detected in behavior imply that there is some random delay about
       0-5 + 10 seconds just like someone would browser thru the levels and click
       the ">" arrow for 30-45 min. We get all the records for indexing them here."""

    # Get the first page to determine the total number of records
    page = get_trle_page(0)
    total_records = page['records_total']

    # Insert the first page of data
    insert_trle_page(page)
    print(f"Records number:20 of {total_records}")
    delay = random.uniform(10, 15)
    time.sleep(delay)

    # Start offset at 20 and loop through all records in steps of 20
    offset = 20
    while offset < total_records:
        # Fetch the next page of data
        page = get_trle_page(offset)
        insert_trle_page(page)

        # Increment offset by 20 for the next batch
        if offset + 20 > total_records:
            offset = total_records
        else:
            offset += 20
        print(f"Records number:{offset} of {total_records}")

        # Introduce a random delay between 10 and 15 seconds
        delay = random.uniform(10, 15)
        time.sleep(delay)


def test_insert_trcustoms_book():
    # Get the first page to determine the total number of records
    page = get_trcustoms_page(1)
    total_pages = page['total_pages']

    # Insert the first page of data
    insert_trcustoms_page(page)
    print(f"Page number:1 of {total_pages}")
    delay = random.uniform(10, 15)
    time.sleep(delay)

    # Start at page 2 and loop through all records in steps of 20
    page_number = 2
    while page_number <= total_pages:
        # Fetch the next page of data
        page = get_trcustoms_page(page_number)
        insert_trcustoms_page(page)
        print(f"Page number:{page_number} of {total_pages}")

        # Increment page number by 1 for the next batch
        page_number += 1

        # Introduce a random delay between 10 and 15 seconds
        delay = random.uniform(10, 15)
        time.sleep(delay)


def update_keys_trle(certList):
    """Check if identical certificate"""
    connection = sqlite3.connect('index.db')
    cursor = connection.cursor()
    cursor.execute("")
    """ 
    CREATE TABLE TrcustomsKey (
        TrcustomsKeyID INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
        serial TEXT NOT NULL UNIQUE,
        cert TEXT NOT NULL UNIQUE
    )
    CREATE TABLE TrleKey (
        TrleKeyID INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
        serial TEXT NOT NULL UNIQUE,
        cert TEXT NOT NULL UNIQUE
    """

    db_keys = query_return_fetchall("SELECT * FROM TrleKey", None, cursor)

    connection.commit()
    connection.close()


if __name__ == '__main__':
    lock_sock = acquire_lock()
    try:
        if len(sys.argv) != 2:
            logging.info("Usage: python3 make_index_database.py COMMAND")
            logging.info("COMMAND = new, trcustoms, trle, trcustoms_local, trle_local, insert_trcustoms_book, insert_trle_book, trcustoms_key, trle_key")
            logging.info("trcustoms_local_pic")
            sys.exit(1)
        else:
            COMMAND = sys.argv[1]
            if COMMAND == "new":
                make_index_database()
                add_static_data()
            if COMMAND == "check_trle_doubles":
                check_trle_doubles()
            if COMMAND == "insert_trle_book":
                test_insert_trle_book()
            if COMMAND == "insert_trcustoms_book":
                test_insert_trcustoms_book()
            if COMMAND == "trle":
                test_trle()
            if COMMAND == "trle_id":
                print(len(get_trle_level_local_by_id(3528)))
            if COMMAND == "trcustoms_id":
                print(get_trcustoms_level_local_by_id(23))

            if COMMAND == "trcustoms_local_pic":
                test_trcustoms_pic_local()

            if COMMAND == "trle_local":
                test_trle_local()
            if COMMAND == "trcustoms":
                test_trcustoms()
            if COMMAND == "trcustoms_local":
                test_trcustoms_local()
            if COMMAND == "trcustoms_key":
                resp = get_response("https://crt.sh/?q=trcustoms.org&exclude=expired", 'text/html')
                key_list = print_key_list(resp)
                for key in key_list:
                    get_key(key)
            if COMMAND == "trle_key":
                resp = get_response("https://crt.sh/?q=www.trle.net&exclude=expired", 'text/html')
                key_list = print_key_list(resp)
                for key in key_list:
                    get_key(key)
    finally:
        release_lock(lock_sock)
