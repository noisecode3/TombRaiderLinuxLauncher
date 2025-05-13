"""Index dictionary data definitions."""


def make_trle_page_data():
    """trle.net page, represents the same data as a page on the site."""
    return {
        "offset": 0,
        "records_total": 0,
        "levels": [],
    }


def make_trle_level_data():
    """trle.net record data."""
    return {
        "trle_id": 0,
        "author": "",
        "title": "",
        "difficulty": "",
        "duration": "",
        "class": "",
        "type": "",
        "release": "",
    }


def make_trle_tombll_data():
    """Level data the app uses at the moment."""
    return {
        "title": "",
        "authors": [],
        "tags": [],
        "genres": [],
        "type": "",
        "class": "",
        "release": "",
        "difficulty": "",
        "duration": "",
        "screen": "",
        "large_screens": [],
        "zip_files": [],
        "trle_id": 0,
        "trcustoms_id": 0,
        "body": "",
        "walkthrough": ""
    }


def make_trcustoms_page_data():
    """trcustoms.org page, represents the same data as a page on the site."""
    return {
        "current_page": 0,
        "total_pages": 0,
        "records_total": 0,
        "levels": [],
    }


def make_trcustoms_level_data():
    """trcustoms.org record data."""
    return {
        "trcustoms_id": 0,
        "authors": [],
        "title": "",
        "tags": [],
        "genres": [],
        "type": "",
        "release": "",
        "difficulty": "",
        "duration": "",
        "cover": "",
        "cover_md5sum": "",
    }


def make_zip_file():
    """trcustoms.org or TRLE zipfile."""
    return {
        "name": "",
        "size": 0.0,
        "md5": "",
        "url": "",
        "release": "",
        "version": ""
    }


def make_picture():
    """trcustoms.org or TRLE picture data."""
    return {
        "position": 0,
        "md5sum": "",
        "md5": b''
    }
