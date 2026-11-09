"""Common scrape functions for trle.net."""
import hashlib
import logging
import os
import sys
import tempfile
import uuid
from datetime import datetime
from io import BytesIO
from urllib.parse import parse_qs, urljoin, urlparse

from bs4 import BeautifulSoup, Tag
from PIL import Image

import https

# Set up logging
logging.basicConfig(level=logging.DEBUG, format='%(asctime)s %(levelname)s:%(message)s')
logging.getLogger("requests").setLevel(logging.DEBUG)
logger = logging.getLogger(__name__)

###############################################################################
# data validators
###############################################################################


def validate_url(url):
    """Check if a URL belongs to 'trle.net', 'trcustoms.org' or 'trlevel.de'.

    Args:
        url (str): The URL to validate.

    Returns:
        str or None: The URL if it belongs to an accepted domain, otherwise None.
    """
    if url_domain(url) in {"trle.net", "trcustoms.org", "trlevel.de"}:
        return url
    return None


def url_domain(url):
    """Parse and validate a URL, ensuring it is HTTPS and from specific domains.

    This function verifies that the URL:
    1. Has both a valid scheme and network location (netloc).
    2. Uses the HTTPS scheme.
    3. Belongs to either 'trle.net', 'trcustoms.org' or 'trlevel.de'.

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
        logger.error("Invalid URL format: Scheme or netloc is missing.")
        sys.exit(1)

    # Verify the URL uses HTTPS.
    if parsed_url.scheme != "https":
        logger.error("Invalid URL: Only HTTPS URLs are allowed.")
        sys.exit(1)

    # Confirm the domain is allowed.
    if parsed_url.netloc.endswith("trle.net"):
        return "trle.net"
    if parsed_url.netloc.endswith("trcustoms.org"):
        return "trcustoms.org"
    if parsed_url.netloc.endswith("trlevel.de"):
        return "trlevel.de"

    logger.error("Invalid URL: URL must belong to 'trle.net' or 'trcustoms.org'.")
    sys.exit(1)


def is_valid_uuid(value):
    """Validate uuid format."""
    try:
        uuid_obj = uuid.UUID(value, version=4)
        return str(uuid_obj) == value
    except ValueError:
        return False


###############################################################################
# data converters and scrapers
###############################################################################

def trle_url_to_int(url):
    """
    Convert a TRLE level URL into its corresponding integer level ID.

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


def calculate_data_md5(data):
    """Calculate the MD5 checksum of the given data."""
    md5_hash = hashlib.md5(usedforsecurity=False)
    md5_hash.update(data)
    return md5_hash.hexdigest()


def calculate_file_md5(file_path):
    """Calculate the MD5 checksum of a file given its path."""
    md5_hash = hashlib.md5(usedforsecurity=False)

    try:
        with open(file_path, "rb") as f:
            for chunk in iter(lambda: f.read(4096), b""):
                md5_hash.update(chunk)
        return md5_hash.hexdigest()
    except FileNotFoundError:
        print(f"Error: File not found - {file_path}")
        return None
    except PermissionError:
        print(f"Error: Permission denied - {file_path}")
        return None


def cover_to_tempfile(data):
    """Save the image to a temporary file."""
    with tempfile.NamedTemporaryFile(delete=False, suffix=".webp") as temp_image_file:
        temp_image_file.write(data)
        return temp_image_file.name


def normalize_level_name(name):
    """
    Normalize a level name string for creating consistent zip file names.

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
               .replace("#", r"").replace("/", r"").replace("&", r"") \
               .replace("(", r"").replace(")", r"")


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

    # Try to parse ISO format with timezone offset like '2013-08-17T02:00:00+02:00'
    try:
        return datetime.fromisoformat(date_str).strftime('%Y-%m-%d')
    except ValueError:
        pass

    raise ValueError(f"Unsupported date format: {date_str}")


def get_trle_authors(soup: BeautifulSoup) -> list[str]:
    """Extract the authors list."""
    author_names: list[str] = []

    # New layout: one builder card per author
    for link in soup.select('aside.level-builders a.builder-card-link'):
        href = link.get('href')
        if not isinstance(href, str) or not href.startswith('/sc/authorfeatures.php?aid='):
            continue
        h3 = link.find('h3')
        if isinstance(h3, Tag):
            # First string is the name, the arrow span comes after it
            name = next(h3.stripped_strings, '')
            if name:
                author_names.append(name)
    if author_names:
        return author_names

    # Legacy layout: first <td class="medGText"> holds the author links
    first_td = soup.find('td', class_='medGText')
    if isinstance(first_td, Tag):
        for link in first_td.find_all('a'):
            href = link.get('href')
            if isinstance(href, str) and href.startswith('/sc/authorfeatures.php?aid='):
                author_names.append(link.get_text(strip=True))
        if author_names:
            return author_names

    logger.error("Failed to retrieve trle authors")
    sys.exit(1)


def _get_fact(soup: BeautifulSoup, label: str, legacy_label: str) -> str:
    """Look up a level fact by label, new layout first, legacy as fallback."""
    # New layout: <dl class="level-detail-facts"><dt>Label</dt><dd>Value</dd>
    for dt in soup.select('dl.level-detail-facts dt'):
        if dt.get_text(strip=True).casefold() == label.casefold():
            dd = dt.find_next_sibling('dd')
            if isinstance(dd, Tag):
                return dd.get_text(strip=True)

    # Legacy layout: <td>label:</td><td>value</td>
    td = soup.find('td', string=legacy_label)
    if isinstance(td, Tag):
        value_td = td.find_next('td')
        if isinstance(value_td, Tag):
            return value_td.get_text(strip=True)

    return ""


def get_trle_type(soup: BeautifulSoup) -> str:
    """Extract the level type."""
    return _get_fact(soup, 'Engine', 'file type:')


def get_trle_class(soup: BeautifulSoup) -> str:
    """Extract the level class."""
    return _get_fact(soup, 'Class', 'class:')


def get_trle_release(soup: BeautifulSoup) -> str:
    """Extract the release date."""
    raw_date = _get_fact(soup, 'Release date', 'release date:')
    if raw_date:
        return convert_to_iso(raw_date)
    return raw_date


def get_trle_difficulty(soup: BeautifulSoup) -> str:
    """Extract the level difficulty."""
    return _get_fact(soup, 'Difficulty', 'difficulty:')


def get_trle_duration(soup: BeautifulSoup) -> str:
    """Extract the level duration."""
    return _get_fact(soup, 'Duration', 'duration:')


def get_trle_body(soup: BeautifulSoup) -> str:
    """Extract the main level description body (HTML)."""
    tag = soup.select_one('section.level-story div')
    return str(tag) if tag else ""


def get_trle_large_screens(soup: BeautifulSoup):
    """Extract the large screens URLs to the right of page."""
    onmouseover_links = soup.find_all(lambda tag: tag.name == 'a' and 'onmouseover' in tag.attrs)
    return [link['href'] for link in onmouseover_links]


def get_trle_screen(soup: BeautifulSoup) -> str:
    """Extract the main cover image URL."""
    image_tag = soup.select_one('div.level-info-art img')
    src = image_tag.get('src') if image_tag else None
    if not isinstance(src, str):
        logger.error("Failed to retrieve trle cover image")
        sys.exit(1)
    return urljoin('https://www.trle.net', src)


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


def get_trle_title(soup: BeautifulSoup) -> str:
    """Extract the level title from a trle.net level info page."""
    title_tag = soup.select_one('h2#level-detail-title')
    if title_tag is None:
        logger.error("Failed to retrieve trle title")
        sys.exit(1)
    return title_tag.get_text(strip=True)


def get_trle_zip_size(soup: BeautifulSoup) -> str:
    """Extract the zip size in MB as a string, e.g. '1,094'."""
    size_tag = soup.select_one('span.download-size')
    if size_tag is None:
        logger.error("Failed to retrieve trle zip size")
        sys.exit(1)
    return size_tag.get_text(strip=True).split(' MB')[0]


def trle_search_parser(url: str):
    """
    Prepare a URL for level title searches on TRLE by encoding special characters.

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


def url_basename_postfix(url: str):
    """
    Extract the file extension from a URL without the leading dot.

    Args:
        url (str): The URL to extract the file extension from.

    Returns:
        str: The file extension without the leading dot,
        or an empty string if no extension is present.
    """
    return os.path.splitext(os.path.basename(urlparse(url).path))[1][1:]


def url_basename_prefix(url: str):
    """
    Extract the file naem from a URL without the extension.

    Args:
        url (str): The URL to extract the file extension from.

    Returns:
        str: The file name without the extension,
        or an empty string if no extension is present.
    """
    return os.path.splitext(os.path.basename(urlparse(url).path))[0]


###############################################################################
# getter's for https
###############################################################################

def get_soup(url: str):
    """
    Retrieve and parses the HTML content from a URL using BeautifulSoup.

    Args:
        url (str): The URL of the webpage to fetch and parse.

    Returns:
        BeautifulSoup: A BeautifulSoup object representing the parsed HTML content.
    """
    validated_url = validate_url(url)
    if validated_url is None:
        print(f"{url} had wrong domain")
        sys.exit(1)

    html = https.get(validated_url, 'text/html')
    if not isinstance(html, str):
        print(f"{url} could not grab ")
        sys.exit(1)

    return BeautifulSoup(html, 'html.parser')


def get_image(url: str):
    """
    Fetch an image from a URL, handling both JPEG and PNG formats.

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
    ext = url_basename_postfix(url).lower()
    if ext in ('jpg', 'jpeg'):
        return get_jpg(url)
    if ext == 'png':
        return get_png(url)
    print(f"Invalid file format: {ext}")
    sys.exit(1)


def get_jpg(url: str):
    """
    Fetch a JPEG image from a URL.

    Args:
        url (str): The URL of the JPEG image file.

    Returns:
        bytes: The JPEG image content in bytes.
    """
    if validate_url(url) is None:
        print(f"{url} had wrong domain")
        sys.exit(1)
    return https.get(validate_url(url), 'image/jpeg')


def get_png(url: str):
    """
    Fetch a PNG image from a URL.

    Args:
        url (str): The URL of the PNG image file.

    Returns:
        bytes: The PNG image content in bytes.
    """
    if validate_url(url) is None:
        print(f"{url} had wrong domain")
        sys.exit(1)
    return https.get(validate_url(url), 'image/png')


def get_json(url: str):
    """
    Fetch JSON data from a URL.

    Args:
        url (str): The URL of the JSON resource.

    Returns:
        dict: The JSON data parsed into a Python dictionary.
    """
    if validate_url(url) is None:
        print(f"{url} had wrong domain")
        sys.exit(1)
    return https.get(validate_url(url), 'application/json')


def get_zip(url: str):
    """
    Fetch a ZIP file from a URL and returns it in dictionary format.

    Args:
        url (str): The URL of the ZIP file.

    Returns:
        dict: The ZIP file content in a dictionary format, if applicable.
    """
    if validate_url(url) is None:
        print(f"{url} had wrong domain")
        sys.exit(1)
    return https.get(validate_url(url), 'application/zip')
