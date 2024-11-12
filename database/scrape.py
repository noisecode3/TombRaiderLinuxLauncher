"""Scraping of all data; level info, cover images and https keys"""
import sys
import re
import os
import hashlib
import uuid
import logging
import tempfile
from io import BytesIO
from urllib.parse import urlparse, urlencode, parse_qs
from datetime import datetime
from bs4 import BeautifulSoup, Tag
from PIL import Image

import data_factory
import https

# Set up logging
logging.basicConfig(level=logging.DEBUG, format='%(asctime)s %(levelname)s:%(message)s')
logging.getLogger("requests").setLevel(logging.DEBUG)

###############################################################################
# Some basic URL converters
###############################################################################


def trle_search_parser(url):
    """
    Prepares a URL for level title searches on TRLE by encoding special characters.

    Note: This function should generally be avoided in favor of searching the local
    database, as it may not fully cover all cases or include recent updates.

    Args:
        url (str): The original URL string to be formatted for TRLE search compatibility.

    Returns:
        str: The formatted URL with specific characters
        replaced to match TRLE encoding requirements.
    """
    return url.replace(" ", r"+").replace(":", r"%3A") \
              .replace("!", r"%21").replace("#", r"%23") \
              .replace("/", r"%2F").replace("&", r"%26") \
              .replace("'", r"%5C%27")


def url_postfix(url):
    """
    Extracts the file extension from a URL without the leading dot.

    Args:
        url (str): The URL to extract the file extension from.

    Returns:
        str: The file extension without the leading dot,
        or an empty string if no extension is present.
    """
    return os.path.splitext(os.path.basename(urlparse(url).path))[1][1:]


def validate_url(url):
    """Check if a URL belongs to 'trle.net' or 'trcustoms.org'.

    Args:
        url (str): The URL to validate.

    Returns:
        str or None: The URL if it belongs to an accepted domain, otherwise None.
    """
    if url_domain(url) in {"trle.net", "trcustoms.org"}:
        return url
    return None


def url_domain(url):
    """Parse and validate a URL, ensuring it is HTTPS and from specific domains.

    This function verifies that the URL:
    1. Has both a valid scheme and network location (netloc).
    2. Uses the HTTPS scheme.
    3. Belongs to either 'trle.net' or 'trcustoms.org'.

    Args:
        url (str): The URL to check.

    Returns:
        str: The domain name if valid.

    Exits:
        Logs an error and terminates the program if validation fails.
    """
    # Parse URL and check if scheme and netloc are present.
    parsed_url = urlparse(url)
    if not all([parsed_url.scheme, parsed_url.netloc]):
        logging.error("Invalid URL format: Scheme or netloc is missing.")
        sys.exit(1)

    # Verify the URL uses HTTPS.
    if parsed_url.scheme != "https":
        logging.error("Invalid URL: Only HTTPS URLs are allowed.")
        sys.exit(1)

    # Confirm the domain is allowed.
    if parsed_url.netloc.endswith("trle.net"):
        return "trle.net"
    if parsed_url.netloc.endswith("trcustoms.org"):
        return "trcustoms.org"

    logging.error("Invalid URL: URL must belong to 'trle.net' or 'trcustoms.org'.")
    sys.exit(1)


def trle_url_to_int(url):
    """
    Converts a TRLE level URL into its corresponding integer level ID.

    This function processes URLs from the TRLE website that contain a level ID
    as a query parameter (lid). The following URL formats are usually used:

    1. Level features page:
       https://www.trle.net/sc/levelfeatures.php?lid=

    2. Level download page:
       https://www.trle.net/scadm/trle_dl.php?lid=

    3. Level walkthrough page:
       https://www.trle.net/sc/Levelwalk.php?lid=

    Args:
        url (str): The URL string to be processed.

    Returns:
        int or None: The level ID as an integer if extraction is successful,
                     otherwise None.
    """
    try:
        # Parse the URL and extract the 'lid' query parameter.
        lid_value = int(parse_qs(urlparse(url).query).get('lid', [None])[0])
        return lid_value
    except (TypeError, ValueError):
        # Return None if the 'lid' could not be converted to an integer.
        return None


def is_valid_uuid(value):
    """Validate uuid format"""
    try:
        uuid_obj = uuid.UUID(value, version=4)
        return str(uuid_obj) == value
    except ValueError:
        return False


###############################################################################
# raw data converters
###############################################################################

def calculate_md5(data):
    """Calculate the MD5 checksum of the given data."""
    md5_hash = hashlib.md5(usedforsecurity=False)
    md5_hash.update(data)
    return md5_hash.hexdigest()


def cover_to_tempfile(data):
    """Save the image to a temporary file."""
    with tempfile.NamedTemporaryFile(delete=False, suffix=".webp") as temp_image_file:
        temp_image_file.write(data)
        return temp_image_file.name


def cover_resize_or_convert_to_webp(input_img, width=None, height=None):
    """Resize and/or convert image to .webp format."""
    img = Image.open(BytesIO(input_img))

    if width is None and height is None:
        original_width, original_height = img.size
        if width is None:
            width = original_width
        if height is None:
            height = original_height
        img = img.resize((width, height))

    webp_image = BytesIO()

    # Convert the image to .webp format
    img.save(webp_image, format='WEBP')

    # Get the image data as bytes
    return webp_image.getvalue()


def get_trle_cover_by_id(trle_id):
    """
    Fetch the TRLE level cover image by level ID.

    This function takes a TRLE level ID, validates it, and uses it to
    construct the filename for the level's cover image. It then retrieves
    the image using `get_trle_cover`.

    Args:
        trle_id (str): The level ID as a string, which should be numeric.

    Returns:
        The cover image data returned by `get_trle_cover`.

    Exits:
        Prints an error and exits if `trle_id` is not a valid numeric string.
    """
    # Validate that the trle_id is numeric.
    if not trle_id.isdigit():
        print("Invalid ID number.")
        sys.exit(1)

    # Construct the image filename (e.g., '1234.jpg') and fetch the cover image.
    return get_trle_cover(f"{trle_id}.jpg")


def normalize_level_name(name):
    """
    Normalizes a level name string for creating consistent
    zip file names and enabling lenient searches.

    This function removes spaces and special characters to create a simplified version
    of the level name suitable for use in file naming conventions, particularly for
    zip files, and to standardize level names in searches.

    Args:
        name (str): The level name to be processed.

    Returns:
        str: A normalized level name with spaces and special characters removed.
    """
    return name.replace(" ", r"").replace("'", r"").replace("-", r"") \
               .replace(":", r"").replace("!", r"") \
               .replace("#", r"").replace("/", r"").replace("&", r"")


def convert_to_iso(date_str):
    """Convert date string from various formats to ISO-8601 (YYYY-MM-DD) format."""

    # Try to parse '01-Jan-2024' format
    try:
        return datetime.strptime(date_str, '%d-%b-%Y').strftime('%Y-%m-%d')
    except ValueError:
        pass

    # Try to parse '2024-09-24T15:12:19.212984Z' ISO format with time and microseconds
    try:
        return datetime.strptime(date_str, '%Y-%m-%dT%H:%M:%S.%fZ').strftime('%Y-%m-%d')
    except ValueError:
        pass

    # Try to parse '1999-08-29T00:00:00Z' ISO format without microseconds
    try:
        return datetime.strptime(date_str, '%Y-%m-%dT%H:%M:%SZ').strftime('%Y-%m-%d')
    except ValueError:
        pass

    raise ValueError(f"Unsupported date format: {date_str}")


###############################################################################
# Getters functions that return data from the sites with get response
###############################################################################

def get_soup(url):
    """
    Retrieves and parses the HTML content from a URL using BeautifulSoup.

    Args:
        url (str): The URL of the webpage to fetch and parse.

    Returns:
        BeautifulSoup: A BeautifulSoup object representing the parsed HTML content.
    """
    if validate_url(url) is None:
        print(f"{url} had wrong domain")
        sys.exit(1)
    return BeautifulSoup(https.get(validate_url(url), 'text/html'), 'html.parser')


def get_image(url):
    """
    Fetches an image from a URL, handling both JPEG and PNG formats.

    Args:
        url (str): The URL of the image file.

    Returns:
        bytes: The image content in bytes, based on the file format.

    Raises:
        SystemExit: If the file format is unsupported.
    """
    if validate_url(url) is None:
        print(f"{url} had wrong domain")
        sys.exit(1)
    ext = url_postfix(url).lower()
    if ext in ('jpg', 'jpeg'):
        return get_jpg(url)
    if ext == 'png':
        return get_png(url)
    print(f"Invalid file format: {ext}")
    sys.exit(1)


def get_jpg(url):
    """
    Fetches a JPEG image from a URL.

    Args:
        url (str): The URL of the JPEG image file.

    Returns:
        bytes: The JPEG image content in bytes.
    """
    if validate_url(url) is None:
        print(f"{url} had wrong domain")
        sys.exit(1)
    return https.get(validate_url(url), 'image/jpeg')


def get_png(url):
    """
    Fetches a PNG image from a URL.

    Args:
        url (str): The URL of the PNG image file.

    Returns:
        bytes: The PNG image content in bytes.
    """
    if validate_url(url) is None:
        print(f"{url} had wrong domain")
        sys.exit(1)
    return https.get(validate_url(url), 'image/png')


def get_json(url):
    """
    Fetches JSON data from a URL.

    Args:
        url (str): The URL of the JSON resource.

    Returns:
        dict: The JSON data parsed into a Python dictionary.
    """
    if validate_url(url) is None:
        print(f"{url} had wrong domain")
        sys.exit(1)
    return https.get(validate_url(url), 'application/json')


def get_zip(url):
    """
    Fetches a ZIP file from a URL and returns it in dictionary format.

    Args:
        url (str): The URL of the ZIP file.

    Returns:
        dict: The ZIP file content in a dictionary format, if applicable.
    """
    if validate_url(url) is None:
        print(f"{url} had wrong domain")
        sys.exit(1)
    return https.get(validate_url(url), 'application/zip')


###############################################################################
# This section handles page scraping for level data from TRCustoms and TRLE.
#
# The TRCustoms site is reliably supported and works flawlessly with this scraping logic.
# For TRLE, page scraping is implemented primarily for viewing the site’s content.
#
# Known Issues with TRLE Scraping:
# - **Sorting Bug**: TRLE has a known bug in its record sorting mechanism, which occasionally
#   causes certain records to be omitted from page offsets. This issue is particularly
#   problematic with levels that have multiple authors.
# - **Duplicate Records**: Duplicate entries may appear on TRLE due to sorting inconsistencies.
# - **Random Sorting Errors**: The sorting errors are unpredictable, often changing daily without
#   any levels being added or modified. These errors can persist for a few hours (typically 3-5)
#   before resolving on their own, only to reappear with different records affected the next day.
#
# Due to these limitations, TRLE scraping should be treated as approximate and may require
# periodic manual checks to confirm the accuracy of fetched data.
#
# get_trcustoms_page(page_number, False)
# get_trle_page(offset, False)
###############################################################################

def get_trle_page(offset, sort_created_first=False):
    """Scrape one TRLE page where the offset starts from the earliest date."""
    params = {
        "atype": "",
        "author": "",
        "level": "",
        "class": "",
        "type": "",
        "difficulty": "",
        "durationclass": "",
        "rating": "",
        "sortidx": 8,
        "sorttype": 2 if sort_created_first else 1,
        "idx": "" if offset == 0 else str(offset)
    }
    query_string = urlencode(params)
    soup = get_soup(f"https://www.trle.net/pFind.php?{query_string}")
    page = data_factory.make_trle_page_data()
    page['offset'] = offset

    # Find total records
    span = soup.find('span', class_='navText')
    if span:
        page['records_total'] = int(span.text.strip().split()[0])
    else:
        print("Total records not found")
        sys.exit(1)

    # Find data table
    table = soup.find('table', class_='FindTable')
    if not isinstance(table, Tag):
        print("Data table not found")
        sys.exit(1)

    page['levels'] = trle_page_table(table.find_all('tr'))

    return page


def trle_page_table(table):
    """filter out data from the TRLE level table result"""
    levels = []

    # Mapping index to level data fields
    field_mapping = {
        0: ('author', lambda cell: cell.get_text(strip=True)),
        5: ('trle_id', 'title', lambda cell: (
                    cell.find('a', href=True)['href'].split('lid=')[-1]
                    if cell.find('a', href=True) else None, cell.get_text(strip=True)
                )
            ),
        6: ('difficulty', lambda cell: cell.get_text(strip=True)),
        7: ('duration', lambda cell: cell.get_text(strip=True)),
        8: ('class', lambda cell: cell.get_text(strip=True)),
        10: ('type', lambda cell: cell.get_text(strip=True)),
        13: ('release', lambda cell: convert_to_iso(cell.get_text(strip=True)))
    }

    for row in table[1:]:
        cells = row.find_all('td')
        level = data_factory.make_trle_level_data()

        for idx, cell in enumerate(cells):
            if idx in field_mapping:
                if idx == 5:
                    level['trle_id'], level['title'] = field_mapping[idx][2](cell)
                else:
                    field_name, extractor = field_mapping[idx]
                    level[field_name] = extractor(cell)

        levels.append(level)

    return levels


def get_trcustoms_page(page_number, sort_created_first=False):
    """Scrape one trcustoms page where the offset starts from the earliest date."""
    host = "https://trcustoms.org/api/levels/"
    if sort_created_first:
        sort = "-created"
    else:
        sort = "created"
    params = {
        "sort": sort,
        "is_approved": 1,
        "page": "" if page_number == 0 else str(page_number)
    }
    query_string = urlencode(params)
    data = get_json(f"{host}?{query_string}")
    page = data_factory.make_trcustoms_page_data()
    page['current_page'] = data.get('current_page')
    page['total_pages'] = data.get('last_page')
    page['records_total'] = data.get('total_count')
    results = data.get('results')

    for item in results:
        repacked_data = data_factory.make_trcustoms_level_data()
        for author in item['authors']:
            repacked_data['authors'].append(author['username'])
        for tag in item['tags']:
            repacked_data['tags'].append(tag['name'])
        for genre in item['genres']:
            repacked_data['genres'].append(genre['name'])
        repacked_data['release'] = convert_to_iso(item['created'])
        repacked_data['cover'] = item['cover']['url']
        repacked_data['cover_md5sum'] = item['cover']['md5sum']
        repacked_data['trcustoms_id'] = item['id']
        repacked_data['title'] = item['name']
        repacked_data['type'] = item['engine']['name']
        repacked_data['difficulty'] = item['difficulty'].get('name', None) \
            if item['difficulty'] else None
        repacked_data['duration'] = item['duration'].get('name', None) \
            if item['duration'] else None
        page['levels'].append(repacked_data)
    return page


###############################################################################
# Image Fetching and Processing for TRLE and TRCustoms Levels
# ------------------------------------------------------------
# This module provides functions to retrieve, validate, and process cover images
# for levels hosted on TRLE and TRCustoms. These images are processed for display
# or storage, ensuring consistent format and data integrity.
#
# Key Functions:
# - `get_trle_cover`: Retrieves a single cover image from TRLE using the level ID,
#    resizing and converting it to `.webp` format for compatibility.
# - `get_trle_cover_list`: Retrieves a list of cover images from TRLE based on
#    provided URLs, processing both primary and large cover images if available.
# - `get_trcustoms_cover`: Retrieves and validates a TRCustoms image using its UUID
#    and MD5 checksum to ensure data accuracy, with options for temporary file storage.
# - `get_trcustoms_cover_list`: Processes a list of TRCustoms images, validating each
#    with MD5 checksums and preparing them for immediate use or temporary storage.
#
# Assumptions:
# - HTTPS requests are pre-configured with appropriate headers.
# - Images are fetched in formats specified in the URL, typically JPG or PNG.
# - The `cover_resize_or_convert_to_webp` function is used to standardize image formats.
#
# These functions facilitate smooth image retrieval, data integrity checks, and
# format conversions, suitable for terminal display or further processing.
###############################################################################

def get_trle_cover_list(screen, large_screens=None, want_tempfile=False):
    """
    Retrieve and prepare a list of cover images from TRLE.

    This function processes a primary cover image (`screen`) and additional
    larger cover images (`large_screens`), retrieves each by its URL, and stores
    the processed image data in a list. Optionally, it saves images as temporary files.

    Args:
        screen (str): URL of the primary TRLE cover image.
        large_screens (list, optional): A list of URLs for additional larger TRLE cover images.
        want_tempfile (bool): If True, stores images in temporary files instead of
                              returning raw image data.

    Returns:
        list: A list containing processed image data (or temporary file paths if
              `want_tempfile` is True) for the `screen` image followed by any additional
               images in `large_screens`.
    """
    # Define the base URL to simplify URL processing.
    base_url = "https://www.trle.net/screens/"

    # Initialize an empty list to store the processed cover images.
    level_list = []

    # Process the `screen` image if it is provided.
    if screen is not None:
        # Remove the base URL part to get the relative path, then fetch the cover image.
        level_list.append(get_trle_cover(screen.replace(base_url, ""), want_tempfile))

    # Process the `large_screens` images if the list is provided.
    if large_screens is not None:
        for cover in large_screens:
            # Remove the base URL part to get the relative path, then fetch the cover image.
            level_list.append(get_trle_cover(cover.replace(base_url, ""), want_tempfile))

    # Return the list of processed images or temporary file paths.
    return level_list


def get_trle_cover(level, want_tempfile=False):
    """Retrieve and process the cover image for a TRLE level.

    This function constructs a URL to a TRLE level's cover image and then
    retrieves the image for processing. The image URL should correspond
    to images hosted on 'https://www.trle.net/screens/' and is expected
    in the form of a filename (e.g., "3175.jpg").

    Example inputs for `level`:
        - "3175.jpg" (single image file)
        - "large/3175a.jpg" (image in the 'large' directory)
        - "large/3175b.jpg", "large/3175c.jpg", etc. (for additional images)

    Args:
        level (str): The file path or name of the image to retrieve,
                     relative to the TRLE server's screen directory.

    Returns:
        Processed image data in `.webp` format, as generated by
        `cover_resize_or_convert_to_webp`.

    Exits:
        Logs an error and exits if `level` is not provided as a valid string.
    """
    # Check if the level string is provided and valid.
    if not level:
        logging.error("Invalid input: A valid image filename (e.g., '3175.jpg') is required.")
        sys.exit(1)

    # Construct the full URL for the cover image on TRLE.net.
    image_url = "https://www.trle.net/screens/" + level

    # Fetch and process the image, converting it to webp format.
    response = cover_resize_or_convert_to_webp(get_jpg(image_url))

    # If a temporary file is requested, save the image to a temp file and return its path.
    if want_tempfile:
        return cover_to_tempfile(response)

    # Otherwise, return the raw image data.
    return response


def get_trcustoms_cover_list(levels, want_tempfile=False):
    """Get a list of picture data ready to use from Trcustoms"""
    base_url = "https://data.trcustoms.org/media/level_images/"
    level_list = []

    for level in levels:
        file = level['cover'].replace(base_url, "")
        level_list.append(get_trcustoms_cover(file, level['cover_md5sum'], want_tempfile))

    return level_list


def get_trcustoms_cover(image, md5sum, want_tempfile=False):
    """
    Retrieve and validate an image from TRCustoms.

    This function downloads an image file from the TRCustoms server,
    validates the image by checking its MD5 checksum, and optionally
    stores it in a temporary file if specified.

    Args:
        image (str): The image filename, including the format (e.g., 'uuid.png').
        md5sum (str): The expected MD5 checksum of the image to ensure data integrity.
        want_tempfile (bool): If True, saves the image to a temporary file.

    Returns:
        The downloaded image data if want_tempfile is False,
        or the path to a temporary file containing the image if True.

    Exits:
        Prints an error and exits if the image UUID is invalid or the MD5 checksum fails.
    """
    # Extract the image file format (e.g., 'png' or 'jpg').
    image_format = url_postfix(image)

    # Remove the file extension from the image name to obtain the UUID.
    image_uuid = image.replace('.' + image_format, "")

    # Validate that the extracted image UUID is in a proper format.
    if not is_valid_uuid(image_uuid):
        print(f"Invalid image UUID '{image_uuid}'")
        sys.exit(1)

    # Construct the full URL for the image on the TRCustoms server.
    url = f"https://data.trcustoms.org/media/level_images/{image_uuid}.{image_format}"

    # Send a request to download the image from the URL.
    response = get_image(url)

    # Calculate the MD5 checksum of the downloaded image.
    downloaded_md5sum = calculate_md5(response)

    # Verify the downloaded image's MD5 checksum against the expected checksum.
    if downloaded_md5sum != md5sum:
        print(f"MD5 mismatch: Expected {md5sum}, got {downloaded_md5sum}")
        sys.exit(1)

    # If a temporary file is requested, save the image to a temp file and return its path.
    if want_tempfile:
        return cover_to_tempfile(response)

    # Otherwise, return the raw image data.
    return response


# ID Scraping for TRLE and TRCustoms Levels
# The TRLE and TRCustoms websites allow level data retrieval by using level IDs
# in their URLs: TRCustoms: "https://trcustoms.org/levels/"<ID> or
# "https://trcustoms.org/api/levels/"<ID> (for JSON data).
# This ID-based approach lets us build a structured index of levels for TRLE
# directly from level IDs. Note: Creating this index is slow, requiring one HTTP
# request per level, rather than a bulk request (e.g., 1 request per 20 levels).
# The final TRLE database will store details, walkthroughs, and download files,
# integrating data from both TRLE and TRCustoms.
###############################################################################

def get_trle_walkthrough(level_soup):
    """Finds the walkthrough link on the TRLE page."""
    walkthrough_link = level_soup.find('a', string='Walkthrough')
    if walkthrough_link:
        # Constructs the walkthrough URL
        url = 'https://www.trle.net/sc/' + walkthrough_link['href']
        print(url)
    else:
        logging.info("Walkthrough not found")
        return ""

    # Retrieves the walkthrough content by loading the walkthrough URL
    soup = get_soup(url)
    iframe_tag = soup.find('iframe')
    if not iframe_tag or not isinstance(iframe_tag, Tag):
        sys.exit(1)

    # Extracts the source URL from the iframe
    iframe_src = iframe_tag['src']
    if not iframe_src or not isinstance(iframe_src, str):
        sys.exit(1)

    # Fetches the walkthrough content from the extracted URL
    url = "https://www.trle.net" + iframe_src

    # Check the type of "document"
    typ = url_postfix(url)
    if typ == 'jpg':
        # we should handle all images here but right now
        # we return "" this is a bit more complex
        # want cant just give binary image as text to
        # qt, we need to implement out own html "document" thu a filter
        response = ""
        # response = https.get(url, 'image/jpeg')
    else:
        response = https.get(url, 'text/html')

    if response:
        return response
    return None


def get_trle_zip_file(soup):
    """Locates the download link for the ZIP file on the TRLE page and return it."""
    download_link = soup.find('a', string='Download')
    if download_link:
        return https.get(download_link['href'], 'application/zip')
    logging.error("Error fetching download url")
    sys.exit(1)


def get_trle_authors(soup):
    """Extracts the authors list."""
    # Find the first <td> with class "medGText"
    first_td = soup.find('td', class_='medGText')

    # Find all anchor tags within the first <td>
    author_links = first_td.find_all('a')

    # Filter the anchor tags manually based on href attribute
    author_names = []
    for link in author_links:
        href = link.get('href')
        if href and href.startswith('/sc/authorfeatures.php?aid='):
            author_names.append(link.text)
    return author_names


def get_trle_type(soup):
    """Extracts the level type."""
    return soup.find('td', string='file type:').find_next('td').get_text(strip=True) or ""


def get_trle_class(soup):
    """Extracts the level class."""
    return soup.find('td', string='class:').find_next('td').get_text(strip=True) or ""


def get_trle_release(soup):
    """Extracts the release date."""
    return soup.find('td', string='release date:').find_next('td').get_text(strip=True) or ""


def get_trle_difficulty(soup):
    """Extracts the level difficulty."""
    difficulty_td = soup.find('td', string='difficulty:')
    if difficulty_td:
        next_td = difficulty_td.find_next('td')
        if next_td:
            return next_td.get_text(strip=True)
    return ""


def get_trle_duration(soup):
    """Extracts the level duration."""
    duration_td = soup.find('td', string='duration:')
    if duration_td:
        next_td = duration_td.find_next('td')
        if next_td:
            return next_td.get_text(strip=True)
    return ""


def get_trle_body(soup):
    """Extracts the main level description body."""
    specific_tags = soup.find_all('td', class_='medGText', align='left', valign='top')
    return str(specific_tags[1]) if len(specific_tags) >= 2 else ""


def get_trle_large_screens(soup):
    """Extracts the large screens URLs to the right of page."""
    onmouseover_links = soup.find_all(lambda tag: tag.name == 'a' and 'onmouseover' in tag.attrs)
    return [link['href'] for link in onmouseover_links]


def get_trle_screen(soup):
    """Extracts the main cover image URL."""
    image_tag = soup.find('img', class_='border')
    return 'https://www.trle.net' + image_tag['src']


def get_trle_title(soup):
    """Extracts title at the to of the page."""
    title_span = soup.find('span', class_='subHeader')
    if title_span:
        title = title_span.get_text(strip=True)
        br_tag = title_span.find('br')
        if br_tag:
            return title_span.contents[0].strip()
        return title
    logging.error("Failed to retrieve trle title")
    sys.exit(1)


def get_trle_level(soup, data):
    """Calls all the other soup extracts for TRLE."""
    data['title'] = get_trle_title(soup)
    if not data['title']:
        logging.info("This was an empty page")
        return
    data['authors'] = get_trle_authors(soup)
    data['type'] = get_trle_type(soup)
    data['class'] = get_trle_class(soup)
    data['release'] = get_trle_release(soup)
    data['difficulty'] = get_trle_difficulty(soup)
    data['duration'] = get_trle_duration(soup)
    data['screen'] = get_trle_screen(soup)
    data['large_screens'] = get_trle_large_screens(soup)
    data['zip_files'] = [get_trle_zip_file(soup)]
    data['body'] = get_trle_body(soup)
    data['walkthrough'] = get_trle_walkthrough(soup)


def get_trcustoms_level(url, data):
    """Gets the main json and also looks for corresponding TRLE"""
    if "api" not in url:
        parts = url.split("/")
        url = f"{parts[0]}//{parts[2]}/api/{'/'.join(parts[3:])}"
        trcustom_level = https.get(url, 'application/json')

        title = trcustom_level['name']
        title = trle_search_parser(title)
        # Look out for + ' & !
        trle_url = get_trle_index(title)  # need to match this with simple words, no &#!...etc
        trle_soup = get_soup(trle_url)
        data['title'] = get_trle_title(trle_soup)
        data['authors'] = get_trle_authors(trle_soup)
        data['type'] = get_trle_type(trle_soup)
        data['class'] = get_trle_class(trle_soup)
        data['release'] = get_trle_release(trle_soup)
        data['difficulty'] = get_trle_difficulty(trle_soup)
        data['duration'] = get_trle_duration(trle_soup)
        data['screen'] = get_trle_screen(trle_soup)
        data['large_screens'] = get_trle_large_screens(trle_soup)
        data['zip_files'] = [get_trle_zip_file(trle_soup)]
        data['body'] = get_trle_body(trle_soup)
        data['walkthrough'] = get_trle_walkthrough(trle_soup)
        data['tags'] = [genre['name'] for genre in trcustom_level['tags']]
        data['genres'] = [genre['name'] for genre in trcustom_level['genres']]

        for file_data in trcustom_level['files']:
            zip_file = https.get(file_data['url'], 'application/zip')

            name = normalize_level_name(trcustom_level['name'])
            authors = ""
            for author in trcustom_level['authors']:
                if authors != "":
                    authors = authors + "-"
                authors = authors + author['username']

            if file_data['version'] == 1:
                version = ""
            else:
                version = f"-V{file_data['version']}"
            zip_file['name'] = f"{file_data['id']}-{name}{version}-{authors}.zip"

            zip_file['url'] = file_data['url']
            zip_file['release'] = file_data['created']
            zip_file['version'] = file_data['version']
            data['zip_files'].append(zip_file)

        data['trle_id'] = trle_url_to_int(trle_url)
        data['trcustoms_id'] = trcustom_level['id']
    return ""


# Consider implementing a separate function for TRLE title matching,
# using an exact word matching parser specific to TRLE.
# The TRLE matching mechanism requires an *exact match* for each word in the title.
# For example, searching for "EVIL" will not match a level titled "#E.V.I.L".
# Instead, we would need to search with "e.v.i.l" as each segment (word) in the
# title must be precisely matched.
#
# To achieve this, each word in the search term is separated by "+" signs:
# For instance:
# - "some:name" would be split into "some:+name"
# - "some - name" would become "some+-+name"
#
# This could potentially be handled locally with our own database,
# which would allow us to index levels by title and perform exact word searches.
# Implementing exact word matching locally would allow us to find titles
# like "some - name" by searching simply for "some+-" without relying on TRLE's
# search constraints.

def get_trle_index(title):
    """
    Searches for a level on trle.net by its title and returns the URL
    of the selected level's details page.

    Parameters:
        title (str): The title of the level to search for.

    Returns:
        str: URL of the selected level's details page on trle.net.
             If multiple results are found, prompts the user to choose one.
             If exactly one result is found, returns that URL directly.
             If no results are found, the function logs an error and exits.

    Behavior:
        - Constructs the search URL with the provided title.
        - Uses `get_soup` to fetch and parse the HTML content
            of the search results page.
        - Finds all anchor (`<a>`) tags with `href` attributes
            containing `/sc/levelfeatures.php?lid=`.
        - Displays a list of search results if multiple results are
            found and prompts the user to choose one.
        - If only one result is found, returns that URL immediately.
        - If no result is found, logs an error and terminates the program.

    Example:
        >>> get_trle_index("Some Level Name")
        "https://www.trle.net/sc/levelfeatures.php?lid=1234"
    """
    url = "https://www.trle.net/pFind.php?atype=1&author=&level=" + trle_search_parser(title)

    # Get the parsed HTML soup of the search page
    soup = get_soup(url)

    # Find all <a> tags with href containing '/sc/levelfeatures.php?lid='
    anchor_tags = soup.find_all('a', href=re.compile(r'/sc/levelfeatures\.php\?lid='))

    # Loop through each anchor tag and print the href and text for user selection
    for i, tag in enumerate(anchor_tags, start=1):
        print(f"{i}) {tag.text}, Href: {tag['href']}")

    anchor_tags_len = len(anchor_tags)

    # Handle cases based on the number of search results
    if anchor_tags_len > 1:
        number_input = int(input("Pick A Number From The List: "))
        if 1 <= number_input <= anchor_tags_len:
            return "https://www.trle.net" + anchor_tags[number_input-1]['href']
    elif anchor_tags_len == 1:
        return "https://www.trle.net" + anchor_tags[0]['href']

    # Log an error if no results are found and exit
    logging.error("trcustoms.org only not implemented")
    sys.exit(1)
