"""Scraping of all data; level info, cover images and https keys"""
import sys
import re
import os
import hashlib
import socket
import uuid
import time
import json
import logging
import tempfile
from io import BytesIO
from urllib.parse import urlparse, urlencode, parse_qs
from datetime import datetime
import pycurl
from bs4 import BeautifulSoup, Tag
from PIL import Image
from cryptography import x509
from cryptography.x509.oid import ExtensionOID
from cryptography.hazmat.backends import default_backend
from cryptography.hazmat.primitives import serialization

import index_data
import get_leaf_cert

MISCONFIGURED_SERVER = False
LEAF_CERT = None

# Set up logging
logging.basicConfig(level=logging.DEBUG, format='%(asctime)s %(levelname)s:%(message)s')
logging.getLogger("requests").setLevel(logging.DEBUG)

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


def get_response(url, content_type):
    """Handle all https requests"""
    valid_content_types = [
        'text/html',
        'application/json',
        'application/pkix-cert',
        'image/jpeg',
        'image/png'
    ]

    if content_type not in valid_content_types:
        logging.error("Invalid content type: %s", content_type)
        sys.exit(1)

    max_retries = 3
    retries = 0
    curl = None
    headers = None
    response_buffer = None
    temp_cert_path = None

    while retries < max_retries:
        try:
            response_buffer = BytesIO()
            headers_buffer = BytesIO()
            curl = pycurl.Curl()
            curl.setopt(pycurl.URL, url)
            curl.setopt(pycurl.WRITEDATA, response_buffer)
            curl.setopt(pycurl.WRITEHEADER, headers_buffer)

            global MISCONFIGURED_SERVER
            if MISCONFIGURED_SERVER:
                global LEAF_CERT
                if not LEAF_CERT:
                    sys.exit(1)

                # Write the certificate to a temporary file manually
                temp_cert_file = tempfile.NamedTemporaryFile(delete=False)  # `delete=False` prevents auto-deletion
                temp_cert_file.write(LEAF_CERT)
                temp_cert_file.flush()
                temp_cert_path = temp_cert_file.name
                temp_cert_file.close()  # Close the file so it can be accessed by pycurl

                # Set CAINFO to use the temporary certificate file
                curl.setopt(pycurl.CAINFO, temp_cert_path)

            headers_list = [
                'User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/91.0.4472.124 Safari/537.36',
                'Accept: */*',
            ]
            curl.setopt(pycurl.HTTPHEADER, headers_list)
            curl.perform()

            response_code = curl.getinfo(pycurl.RESPONSE_CODE)

            if response_code != 200:
                retries += 1
                time.sleep(3)
                logging.warning(f"Retrying... Response code: {response_code}")
                curl.close()
                continue

            headers = headers_buffer.getvalue().decode('utf-8')
            break

        except pycurl.error as curl_error:
            if curl_error.args[0] == 60:  # SSL certificate error
                LEAF_CERT = get_leaf_cert.run(url)

                if LEAF_CERT:
                    try:
                        LEAF_CERT = LEAF_CERT.public_bytes(encoding=serialization.Encoding.PEM)
                        MISCONFIGURED_SERVER = True
                        logging.info("Leaf certificate retrieved and applied.")
                    except Exception as e:
                        logging.error("Failed to convert leaf certificate to PEM: %s", e)
                        sys.exit(1)
                else:
                    logging.error("Failed to retrieve leaf certificate. Exiting.")
                    sys.exit(1)
                continue

            logging.error("Request failed: %s", curl_error)
            retries += 1
            if retries >= max_retries:
                logging.error("Max retries reached. Exiting.")
                sys.exit(1)

        finally:
            if temp_cert_path and os.path.exists(temp_cert_path):
                os.remove(temp_cert_path)  # Ensure the temp cert file is deleted after the request

    if curl is None:
        logging.error("No curl instance")
        sys.exit(1)

    if headers is None:
        logging.error("No headers received")
        sys.exit(1)

    if response_buffer is None:
        logging.error("No response received")
        sys.exit(1)

    # Extract Content-Type from the headers
    response_content_type = None
    for header in headers.splitlines():
        if header.lower().startswith('content-type:'):
            response_content_type = header.split(':', 1)[1].split(';')[0].strip()
            break

    # Validate and return the response based on content type
    if response_content_type == content_type:
        if content_type == 'text/html':
            return response_buffer.getvalue().decode('utf-8')
        elif content_type == 'application/json':
            return json.loads(response_buffer.getvalue().decode('utf-8'))
        elif content_type in ['image/jpeg', 'image/png']:
            return response_buffer.getvalue()
        elif content_type == 'application/pkix-cert':
            return response_buffer.getvalue()
    else:
        logging.error("Unexpected content type: %s, expected %s", response_content_type, content_type)
        sys.exit(1)
def validate_pem(pem):
    """Validate the certificate as a text"""
    # Check if the response contains a PEM key
    pem_pattern = r'-----BEGIN CERTIFICATE-----(.*?)-----END CERTIFICATE-----'
    pem_match = re.search(pem_pattern, pem, re.DOTALL)

    if not pem_match:
        print("PEM Key not found.")
        sys.exit(1)


def trle_page_table(table):
    """filter out data from the TRLE level table result"""
    levels = []

    # Mapping index to level data fields
    field_mapping = {
        0: ('author', lambda cell: cell.get_text(strip=True)),
        5: ('trle_id', 'title', lambda cell: (
                cell.find('a', href=True)['href'].split('lid=')[-1] \
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
        level = index_data.make_trle_level_data()

        for idx, cell in enumerate(cells):
            if idx in field_mapping:
                if idx == 5:
                    level['trle_id'], level['title'] = field_mapping[idx][2](cell)
                else:
                    field_name, extractor = field_mapping[idx]
                    level[field_name] = extractor(cell)

        levels.append(level)

    return levels


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
    url = f"https://www.trle.net/pFind.php?{query_string}"
    soup = BeautifulSoup(get_response(url, 'text/html'), 'html.parser')
    page = index_data.make_trle_page_data()
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


def get_trcustoms_page(page_number, sort_created_first=False):
    """Scrape one trcustoms page where the offset starts from the earliest date."""
    host = "https://trcustoms.org/api/levels/"
    if sort_created_first:
        sort="-created"
    else:
        sort="created"
    params = {
        "sort": sort,
        "is_approved": 1,
        "page": "" if page_number == 0 else str(page_number)
    }
    query_string = urlencode(params)
    url = f"{host}?{query_string}"
    data = get_response(url, 'application/json')
    if not isinstance(data, dict):
        logging.error("Data type error, expected dict got %s", type(data))
        sys.exit(1)

    page = index_data.make_trcustoms_page_data()
    page['current_page'] = data.get('current_page')
    page['total_pages'] = data.get('last_page')
    page['records_total'] = data.get('total_count')

    results = data['results']
    for item in results:
        repacked_data = index_data.make_trcustoms_level_data()
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


def get_trle_cover(trle_id):
    """Fetch TRLE level picture by id"""
    if not trle_id.isdigit():
        print("Invalid ID number.")
        sys.exit(1)
    url = f"https://www.trle.net/screens/{trle_id}.jpg"

    response = get_response(url, 'image/jpeg')
    return cover_resize_to_webp(response)


def is_valid_uuid(value):
    """Validate uuid format"""
    try:
        uuid_obj = uuid.UUID(value, version=4)
        return str(uuid_obj) == value
    except ValueError:
        return False


def calculate_md5(data):
    """Calculate the MD5 checksum of the given data."""
    md5_hash = hashlib.md5(usedforsecurity=False)
    md5_hash.update(data)
    return md5_hash.hexdigest()


def get_cover_list(levels):
    """Get a list picture data ready to use"""
    base_url = "https://data.trcustoms.org/media/level_images/"
    level_list = []

    for level in levels:
        file = level['cover'].replace(base_url, "")

        filename, ext = os.path.splitext(file)

        if ext.lower() in ('.jpg', '.jpeg', '.png'):
            level_list.append(get_trcustoms_cover(filename, level['cover_md5sum'], ext[1:]))
        else:
            print(f"Skipping level {level['title']}, invalid file format: {ext}")
            sys.exit(1)

    return level_list


def get_trcustoms_cover(image_uuid, md5sum, image_format):
    """Getting pictures from internet and displaying on the terminal"""
    if not is_valid_uuid(image_uuid):
        print("Invalid image UUID.")
        sys.exit(1)
    if image_format.lower() not in ["jpg", "jpeg", "png"]:
        print("Invalid image format.")
        sys.exit(1)

    url = f"https://data.trcustoms.org/media/level_images/{image_uuid}.{image_format}"
    if image_format.lower() == "jpg":
        image_format = "jpeg"
    response = get_response(url, f"image/{image_format}")

    # Check if the MD5 sum matches
    downloaded_md5sum = calculate_md5(response)
    if downloaded_md5sum != md5sum:
        print(f"MD5 mismatch: Expected {md5sum}, got {downloaded_md5sum}")
        sys.exit(1)

    # Save the image to a temporary file
    with tempfile.NamedTemporaryFile(delete=False, suffix=".webp") as temp_image_file:
        temp_image_file.write(cover_resize_to_webp(response))
        return temp_image_file.name


def get_trcustoms_page_cover_list(levels):
    """Fetch a list of cover images from trcustoms""" 
    base_url = "https://data.trcustoms.org/media/level_images/"
    level_list = []

    for level in levels:
        file = level['cover'].replace(base_url, "")

        filename, ext = os.path.splitext(file)

        if ext.lower() in ('.jpg', '.jpeg', '.png'):
            level_list.append(get_trcustoms_cover(filename, level['cover_md5sum'], ext[1:]))
        else:
            print(f"Skipping level {level['title']}, invalid file format: {ext}")
            sys.exit(1)

    return level_list


def cover_resize_to_webp(input_img):
    """webp is the default we use here with 320x240 max resolution"""
    img = Image.open(BytesIO(input_img))

    # Convert to terminal character size
    img = img.resize((320, 240))
    webp_image = BytesIO()

    # Convert the image to .webp format
    img.save(webp_image, format='WEBP')

    # Get the image data as bytes
    return webp_image.getvalue()


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


def get_key(id_number):
    """Get the certificate from crt"""
    # Input validation
    if not id_number.isdigit():
        print("Invalid ID number.")
        sys.exit(1)
    html = get_response(f"https://crt.sh/?id={id_number}", 'text/html')

    # Create a BeautifulSoup object
    soup = BeautifulSoup(html, 'html.parser')
    body_tag = soup.find("body")
    if not isinstance(body_tag, Tag):
        logging.error("Data type error, expected Tag got %s", type(body_tag))
        sys.exit(1)

    td_text_tag = body_tag.find("td", class_="text")
    if not isinstance(td_text_tag, Tag):
        logging.error("Data type error, expected Tag got %s", type(td_text_tag))
        sys.exit(1)

    a_tag = td_text_tag.find('a', text=re.compile(r'Serial'))
    if not isinstance(a_tag, Tag):
        logging.error("Data type error, expected Tag got %s", type(a_tag))
        sys.exit(1)

    href = a_tag['href']
    if not isinstance(href, str):
        logging.error("Data type error, expected str got %s", type(href))
        sys.exit(1)

    # Parse the query string to get the 'serial' parameter
    query_params = parse_qs(urlparse(href).query)
    serial_number = query_params.get('serial', [None])[0]
    if not serial_number:
        logging.error("Serial number not found")
        sys.exit(1)

    # Normalize serial by stripping leading zeros
    serial_number = serial_number.lstrip('0')

    if not serial_number:
        print("Serial Number tag not found.")
        sys.exit(1)

    print("Serial Number:", serial_number)

    return validate_downloaded_key(id_number, serial_number)


def validate_downloaded_key(id_number, expected_serial):
    """Validate the certificate in binary form with the cryptography module"""
    pem_key = get_response(f"https://crt.sh/?d={id_number}", 'application/pkix-cert')

    if not isinstance(pem_key, bytes):
        logging.error("Data type error, expected bytes got %s", type(pem_key))
        sys.exit(1)

    # Load the certificate
    certificate = x509.load_pem_x509_certificate(pem_key, default_backend())

    # Extract the serial number and convert it to hex (without leading '0x')
    hex_serial = f'{certificate.serial_number:x}'

    # Compare the serial numbers
    if hex_serial == expected_serial:
        print("The downloaded PEM key matches the expected serial number.")
    else:
        logging.error("Serial mismatch! Expected: %s, but got: %s", expected_serial, hex_serial)
        sys.exit(1)

    # Extract and validate the domain (Common Name)
    valid_domains = ["trle.net", "trcustoms.org", "data.trcustoms.org", "staging.trcustoms.org"]

    # Check the Common Name (CN) in the certificate subject
    comon_name = certificate.subject.get_attributes_for_oid(x509.NameOID.COMMON_NAME)[0].value
    if comon_name in valid_domains:
        print(f"Valid domain found in CN: {comon_name}")
    else:
        logging.error("Invalid domain in CN: %s", comon_name)
        sys.exit(1)

    # Extract the Subject Alternative Name (SAN) extension
    try:
        san_extension = certificate.extensions \
            .get_extension_for_oid(ExtensionOID.SUBJECT_ALTERNATIVE_NAME)

        # Extract all DNS names listed in the SAN extension
        dns_names = san_extension.value.get_values_for_type(x509.DNSName) # type: ignore

        print(f"DNS Names in SAN: {dns_names}")

        # Check if any of the DNS names match the valid domain list
        valid_domains = ["trle.net", "www.trle.net", "trcustoms.org", "*.trcustoms.org",
                         "data.trcustoms.org", 'staging.trcustoms.org']

        if all(domain in valid_domains for domain in dns_names):
            print(f"Valid domain found in SAN: {dns_names}")
        else:
            print(f"Invalid domain in SAN: {dns_names}")
            sys.exit(1)

    except x509.ExtensionNotFound:
        print("No Subject Alternative Name (SAN) extension found in the certificate.")

    pem_data = certificate.public_bytes(encoding=serialization.Encoding.PEM)
    return pem_data.decode('utf-8')


def get_key_list(html):
    """scrape keys and key status here
       we cant depend on local keys from package manger that might be incomplete"""

    soup = BeautifulSoup(html, 'html.parser')
    # Find the table containing the keys
    table = soup.find_all('table')[2]  # Adjust index if necessary

    # Iterate over the rows (skipping the header row)
    ids = []
    for row in table.find_all('tr')[1:]:
        key_column = row.find_all('td')[0]  # Get the first column
        key_striped = key_column.text.strip()  # Extract the key text
        print(f"Key: {key_striped}")
        ids.append(key_striped)

    return ids


def trcustoms_key_list():
    """Get list of utf-8 public key for Trcustoms"""
    key_list = get_response("https://crt.sh/?q=trcustoms.org&exclude=expired", 'text/html')
    validated = get_key_list(key_list)

    public_key_list = []
    for key in validated:
        time.sleep(5)
        public_key_list.append(get_key(key))

    return public_key_list


def trle_key_list():
    """Get list of utf-8 public key for TRLE"""
    resp = get_response("https://crt.sh/?q=www.trle.net&exclude=expired", 'text/html')
    key_list = get_key_list(resp)

    public_key_list = []
    for key in key_list:
        time.sleep(5)
        public_key_list.append(get_key(key))

    return public_key_list



'''
# Create a temporary file to hold the combined certificate
with tempfile.NamedTemporaryFile(mode='w', delete=False) as temp_cert_file:
    # Write all certificates into the temporary file
    for cert in cert_list:
        temp_cert_file.write(cert)
    
    # Store the name of the temporary file
    temp_cert_filename = temp_cert_file.name

# Now use this temporary file with requests for SSL verification
response = requests.get(url, verify=temp_cert_filename, timeout=5)

# Once done, you can clean up the temporary file
import os
os.remove(temp_cert_filename)
'''

#ab:a9:b5:e7:a4:8c:f3:fc:5a:73:da:16:04:36:03:20
#https://crt.sh/?serial=ab%3Aa9%3Ab5%3Ae7%3Aa4%3A8c%3Af3%3Afc%3A5a%3A73%3Ada%3A16%3A04%3A36%3A03%3A20
