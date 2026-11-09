"""Scraping of all data; level info, cover images and https keys."""
import logging
import re
import sys
from urllib.parse import parse_qs, urldefrag, urlencode, urljoin, urlparse

from bs4 import BeautifulSoup, Tag

import data_factory
import scrape_common
import scrape_trle_download

# Set up logging
logging.basicConfig(level=logging.DEBUG, format='%(asctime)s %(levelname)s:%(message)s')
logging.getLogger("requests").setLevel(logging.DEBUG)
logger = logging.getLogger(__name__)


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

def get_trle_page(offset=0, sort_created_first=False):
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
        "idx": int(offset)
    }
    query_string = urlencode(params)
    soup = scrape_common.get_soup(f"https://www.trle.net/pFind.php?{query_string}")
    page = data_factory.TrlePage()
    page.offset = offset

    # Find total records
    strong = soup.select_one('div.find-result-bar p strong')
    if strong:
        page.records_total = int(strong.text.strip().replace(',', ''))
    else:
        print("Total records not found")
        sys.exit(1)

    # Find data table
    table = soup.find('table', class_='FindTable')
    if not isinstance(table, Tag):
        print("Data table not found")
        sys.exit(1)

    page.levels = trle_page_table(table.find_all('tr'))

    return page


def trle_page_table(rows):
    """Filter out data from the TRLE level table result."""
    levels = []

    for row in rows:
        cells = row.find_all('td')
        if len(cells) < 14:  # header row (th) or malformed
            continue

        level = data_factory.TrleLevel()
        level.author = cells[0].get_text(strip=True)

        link = cells[5].find('a', href=True)
        if not link:
            continue

        level.trle_id = int(parse_qs(urlparse(link['href']).query)['lid'][0])
        level.title = link.get_text(strip=True)

        level.difficulty = cells[6].get_text(strip=True)
        level.duration = cells[7].get_text(strip=True)
        level.class_ = cells[8].get_text(strip=True)
        level.type = cells[10].get_text(strip=True)
        level.release = scrape_common.convert_to_iso(cells[13].get_text(strip=True))

        levels.append(level)

    return levels


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
        logger.error("Invalid input: A valid image filename (e.g., '3175.jpg') is required.")
        sys.exit(1)

    # Construct the full URL for the cover image on TRLE.net.
    image_url = "https://www.trle.net/screens/" + level

    # Fetch and process the image, converting it to webp format.
    response = scrape_common.cover_resize_or_convert_to_webp(scrape_common.get_jpg(image_url))

    # If a temporary file is requested, save the image to a temp file and return its path.
    if want_tempfile:
        return scrape_common.cover_to_tempfile(response)

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
    """Find the walkthrough link on the TRLE page."""
    walkthrough_link = level_soup.select_one('a[href*="Levelwalk.php"]')
    href = walkthrough_link.get('href') if walkthrough_link else None
    if not isinstance(href, str):
        logger.info("Walkthrough link not found")
        return ""

    url, _ = urldefrag(urljoin('https://www.trle.net/sc/', href))

    soup = scrape_common.get_soup(url)

    # Normal case on new layout: level has no walkthrough yet
    if soup.select_one('p.walkthrough-empty'):
        logger.info("No walkthrough for %s", url)
        return ""

    iframe_tag = soup.find('iframe')
    if not isinstance(iframe_tag, Tag):
        # Unknown layout: don't kill the whole sync over a missing walkthrough
        logger.error("Unrecognized walkthrough page layout: %s", url)
        return ""

    iframe_src = iframe_tag.get('src')
    if not isinstance(iframe_src, str) or not iframe_src:
        logger.error("Iframe without src on %s", url)
        return ""

    url = urljoin('https://www.trle.net', iframe_src)

    if scrape_common.url_basename_postfix(url) == 'jpg':
        # Images are not handled yet (can't give binary as text to Qt)
        return ""

    return scrape_common.https.get(url, 'text/html') or ""


def get_trle_download_href(soup: BeautifulSoup) -> str:
    """Return the download link href, e.g. /scadm/trle_dl.php?lid=3788."""
    link = soup.select_one('a[href*="trle_dl.php?lid="]')
    href = link.get('href') if link else None
    if not isinstance(href, str):
        logger.error("Failed to retrieve trle download link")
        sys.exit(1)
    return href


def get_trle_level_card(soup: BeautifulSoup, data):
    """Call all the other soup extracts for TRLE card."""
    data.title = scrape_common.get_trle_title(soup)
    data.authors = scrape_common.get_trle_authors(soup)
    data.type = scrape_common.get_trle_type(soup)
    data.class_ = scrape_common.get_trle_class(soup)
    data.release = scrape_common.get_trle_release(soup)
    data.difficulty = scrape_common.get_trle_difficulty(soup)
    data.duration = scrape_common.get_trle_duration(soup)
    data.screen = scrape_common.get_trle_screen(soup)
    data.trle_id = scrape_common.trle_url_to_int(get_trle_download_href(soup))


def get_trle_level(soup: BeautifulSoup, data):
    """Call all the other soup extracts for TRLE."""
    data.title = scrape_common.get_trle_title(soup)
    data.authors = scrape_common.get_trle_authors(soup)
    data.type = scrape_common.get_trle_type(soup)
    data.class_ = scrape_common.get_trle_class(soup)
    data.release = scrape_common.get_trle_release(soup)
    data.difficulty = scrape_common.get_trle_difficulty(soup)
    data.duration = scrape_common.get_trle_duration(soup)
    data.screen = scrape_common.get_trle_screen(soup)
    data.large_screens = scrape_common.get_trle_large_screens(soup)
    data.trle_id = scrape_common.trle_url_to_int(get_trle_download_href(soup))
    data.zip_files = scrape_trle_download.get_zip_file_info(data.trle_id)
    data.body = scrape_common.get_trle_body(soup)
    data.walkthrough = get_trle_walkthrough(soup)


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
    Search for a level by its title and returns the URLs.

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
    url = "https://www.trle.net/pFind.php?atype=1&author=&level=" + \
        scrape_common.trle_search_parser(title)

    # Get the parsed HTML soup of the search page
    soup = scrape_common.get_soup(url)

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
    logger.error("trcustoms.org only not implemented")
    sys.exit(1)
