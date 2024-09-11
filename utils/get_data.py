"""
Grab raw data from trle.net/TRCustoms.org and put it in a data.json file
"""
import re
import sys
import json
import hashlib
import logging
import socket
from urllib.parse import urlparse, parse_qs
import requests
from bs4 import BeautifulSoup
from tqdm import tqdm

# Set up logging
logging.basicConfig(level=logging.DEBUG, format='%(asctime)s %(levelname)s:%(message)s')
logging.getLogger("requests").setLevel(logging.DEBUG)

CERT = '/etc/ssl/certs/ca-certificates.crt'

def acquire_lock():
    """Create a TCP socket to ensure single instance."""
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    try:
        # Bind to localhost and a specific port
        sock.bind(('127.0.0.1', 55234))
        return sock
    except socket.error:
        logging.error("Another instance is already running")
        sys.exit(1)


def release_lock(sock):
    """Release the socket and close the connection."""
    sock.close()


def validate_url(url_input):
    """Set some constraints to the URL"""
    parsed_url = urlparse(url_input)

    if not all([parsed_url.scheme, parsed_url.netloc]):
        logging.error("Invalid URL format.")
        sys.exit(1)

    if parsed_url.scheme != "https":
        logging.error("Only HTTPS URLs are allowed.")
        sys.exit(1)

    if parsed_url.netloc.endswith("trle.net"):
        return "trle.net"

    if parsed_url.netloc.endswith("trcustoms.org"):
        return "trcustoms.org"

    logging.error("URL must belong to the domain 'trle.net' or 'trcustoms.org'.")
    sys.exit(1)


def calculate_md5(url_input, cert_path_input):
    """Record md5 from download URL"""
    try:
        zip_response = requests.get(url_input, verify=cert_path_input, stream=True, timeout=10)
        zip_response.raise_for_status()
        total_length = int(zip_response.headers.get('content-length', 0))

        # Create an MD5 hash object once
        md5_hash = hashlib.md5(usedforsecurity=False)

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


def make_dictionary():
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


def get_soup(url, cert):
    try:
        response = requests.get(url, verify=cert, timeout=5)
        response.raise_for_status()
    except requests.exceptions.RequestException as response_error:
        logging.error("Error fetching URL %s: %s", url, response_error)
        sys.exit(1)

    if response.status_code == 200:
        return BeautifulSoup(response.text, 'html.parser')
    return ""


def get_trle_walkthrough(level_soup):
    walkthrough_link = level_soup.find('a', string='Walkthrough')
    if walkthrough_link:
        url = 'https://www.trle.net/sc/' + walkthrough_link['href']
    else:
        logging.info("Walkthrough not found" )
        return ""

    try:
        soup = get_soup(url, CERT)
        iframe_tag = soup.find('iframe')
        iframe_src = iframe_tag['src']
        url = "https://www.trle.net" + iframe_src
        response = requests.get(url, verify=CERT, timeout=5)
        if response.status_code == 200:
            return response.text
        logging.error('Failed to retrieve iframe content. Status code: %s', \
        response.status_code)
    except requests.exceptions.RequestException as walkthrough_response_error:
        logging.error("Failed to retrieve Walkthrough from %s: %s", \
        url, walkthrough_response_error)


def make_zip_file():
    return {
        "name" :"",
        "size" :"",
        "md5" :"",
        "url" :"",
        "release" : "",
        "version" : ""
    }


def get_trle_zip_file(soup):
    zip_file = make_zip_file() 
    zip_file["size"] = float(
        soup.find('td', string='file size:')
        .find_next('td')
        .get_text(strip=True)
        .replace(',', '')
        .replace('MB', '')
    ) or 0.0

    download_link = soup.find('a', string='Download')
    if download_link:
        url = download_link['href']
    else:
        logging.error("Error fetching download url")
        sys.exit(1)

    try:
        head_response = requests.head(url, verify=CERT, timeout=5, allow_redirects=True)
        head_response.raise_for_status()

        # Check if the content type is a zip file
        if 'Content-Type' in head_response.headers and \
        head_response.headers['Content-Type'] == 'application/zip':
            zip_file['url'] = head_response.url
            zip_file['name'] = zip_file['url'].split('/')[-1]

            # Calculate the MD5 checksum without saving the file to disk
            zip_file['md5'] = calculate_md5(zip_file['url'], CERT)

            if zip_file['md5']:
                print(f"Download URL: {zip_file['url']}")
                print(f"File Name: {zip_file['name']}")
                print(f"MD5 Checksum: {zip_file['md5']}")
                return zip_file
            logging.error("Failed to calculate MD5 checksum.")
            return zip_file
        else:
            logging.error("The file at {zip_file['url']} is not a ZIP file. Content-Type: %s", \
            head_response.headers.get('Content-Type'))
            return zip_file

    except requests.exceptions.RequestException as head_response_error:
        logging.error("Failed to retrieve file information from %s: %s", \
        url, head_response_error)


def get_trle_authors(soup):
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
    return soup.find('td', string='file type:').find_next('td').get_text(strip=True) or ""


def get_trle_class(soup):
    return soup.find('td', string='class:').find_next('td').get_text(strip=True) or ""


def get_trle_release(soup):
    return soup.find('td', string='release date:').find_next('td').get_text(strip=True) or ""


def get_trle_difficulty(soup):
    difficulty_td = soup.find('td', string='difficulty:')
    if difficulty_td:
        next_td = difficulty_td.find_next('td')
        if next_td:
            return next_td.get_text(strip=True)
    return ""


def get_trle_duration(soup):
    duration_td = soup.find('td', string='duration:')
    if duration_td:
        next_td = duration_td.find_next('td')
        if next_td:
            return next_td.get_text(strip=True)
    return ""


def get_trle_body(soup):
    specific_tags = soup.find_all('td', class_='medGText', align='left', valign='top')
    return str(specific_tags[1]) if len(specific_tags) >= 2 else ""


def get_trle_large_screens(soup):
    onmouseover_links = soup.find_all(lambda tag: tag.name == 'a' and 'onmouseover' in tag.attrs)
    return [link['href'] for link in onmouseover_links]


def get_trle_screen(soup):
    image_tag = soup.find('img', class_='border')
    return 'https://www.trle.net' + image_tag['src']


def get_trle_title(soup):
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
    data['title'] = get_trle_title(soup)
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

def trle_search_post_parser(url):
    return url.replace(" ", r"+").replace("'", r"%5C%27").replace(":", r"%3A")\
            .replace("!", r"%21").replace("#", r"%21").replace("/", r"%2F").replace("&", r"%26")


def trle_url_to_int(url):
    try:
        lid_value = int(parse_qs(urlparse(url).query).get('lid', [None])[0])
        return lid_value
    except (TypeError, ValueError):
        return None


def get_trcustoms_level(url, data):
    if "api" not in url:
        parts = url.split("/")
        url = f"{parts[0]}//{parts[2]}/api/{'/'.join(parts[3:])}"
    try:
        response = requests.get(url, verify=CERT, timeout=5)
        response.raise_for_status()
    except requests.exceptions.RequestException as response_error:
        logging.error("Error fetching URL %s: %s", url, response_error)
        sys.exit(1)

    if response.status_code == 200:
        if 'application/json' in response.headers.get('Content-Type', ''):
            trcustom_level = response.json()
            title = trcustom_level['name']
            title = trle_search_post_parser(title)
            # Look out for + ' & !
            trle_url = get_trle_index(title)
            trle_soup = get_soup(trle_url, CERT)
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
                zip_file = make_zip_file()

                name = trcustom_level['name'].replace(" ", r"")\
                        .replace("'", r"").replace("-", r"")\
                        .replace(":", r"").replace("!", r"")\
                        .replace("#", r"").replace("/", r"").replace("&", r"")
                authors = ""
                for author in trcustom_level['authors']:
                    if authors != "":
                        authors = authors +"-"
                    authors = authors + author['username']

                if file_data['version'] == 1:
                    version = ""
                else:
                    version = f"-V{file_data['version']}"
                zip_file['name'] = f"{file_data['id']}-{name}{version}-{authors}.zip"
                zip_file['size'] =  round((file_data['size'] / 1024) / 1024, 2)
                zip_file['url'] = file_data['url']
                zip_file['release'] = file_data['created']
                zip_file['version'] = file_data['version']
                zip_file['md5'] = calculate_md5(file_data['url'], CERT)
                data['zip_files'].append(zip_file)
            data['trle_id'] =  trle_url_to_int(trle_url)
            data['trcustoms_id'] = trcustom_level['id']
    return ""

def get_trle_index(title):
    url = "https://www.trle.net/pFind.php?atype=1&author=&level=" + title
    print(url)
    try:
        response = requests.get(url, verify=CERT, timeout=5)
        response.raise_for_status()
    except requests.exceptions.RequestException as response_error:
        logging.error("Error fetching URL %s: %s", url, response_error)
        sys.exit(1)

    if response.status_code == 200:
        #print(response.text)
        soup = BeautifulSoup(response.text, 'html.parser')
        # Find all <a> tags where href contains '/sc/levelfeatures.php?lid='
        anchor_tags = soup.find_all('a', href=re.compile(r'/sc/levelfeatures\.php\?lid='))
        # Loop through each anchor tag and print the href and text
        i = 0
        for tag in anchor_tags:
            i = i+1
            print(f"{i}) {tag.text}, Href: {tag['href']}")
        anchor_tags_len = len(anchor_tags)
        if anchor_tags_len > 1:
            number_input = int(input("Pick A Number From The List: "))
            if 1  <= number_input <= anchor_tags_len:
                return "https://www.trle.net" + anchor_tags[number_input-1]['href']
        if anchor_tags_len == 1:
            return "https://www.trle.net" + anchor_tags[0]['href']
        logging.error("trcustoms.org only not implemented")
        sys.exit(1)
    return ""

    #"https://trcustoms.org/levels/"#num

# with json api
#"https://trcustoms.org/api/levels/"#num

#search trle url





# Replace custom tags with HTML spans and apply classes
# https://raw.githubusercontent.com/rr-/TRCustoms/develop/frontend/src/components/markdown-composer/MarkdownButtons/index.tsx
def custom_markdown_parser(text):
    text = re.sub(r'\[o\](.*?)\[/o\]', r'<span class="object">\1</span>', text)  # blue text for objects
    text = re.sub(r'\[s\](.*?)\[/s\]', r'<span class="secret">\1</span>', text)   # secret styling
    text = re.sub(r'\[p\](.*?)\[/p\]', r'<span class="pickup">\1</span>', text)   # pickup styling
    text = re.sub(r'\[e\](.*?)\[/e\]', r'<span class="enemy">\1</span>', text)    # enemy styling
    text = re.sub(r'\[t\](.*?)\[/t\]', r'<span class="trap">\1</span>', text)     # trap styling
    text = re.sub(r'\[center\](.*?)\[/center\]', r'<div style="text-align: center;">\1</div>', text)  # center align

    return text

# Example usage
#description = """[center]**Tomb Raider: Pandora's Box**[/center]
#[s]Secret text[/s] [o]Object text[/o] [p]Pickup text[/p]"""

#parsed_description = custom_markdown_parser(description)
#print(parsed_description)




if __name__ == '__main__':
    lock_sock = acquire_lock()
    try:
        if len(sys.argv) != 2:
            logging.error("Usage: python3 getData.py URL")
            sys.exit(1)
        else:
            URL = sys.argv[1]
            HOST = validate_url(URL)
            DATA = make_dictionary()
            if HOST == "trle.net":
                SOUP = get_soup(URL, CERT)
                get_trle_level(SOUP, DATA)
                with open('data.json', mode='w', encoding='utf-8') as json_file:
                    json.dump(DATA, json_file)
            if HOST == "trcustoms.org":
                get_trcustoms_level(URL, DATA)
                with open('data.json', mode='w', encoding='utf-8') as json_file:
                    json.dump(DATA, json_file)
    finally:
        release_lock(lock_sock)
