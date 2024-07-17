"""
Grab raw data from trle.net and put it in a data.json file
"""
import os
import sys
import time
import json
import fcntl
import hashlib
import requests
import logging
from bs4 import BeautifulSoup

# Set up logging
logging.basicConfig(level=logging.DEBUG, format='%(asctime)s %(levelname)s:%(message)s')
logging.getLogger("requests").setLevel(logging.DEBUG)
logging.getLogger("urllib3").setLevel(logging.DEBUG)

if __name__ == "__main__":
    if len(sys.argv) != 2:
        logging.error("Usage: python3 getData.py URL")
        sys.exit(1)
    else:
        url = sys.argv[1]

lock_file = '/tmp/TRLE.lock'
try:
    lock_fd = open(lock_file, 'w')
    fcntl.flock(lock_fd, fcntl.LOCK_EX | fcntl.LOCK_NB)
except IOError:
    logging.error("Another instance is already running")
    sys.exit(1)

time.sleep(2)
# test url
# url = 'https://www.trle.net/sc/levelfeatures.php?lid=3573'

cert = '/home/noisecode3/mySecretVirusFolder/trle-net-chain.pem'

try:
    response = requests.get(url, verify=cert)
    response.raise_for_status()
except requests.exceptions.RequestException as e:
    logging.error(f"Error fetching URL {url}: {e}")
    sys.exit(1)

if response.status_code == 200:
    soup = BeautifulSoup(response.text, 'html.parser')
    logging.debug(f'response.text: {response.text}')

    title_span = soup.find('span', class_='subHeader')
    if title_span:
        title = title_span.get_text(strip=True)
        br_tag = title_span.find('br')
        if br_tag:
            title = title_span.contents[0].strip()
    else:
        title = "missing"

    author = soup.find('a', class_='linkl').get_text(strip=True) or "missing"
    type = soup.find('td', string='file type:').find_next('td').get_text(strip=True) or "missing"
    class_ = soup.find('td', string='class:').find_next('td').get_text(strip=True) or "missing"
    releaseDate = soup.find('td', string='release date:').find_next('td').get_text(strip=True) or "missing"
    difficulty = soup.find('td', string='difficulty:').find_next('td').get_text(strip=True) or "missing"
    duration = soup.find('td', string='duration:').find_next('td').get_text(strip=True) or "missing"
    
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
        url = download_link['href']
        time.sleep(2)
        try:
            response2 = requests.head(url, verify=cert, allow_redirects=True)
            response2.raise_for_status()
            download_url = response2.url
            file_name = response2.url.split('/')[-1]
            zipFileName = file_name
            md5_checksum = hashlib.md5(requests.get(url, verify=cert).content).hexdigest()
            zipFileMd5 = md5_checksum
        except requests.exceptions.RequestException as e:
            logging.error(f"Failed to retrieve file information from {url}: {e}")

    walkthrough_link = soup.find('a', string='Walkthrough')
    if walkthrough_link:
        url = 'https://www.trle.net/sc/' + walkthrough_link['href']
        time.sleep(2)
        try:
            response3 = requests.get(url, verify=cert)
            response3.raise_for_status()
            soup2 = BeautifulSoup(response3.text, 'html.parser')
            iframe_tag = soup2.find('iframe')
            iframe_src = iframe_tag['src']
            url = "https://www.trle.net" + iframe_src
            response4 = requests.get(url, verify=cert)
            if response4.status_code == 200:
                walkthrough = response4.text
            else:
                logging.error(f'Failed to retrieve iframe content. Status code: {response4.status_code}')
        except requests.exceptions.RequestException as e:
            logging.error(f"Failed to retrieve Walkthrough from {url}: {e}")

    onmouseover_links = soup.find_all(lambda tag: tag.name == 'a' and 'onmouseover' in tag.attrs)
    hrefs = [link['href'] for link in onmouseover_links]
    screensLarge = hrefs

    image_tag = soup.find('img', class_='border')
    screen = 'https://www.trle.net' + image_tag['src']

    def get_var(var_name):
        return globals().get(var_name, "")

    data = {
        "title": title,
        "author": author,
        "type": type,
        "class_": class_,
        "releaseDate": releaseDate,
        "difficulty": difficulty,
        "duration": duration,
        "screen": screen,
        "screensLarge": screensLarge,
        "zipFileSize": zipFileSize,
        "zipFileName": zipFileName,
        "zipFileMd5": zipFileMd5,
        "body": body,
        "walkthrough": get_var("walkthrough"),
        "download_url": download_url,
    }
    if body:
        data["body"] = str(body)
    try:
        if walkthrough:
            data["walkthrough"] = str(walkthrough)
    except NameError:
        data["walkthrough"] = ""

    with open('data.json', 'w') as json_file:
        json.dump(data, json_file)
else:
    logging.error(f'Failed to retrieve content. Status code: {response.status_code}')

lock_fd.close()
os.remove(lock_file)
