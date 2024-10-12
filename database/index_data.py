"""Index dictionary data definitions"""
def make_trle_page_data():
    """trle.net page, represents the same data as a page on the site"""
    return {
        "offset": 0,
        "records_total": 0,
        "levels": [],
    }


def make_trle_level_data():
    """trle.net record data"""
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


def make_trcustoms_page_data():
    """trcustoms.org page, represents the same data as a page on the site"""
    return {
        "current_page": 0,
        "total_pages": 0,
        "records_total": 0,
        "levels": [],
    }


def make_trcustoms_level_data():
    """trcustoms.org record data"""
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
