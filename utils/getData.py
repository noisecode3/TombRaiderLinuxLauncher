"""
X-files
"""
import os
import sys
import time
import json
import fcntl
import hashlib
import requests
from bs4 import BeautifulSoup
if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Usage: python3 getData.py URL")
        sys.exit(1)
    else:
        url = sys.argv[1]
lock_file = '/tmp/TRLE.lock'
try:
    lock_fd = open(lock_file, 'w')
    fcntl.flock(lock_fd, fcntl.LOCK_EX | fcntl.LOCK_NB)
except IOError:
    print("Another instance is already running")
    sys.exit(1)

time.sleep(2)
# test url
# url = 'https://www.trle.net/sc/levelfeatures.php?lid=3573'

response = requests.get(url)

if response.status_code == 200:
    soup = BeautifulSoup(response.text, 'html.parser')
    # print(f'response.text: {response.text}')

    # Extracting information
    title = soup.find('span', class_='subHeader').get_text(strip=True) \
        or "missing"
    author = soup.find('a', class_='linkl').get_text(strip=True) \
        or "missing"
    type = soup.find('td', string='file type:').find_next('td').get_text(strip=True) \
        or "missing"
    levelclass = soup.find('td', string='class:').find_next('td').get_text(strip=True) \
        or "missing"
    releasedate = soup.find('td', string='release date:').find_next('td').get_text(strip=True) \
        or "missing"
    difficulty = soup.find('td', string='difficulty:').find_next('td').get_text(strip=True) \
        or "missing"
    duration = soup.find('td', string='duration:').find_next('td').get_text(strip=True) \
        or "missing"
    specific_tags = soup.find_all('td', class_='medGText', align='left', valign='top')
    if len(specific_tags) >= 2:
        body = specific_tags[1]
    else:
        body = "missing"
    zipFileSize = float(soup.find('td', string='file size:').find_next('td').get_text(strip=True).replace('MB', '')) \
        or 0.0
    download_link = soup.find('a', string='Download')
    if download_link:
        url = download_link['href']
        time.sleep(2)
        response2 = requests.head(url, allow_redirects=True)

        if response2.status_code == 200:
            # Extract the file name from the URL
            file_name = response2.url.split('/')[-1]
            zipFileName = file_name

            md5_checksum = hashlib.md5(requests.get(url).content).hexdigest()
            zipFileMd5 = md5_checksum
        else:
            print(f'Failed to retrieve file information. Status code: {response2.status_code}')

    walkthrough_link = soup.find('a', string='Walkthrough')
    if walkthrough_link:
        url = 'https://www.trle.net/sc/' + walkthrough_link['href']
        time.sleep(2)
        response3 = requests.get(url)

        if response3.status_code == 200:
            soup2 = BeautifulSoup(response3.text, 'html.parser')
            ifram_tag = soup2.find('iframe')
            ifram_src = ifram_tag['src']
            url = "https://www.trle.net" + ifram_src
            response4 = requests.get(url)
            if response4.status_code == 200:
                walkthrough = response4.text
            else:
                print(f'Failed to retrieve file information. Status code: {response3.status_code}')
        else:
            print(f'Failed to retrieve file information. Status code: {response3.status_code}')

    # Find all the onmouseover links
    onmouseover_links = soup.find_all(lambda tag: tag.name == 'a' and 'onmouseover' in tag.attrs)

    # Extract the href attribute from the onmouseover links
    hrefs = [link['href'] for link in onmouseover_links]
    screensLarge = hrefs

    image_tag = soup.find('img', class_='border')

    screen = 'https://www.trle.net' + image_tag['src']

    # Print the results
    """
    print(f'Title: {title}')
    print(f'Author: {author}')
    print(f'Type: {type}')
    print(f'Level Class: {levelclass}')
    print(f'Release Date: {releasedate}')
    print(f'Difficulty: {difficulty}')
    print(f'Duration: {duration}')
    print(f'Screen: {screen}')
    print(f'Screens Large: {screensLarge}')
    print(f'Zip File Size: {zipFileSize} MB')
    print(f'Body: {body}')
    print(f'Walkthrough: {walkthrough}')
    print(f'Zip File Name: {zipFileName}')
    print(f'Zip File Name: {zipFileMd5}')
    """
    # Create a dictionary with your variables
    data = {
        "title": title,
        "author": author,
        "type": type,
        "levelclass": levelclass,
        "releasedate": releasedate,
        "difficulty": difficulty,
        "duration": duration,
        "screen": screen,
        "screensLarge": screensLarge,
        "zipFileSize": zipFileSize,
        "zipFileName": zipFileName,
        "zipFileMd5": zipFileMd5,
        "body": body,
        "walkthrough": walkthrough
    }
    if body:
        data["body"] = str(body)
    if walkthrough:
        data["walkthrough"] = str(walkthrough)
    # Write the dictionary to a JSON file
    with open('data.json', 'w') as json_file:
        json.dump(data, json_file)
else:
    print(f'Failed to retrieve content. Status code: {response.status_code}')

lock_fd.close()
os.remove(lock_file)

