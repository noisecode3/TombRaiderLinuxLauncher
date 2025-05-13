"""Common scrape functions for trle.net."""
import os
import sys
import hashlib
import logging
import tempfile
import uuid
from io import BytesIO
from datetime import datetime
from urllib.parse import urlparse, parse_qs
from bs4 import BeautifulSoup
from PIL import Image

import https

# Set up logging
logging.basicConfig(level=logging.DEBUG, format='%(asctime)s %(levelname)s:%(message)s')
logging.getLogger("requests").setLevel(logging.DEBUG)

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
    if parsed_url.netloc.endswith("trlevel.de"):
        return "trlevel.de"

    logging.error("Invalid URL: URL must belong to 'trle.net' or 'trcustoms.org'.")
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

    raise ValueError(f"Unsupported date format: {date_str}")


def get_trle_zip_file(soup):
    """Locates the download link for the ZIP file on the TRLE page and return it."""
    download_link = soup.find('a', string='Download')
    if download_link:
        return get_zip(download_link['href'])
    logging.error("Error fetching download url")
    sys.exit(1)


def get_trle_authors(soup):
    """Extract the authors list."""
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
    """Extract the level type."""
    return soup.find('td', string='file type:').find_next('td').get_text(strip=True) or ""


def get_trle_class(soup):
    """Extract the level class."""
    return soup.find('td', string='class:').find_next('td').get_text(strip=True) or ""


def get_trle_release(soup):
    """Extract the release date."""
    return soup.find('td', string='release date:').find_next('td').get_text(strip=True) or ""


def get_trle_difficulty(soup):
    """Extract the level difficulty."""
    difficulty_td = soup.find('td', string='difficulty:')
    if difficulty_td:
        next_td = difficulty_td.find_next('td')
        if next_td:
            return next_td.get_text(strip=True)
    return ""


def get_trle_duration(soup):
    """Extract the level duration."""
    duration_td = soup.find('td', string='duration:')
    if duration_td:
        next_td = duration_td.find_next('td')
        if next_td:
            return next_td.get_text(strip=True)
    return ""


def get_trle_body(soup):
    """Extract the main level description body."""
    specific_tags = soup.find_all('td', class_='medGText', align='left', valign='top')
    return str(specific_tags[1]) if len(specific_tags) >= 2 else ""


def get_trle_large_screens(soup):
    """Extract the large screens URLs to the right of page."""
    onmouseover_links = soup.find_all(lambda tag: tag.name == 'a' and 'onmouseover' in tag.attrs)
    return [link['href'] for link in onmouseover_links]


def get_trle_screen(soup):
    """Extract the main cover image URL."""
    image_tag = soup.find('img', class_='border')
    return 'https://www.trle.net' + image_tag['src']


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


def get_trle_title(soup):
    """Extract title at the to of the page."""
    title_span = soup.find('span', class_='subHeader')
    if title_span:
        title = title_span.get_text(strip=True)
        br_tag = title_span.find('br')
        if br_tag:
            return title_span.contents[0].strip()
        return title
    logging.error("Failed to retrieve trle title")
    sys.exit(1)


def get_trle_zip_size(soup):
    """Extract the release date."""
    size_str = soup.find('td', string='file size:').find_next('td').get_text(strip=True) or ""
    return size_str.split(' MB')[0]


def trle_search_parser(url):
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


def url_basename_postfix(url):
    """
    Extract the file extension from a URL without the leading dot.

    Args:
        url (str): The URL to extract the file extension from.

    Returns:
        str: The file extension without the leading dot,
        or an empty string if no extension is present.
    """
    return os.path.splitext(os.path.basename(urlparse(url).path))[1][1:]


def url_basename_prefix(url):
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

def get_soup(url):
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


def get_image(url):
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


def get_jpg(url):
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


def get_png(url):
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


def get_json(url):
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


def get_zip(url):
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
