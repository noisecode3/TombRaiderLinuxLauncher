"""
Grab raw data from trle.net and put it in a data.json file
"""
import os
import sys
import time
import json
import fcntl
import hashlib
import logging
from urllib.parse import urlparse
import requests
from bs4 import BeautifulSoup
from tqdm import tqdm

# Set up logging
logging.basicConfig(level=logging.DEBUG, format='%(asctime)s %(levelname)s:%(message)s')
logging.getLogger("requests").setLevel(logging.DEBUG)

def validate_url(url_input):
    """Set some constraints to the URL"""
    parsed_url = urlparse(url_input)

    if not all([parsed_url.scheme, parsed_url.netloc]):
        logging.error("Invalid URL format.")
        sys.exit(1)

    if parsed_url.scheme != "https":
        logging.error("Only HTTPS URLs are allowed.")
        sys.exit(1)

    if not parsed_url.netloc.endswith("trle.net"):
        logging.error("URL must belong to the domain 'trle.net'.")
        sys.exit(1)

    return True


def calculate_md5(url_input, cert_path_input):
    """Record md5 from download URL"""
    try:
        zip_response = requests.get(url_input, verify=cert_path_input, stream=True, timeout=10)
        zip_response.raise_for_status()
        total_length = int(zip_response.headers.get('content-length', 0))

        # Create an MD5 hash object once
        md5_hash = hashlib.md5()

        # Initialize the progress bar
        with tqdm(total=total_length, unit='B', unit_scale=True, desc="Calculating MD5") \
                as progress_bar:
            for chunk in zip_response.iter_content(chunk_size=4096):
                if chunk:  # filter out keep-alive new chunks
                    md5_hash.update(chunk)  # Update the existing MD5 object
                    progress_bar.update(len(chunk))

        # Return the hex digest of the MD5 hash
        return md5_hash.hexdigest()
    except requests.exceptions.RequestException as zip_response_error:
        logging.error("Failed to download %s: %s", url_input, zip_response_error)
        return None


LOCK_FILE = '/tmp/TRLE.lock'
CERT = '/etc/ssl/certs/ca-certificates.crt'
URL = None

if __name__ == "__main__":
    if len(sys.argv) != 2:
        logging.error("Usage: python3 getData.py URL")
        sys.exit(1)
    else:
        URL = sys.argv[1]
        validate_url(URL)

try:
    with open(LOCK_FILE, 'w') as lock_fd:
        fcntl.flock(lock_fd, fcntl.LOCK_EX | fcntl.LOCK_NB)
except IOError:
    logging.error("Another instance is already running")
    sys.exit(1)

time.sleep(2)

try:
    level_response = requests.get(URL, verify=CERT, timeout=5)
    level_response.raise_for_status()
except requests.exceptions.RequestException as level_response_error:
    logging.error("Error fetching URL %s: %s", URL, level_response_error)
    sys.exit(1)

if level_response.status_code == 200:
    soup = BeautifulSoup(level_response.text, 'html.parser')
    logging.debug('level_response.text: %s', level_response.text)

    title_span = soup.find('span', class_='subHeader')
    if title_span:
        TITLE = title_span.get_text(strip=True)
        br_tag = title_span.find('br')
        if br_tag:
            TITLE = title_span.contents[0].strip()
    else:
        TITLE = "missing"

    author = soup.find('a', class_='linkl').get_text(strip=True) or ""
    type_ = soup.find('td', string='file type:').find_next('td').get_text(strip=True) or "missing"
    class_ = soup.find('td', string='class:').find_next('td').get_text(strip=True) or "missing"
    releaseDate = soup.find('td', string='release date:').find_next('td').get_text(strip=True) or ""
    difficulty_td = soup.find('td', string='difficulty:')
    if difficulty_td:
        next_td = difficulty_td.find_next('td')
        if next_td:
            DIFFICULTY = next_td.get_text(strip=True)
        else:
            DIFFICULTY = "missing"
    else:
        DIFFICULTY = "missing"
    duration_td = soup.find('td', string='duration:')
    if duration_td:
        next_td = duration_td.find_next('td')
        if next_td:
            DURATION = next_td.get_text(strip=True)
        else:
            DURATION = "missing"
    else:
        DURATION = "missing"

    specific_tags = soup.find_all('td', class_='medGText', align='left', valign='top')
    body = specific_tags[1] if len(specific_tags) >= 2 else "missing"

    zipFileSize = float(
        soup.find('td', string='file size:')
        .find_next('td')
        .get_text(strip=True)
        .replace(',', '')
        .replace('MB', '')
    ) or 0.0

    download_link = soup.find('a', string='Download')
    if download_link:
        URL = download_link['href']
        time.sleep(2)
        try:
            head_response = requests.head(URL, verify=CERT, timeout=5, allow_redirects=True)
            head_response.raise_for_status()

            # Check if the content type is a zip file
            if 'Content-Type' in head_response.headers and \
            head_response.headers['Content-Type'] == 'application/zip':
                download_url = head_response.url
                zipFileName = download_url.split('/')[-1]

                # Calculate the MD5 checksum without saving the file to disk
                zipFileMd5 = calculate_md5(download_url, CERT)

                if zipFileMd5:
                    print(f"Download URL: {download_url}")
                    print(f"File Name: {zipFileName}")
                    print(f"MD5 Checksum: {zipFileMd5}")
                else:
                    logging.error("Failed to calculate MD5 checksum.")
            else:
                logging.error("The file at {URL} is not a ZIP file. Content-Type: %s", \
                head_response.headers.get('Content-Type'))
                DOWNLOAD_URL = ''
                ZIPFILE_NAME = ''
                ZIPFILE_MD5 = ''

        except requests.exceptions.RequestException as head_response_error:
            logging.error("Failed to retrieve file information from %s: %s", \
            URL, head_response_error)

    WALKTHROUGH = ""
    walkthrough_link = soup.find('a', string='Walkthrough')
    if walkthrough_link:
        URL = 'https://www.trle.net/sc/' + walkthrough_link['href']
        time.sleep(2)
        try:
            walkthrough_response = requests.get(URL, verify=CERT, timeout=5)
            walkthrough_response.raise_for_status()
            soup2 = BeautifulSoup(walkthrough_response.text, 'html.parser')
            iframe_tag = soup2.find('iframe')
            iframe_src = iframe_tag['src']
            URL = "https://www.trle.net" + iframe_src
            response4 = requests.get(URL, verify=CERT, timeout=5)
            if response4.status_code == 200:
                WALKTHROUGH = response4.text
            else:
                logging.error('Failed to retrieve iframe content. Status code: %s', \
                response4.status_code)
        except requests.exceptions.RequestException as walkthrough_response_error:
            logging.error("Failed to retrieve Walkthrough from %s: %s", \
            URL, walkthrough_response_error)

    onmouseover_links = soup.find_all(lambda tag: tag.name == 'a' and 'onmouseover' in tag.attrs)
    hrefs = [link['href'] for link in onmouseover_links]
    screensLarge = hrefs

    image_tag = soup.find('img', class_='border')
    screen = 'https://www.trle.net' + image_tag['src']

    data = {
        "title": TITLE,
        "author": author,
        "type": type_,
        "class_": class_,
        "releaseDate": releaseDate,
        "difficulty": DIFFICULTY,
        "duration": DURATION,
        "screen": screen,
        "screensLarge": screensLarge,
        "zipFileSize": zipFileSize,
        "zipFileName": zipFileName,
        "zipFileMd5": zipFileMd5,
        "body": str(body),
        "walkthrough": WALKTHROUGH,
        "download_url": download_url,
    }

    with open('data.json', 'w') as json_file:
        json.dump(data, json_file)
else:
    logging.error('Failed to retrieve content. Status code: %s', level_response.status_code)

lock_fd.close()
os.remove(LOCK_FILE)
