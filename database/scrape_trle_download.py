"""
Scrape download zip data from trle.net and generate a zip_file dict.

From https://www.trle.net/scadm/trle_dl.php?lid=N we want to isolate this
as there is to many spacial cases, to support low coupling.
Only call get_zip_file_info(lid) where lid is the TRLE level id number.

Documented all relations for not strait forward download

Only some of them will need lid to other domain.
Its better if we scrape with bt4 until it's gets really wired...
For example if someone would have changed the level name to something unpredictable.
Even if we end up on some other site we sill gonna use bt4

´->´= what trle.net directs the download link to.
´x-´= we find another download for the same or reworked version elsewhere.
Scraping should be able to follow but a small list with spacial cases could be accepted.

Fragments of the Core 1158 x- https://trcustoms.org/levels/1137
Land of the Rising Sun 1186 -> trlevel.de x- https://trcustoms.org/levels/1165
LB Advent Calendar 2004 - December 17th 1236 x- NO DOWNLOAD
LB Advent Calendar 2004 - December 19th 1237 x- NO DOWNLOAD
Tutankhamun's Tomb 1297 x- NO DOWNLOAD
Corsica 1300 x- NO DOWNLOAD
Reign of Chaos - Cordoba 1338 x- https://trcustoms.org/levels/1311
Reign of Chaos - Natla's Hideout 1342 x- https://trcustoms.org/levels/1311
LB Advent Calendar 2005 - Once upon a Time 1413 x- NO DOWNLOAD
The Sand Temple 1467 -> trlevel.de x> 6-the-sand-temple/
Catacomb Moments 1526 -> trlevel.de x> 7-catacomb-moments-hard-and-easy-version/
Baptism of Fire 1 - The Buried Temple 1555 -> trle document with picture link
Baptism of Fire 1 - Assyrian Secrets 1556 -> trle document with picture link
Baptism of Fire 1 - The Scroll of Destiny 1557 -> trle document with picture link
Neon God 1565 x- https://trcustoms.org/levels/1537
Jade Empire 1588 x- https://trcustoms.org/levels/1557
Lara and the Easter Egg 1635 x- https://trcustoms.org/levels/1603
Baptism of Fire 2 - Tinnos Revisited 1641 -> trle document with picture link
Baptism of Fire 2 - Hell's Nightmare 1642 -> trle document with picture link
Baptism of Fire 2 - Lost in London 1643 -> trle document with picture link
Baptism of Fire 2 - Forgotten Realm 1644 -> trle document with picture link
Area 56 171 x- NO DOWNLOAD
The Tomb Of Ramses 172 x- NO DOWNLOAD
Young Lara's Home 173 x- NO DOWNLOAD
Black Villa 1738 -> trlevel.de x- https://trcustoms.org/levels/1704
LB Advent Calendar 2007 - Dijon 1739 -> trlevel.de x- https://trcustoms.org/levels/1705
Stone of the four Elements 1794 x- NO DOWNLOAD
Cain's Home 2 1795 x- NO DOWNLOAD
Lara in a Dino Box 1811 x- NO DOWNLOAD
Gabiza's Final Level 1864 x- NO DOWNLOAD
TR Forge Advent Calendar 2008 - Mission Cinnamon Star 1912 x- NO DOWNLOAD
Rock the Easter Boat 1978 -> trlevel.de x> 18-rock-the-easter-boat/
Mystery of the Moon Temple (La Mystere du Temple de Lune) 1989 x- NO DOWNLOAD
Meeting Speed Raiding 2 (2009) 1999 x- https://trcustoms.org/levels/1972
Temple of Light 2339 x- https://trcustoms.org/levels/2311
The Tomb Of Satan 2341 x- https://trcustoms.org/levels/2313
Pyramid of the Rulers 2424 x- https://trcustoms.org/levels/2394
Temple of Qualopec 2425 x- https://trcustoms.org/levels/2395
Qualopecs Scion 2435 x- https://trcustoms.org/levels/2404
Desert Secrets 2438 x- https://trcustoms.org/levels/2407
Lost Valley Revised 2458 x- https://trcustoms.org/levels/2407
St Francis Folly 2468 x- https://trcustoms.org/levels/2437
The Triangle Of Light - Prequel 2476 x- NO DOWNLOAD
TR Forge Advent Calendar 2011 - Gabizas Search Level 2491 x- NO DOWNLOAD
Legend of Triton 2676 x- NO DOWNLOAD
HyperSquare 2727 -> hypersquare.trle.it x- NO DOWNLOAD
BtB2014 - Marahatanahaduhashitva 2757 x- NO DOWNLOAD
Emporia (Demo) 2784 x- NO DOWNLOAD
Forgotten Remnants 2798 x- https://trcustoms.org/levels/2749
Hardcore Level Project 2847 x- NO DOWNLOAD
Hardcore Project 2 2874 x- NO DOWNLOAD
Hardcore Project 3 2876 x- NO DOWNLOAD
TR Forge Advent Calendar 2015 - TR2 Revised Sneak Peek 2881 x- NO DOWNLOAD
Hardcore Project 4 2882 x- NO DOWNLOAD
Hardcore Project 5 2897 x- NO DOWNLOAD
From Darkness to Light 2919 x- NO DOWNLOAD
Tomb of Yarok 2930 x- NO DOWNLOAD
Karels Manor 2989 x- NO DOWNLOAD
Tomb Raider - Crystal of Life Remake (Demo) 3008 x- https://trcustoms.org/levels/2956
Quest for Immortality (Demo) 3013 x- https://trcustoms.org/levels/2961
Hypersquare Seasons 1 + 2 3032 x- NO DOWNLOAD
BtB2018 - Starlantis - Natlas Trail 3041 -> btb page
BtB2018 - Renaissance 3042 -> btb page
BtB2018 - The World of the 3 Wonders 3043 -> btb page
BtB2018 - Lost in Space 3044 -> btb page
BtB2018 - Laras Answer: 42! 3045 -> btb page
BtB2018 - The Frayed Ends of Sanity 3046 -> btb page
BtB2018 - Hope of Meridian 3047 -> btb page
BtB2018 - On the Edge of Forever 3048 -> btb page
Seth's Revenge 1 & 2 3213 x- NO DOWNLOAD
Tomb Raider Chronicles - Memories - Part 1 3222 x- NO DOWNLOAD
Ancient Temple Remake 3228 x- NO DOWNLOAD
BtB2020 - Beyond the Sky 3229 -> btb page
BtB2020 - Garden of the Four Seasons 3230 -> btb page
BtB2020 - The Holy Shell 3231 -> btb page
BtB2020 - The Forgotten Sealed Masks 3232 -> btb page
BtB2020 - Dolma's Fortress 3233 -> btb page
BtB2020 - Manjushri's Sword 3234 -> btb page
BtB2020 - Beyond the Caves of Nyalam 3235 -> btb page
BtB2020 - Big Trouble in Little Tibet 3236 -> btb page
BtB2020 - A Matter of Life and Death 3237 -> btb page
BtB2020 - Nest of the Souls 3238 -> btb page
BtB2020 - Save the Cintamani 3239 -> btb page
BtB2020 - A Debt in Tibet 3240 -> btb page
BtB2020 - Shigatse Stone 3241 -> btb page
BtB2020 - The Dharma Jewel 3242 -> btb page
BtB2020 - The Lost Monastery 3243 -> btb page
BtB2020 - The Beheaded Mountain 3244 -> btb page
The Mystic Artefact - Part 1 3290 x- https://trcustoms.org/levels/3224
Sanguis Virginis (Demo) 3328 x- https://trcustoms.org/levels/3262
Louvre Galleries (Demo) 3362 -> https://trcustoms.org/levels/3295
Kitten Adventure (Demo) 3379 -> https://trcustoms.org/levels/3312
Create a Classic 2021 - Fantome 3426 -> https://trcustoms.org/levels/3359
War of the Worlds 3430 -> https://sites.google.com/view/trwotw/war-of-the-worlds/download
The Forbidden Place 3470 -> https://trcustoms.org/levels/3421
Sanguis Virginis (Definitive Demo) 3491 x- https://trcustoms.org/levels/3438
The Perils Ahead 3495 x- https://trcustoms.org/levels/3466
Kitten Adventure: Ultimate Meow! 3527 -> https://trcustoms.org/levels/3446
Kitten Adventure: A Very Meow Christmas 3528 -> https://trcustoms.org/levels/3479
Shadowy's Cringe April Fools Level 3529 x- https://trcustoms.org/levels/3507
Croft Manor Revised 3543 x- https://trcustoms.org/levels/3518
The Sectored Rig 3547 x- https://trcustoms.org/levels/3590
Sanguis Virginis (Beta Release) 3554 x- https://trcustoms.org/levels/3539
Creative Concept Challenge: Kukulkan's Legacy 3561 -> https://trcustoms.org/levels/3551
Home Attack Remake 3565 x- https://trcustoms.org/levels/3556
Quatex Temple Entrance 3579 x- NO DOWNLOAD
Silent Venice, Deadly Venice 3610 x- https://trcustoms.org/levels/3627
BtB2024 - Ashes beyond Time 3634 -> btb page
BtB2024 - Legend of the Lion Prince 3635 -> btb page
BtB2024 - Peak Pyramid 3636 -> btb page
BtB2024 - Gardens of Ash 3637 -> btb page
BtB2024 - Temple of Amon-Rat 3638 -> btb page
BtB2024 - The Tale of Hatshepsut 3639 -> btb page
BtB2024 - Moonlight Temple 3640 -> btb page
BtB2024 - Challenge of the Highpriest 3641 -> btb page
BtB2024 - A Legacy from the Past 3642 -> btb page
BtB2024 - Protecting Isis 3643 -> btb page
BtB2024 - Dark Secrets of Abydos 3644 -> btb page
BtB2024 - Goddess of Time 3645 -> btb page
BtB2024 - Mystery of the Dark Pyramid 3646 -> btb page
BtB2024 - The Secret of the Sphinx 3647 -> btb page
BtB2024 - Magic Amulet of Bastet 3648 -> btb page
BtB2024 - Beyond the Falls of Wadi El-Rayan 3649 -> btb page
The Temple of Horus 2 680 x- NO DOWNLOAD
Re-enter VCI Labs 805 -> trlevel.de x- https://trcustoms.org/levels/790
The forgotten Valley of Anubis 909 x- NO DOWNLOAD
"""
import re
import sys
import logging
from urllib.parse import urlparse, quote
from bs4 import Tag

import data_factory
import scrape_common

# Set up logging
logging.basicConfig(level=logging.DEBUG, format='%(asctime)s %(levelname)s:%(message)s')
logging.getLogger("requests").setLevel(logging.DEBUG)


def _get_trle_info(lid):
    trle_soup = scrape_common.get_soup(f"https://www.trle.net/sc/levelfeatures.php?lid={lid}")
    if isinstance(trle_soup, Tag):
        trle_title = scrape_common.get_trle_title(trle_soup)
        trle_release = scrape_common.get_trle_release(trle_soup)
        trle_zip_size = float(scrape_common.get_trle_zip_size(trle_soup))
        return [trle_title, trle_release, trle_zip_size]
    return []


def _get_trcustoms_download_info(trcustom_level_dict):
    zip_file = data_factory.make_zip_file()
    title = scrape_common.normalize_level_name(trcustom_level_dict.get('name', ''))
    authors = trcustom_level_dict.get('authors', [])

    last_file = trcustom_level_dict.get('last_file', [])
    version = last_file.get('version', [])
    zip_id = last_file.get('id', 0)
    authors_str = "-".join(author["username"] for author in authors) if authors else ""
    version_str = f"-V{version}" if version != 1 else ""

    zip_file['name'] = f"{zip_id}-{title}{version_str}-{authors_str}.zip"
    zip_file['size'] = round(last_file.get('size', 0) / (1024 * 1024), 2)  # Size in MiB
    zip_file['md5'] = ""
    zip_file['url'] = last_file.get('url', '')
    zip_file['release'] = scrape_common.convert_to_iso(last_file.get('created', ''))
    zip_file['version'] = version
    return zip_file


def _get_trle_btb_download_info(trle_btb_url, lid):
    zip_file = data_factory.make_zip_file()
    parted = trle_btb_url.split("BtB")
    btb_soup = None
    for case in ["web", "Web"]:
        btb_soup = scrape_common.get_soup(parted[0] + "BtB" + f"/{case}/downloads.html")
        if btb_soup:
            break  # Stop when a valid response is found

    if not btb_soup:
        print("Fucked up")
        sys.exit(1)
    trle_info = _get_trle_info(lid)
    trle_title = trle_info[0]
    trle_zip_size = trle_info[2]
    link_tag = None
    if isinstance(trle_info[0], str):
        link_tag = btb_soup.find("a", string=trle_title.split(" - ")[1])

    download_url = None
    if isinstance(link_tag, Tag):
        download_url = link_tag.get("href", '')
    if isinstance(download_url, str):
        zip_file['name'] = urlparse(download_url).path.split("/")[-1]
    zip_file['size'] = trle_zip_size
    zip_file['url'] = download_url
    return zip_file


def _get_generic_download(soup):
    """Extract the download link from picture page."""
    links = [a["href"] for a in soup.find_all("a") if "href" in a.attrs]
    # We only allow it land on trle with https
    link = next(
        (link for link in links if
            link.startswith((
                "http://www.trle.net/levels/levels/",
                "https://www.trle.net/levels/levels/",
                "http://trle.net/levels/levels/",
                "https://trle.net/levels/levels/"))
            and link.endswith(".zip")), None)

    # Convert to HTTPS if found
    if link:
        link = link.replace("http://", "https://", 1)

    # link
    print(f"get_generic_download link: {link}")
    return link


def _get_trlevel_download_info(trle_info):
    zip_file = data_factory.make_zip_file()
    url = "https://www.trlevel.de/filebase/index.php?category-file-list/558-trle-custom-levels/"
    page_prefix = "&sortField=time&sortOrder=DESC&pageNo="
    number = 1
    max_pages = 20

    while True:
        trlevel_soup = scrape_common.get_soup(url + f"{page_prefix}{number}")
        if not trlevel_soup:
            print(f"Failed to fetch page {number}")
            break

        card_links = trlevel_soup.find_all("a", class_="filebaseFileCardLink")
        for link in card_links:
            if trle_info[0].lower() in link.get_text(strip=True).lower():
                level = link['href'].split("https://www.trlevel.de/filebase/index.php?file/")[1]
                level_id = level.split('-')[0]
                download_url = f"https://www.trlevel.de/index.php?file-download/{level_id}/"
                head = scrape_common.https.get(f"{download_url}", "head")
                match = re.search(r'filename\*?=(?:UTF-8\'\')?["\']?([^"\']+)["\']?', head)
                zip_file['name'] = match.group(1) if match else ''
                zip_file['size'] = trle_info[2]
                zip_file['md5'] = ""
                zip_file['url'] = download_url
                break

        if len(card_links) < 20:
            break  # Stop if less than 20 links (end of pages)

        number += 1
        if number > max_pages:
            print("Reached max page limit, stopping.")
            break  # Prevent infinite loops

    return zip_file


def _search_trcustoms(trle_info):
    title = quote(trle_info[0])
    release = trle_info[1]
    trcustoms = scrape_common.get_json(f"https://trcustoms.org/api/levels/?search={title}")
    results = trcustoms.get('results', [])
    for level in results:
        # if level and date match exactly
        if level.get('name', '') == trle_info[0]:
            if scrape_common.convert_to_iso(level.get('created', '')) == \
                    scrape_common.convert_to_iso(release):
                # check if it has the files attribute
                last_file = level.get('last_file', {})
                if last_file:
                    return _get_trcustoms_download_info(level)
    return []


def _check_lid(lid):
    if not isinstance(lid, int):
        if isinstance(lid, str) and lid.isdigit():
            lid = int(lid)
        else:
            logging.error("Error: lid not a digit")
            sys.exit(1)

def _get_download_info(lid, url):
    zip_file = data_factory.make_zip_file()
    trle_info = _get_trle_info(lid)
    zip_file['name'] = urlparse(url).path.split("/")[-1]
    zip_file['size'] = trle_info[2]
    zip_file['md5'] = ""
    zip_file['url'] = url
    return zip_file


def get_zip_file_info(lid):
    """Entery function for trle download module."""
    _check_lid(lid)
    head = scrape_common.https.get(f"https://www.trle.net/scadm/trle_dl.php?lid={lid}", 'head')
    if head:
        print(f"head: {head}")
        redirect_url = head.partition("Location: ")[2].split("\r\n", 1)[0]
        print(f"Location: {redirect_url}")
        if redirect_url:
            if redirect_url.endswith(".zip") and \
                    redirect_url.startswith("https://www.trle.net/levels/levels/"):
                return _get_download_info(lid, redirect_url)

            if redirect_url.startswith("https://www.trle.net/sc/levelfeatures.php?lid="):
                trle_info = _get_trle_info(lid)
                return _search_trcustoms(trle_info)

            if redirect_url.lower().endswith("/btb/web/index.html") and \
                    redirect_url.startswith("https://www.trle.net/levels/levels"):
                return _get_trle_btb_download_info(redirect_url, lid)

            if redirect_url.endswith(".htm") and \
                    redirect_url.startswith("https://www.trle.net/levels/levels/"):
                url = _get_generic_download(scrape_common.get_soup(redirect_url))
                return _get_download_info(lid, url)

            if redirect_url.startswith("https://trcustoms.org/levels/") and \
                    redirect_url.split("/")[-1].isdigit():
                api_url = f"https://trcustoms.org/api/levels/{redirect_url.split("/")[-1]}/"
                trcustoms_level_dict = scrape_common.get_json(api_url)
                return _get_trcustoms_download_info(trcustoms_level_dict)

            if redirect_url == "https://www.trlevel.de":
                trle_info = _get_trle_info(lid)
                return _get_trlevel_download_info(trle_info)

    return []


if __name__ == '__main__':
    if len(sys.argv) != 2:
        logging.error("Usage: python3 scrape_trle_downloads.py LID")
        logging.error("You will find the level id in URL string")
        logging.error("https://www.trle.net/sc/levelfeatures.php?lid=LID")
        sys.exit(1)
    else:
        LID = sys.argv[1]
        if LID == "":
            LID = "1978"
        ZIP_DATA = get_zip_file_info(LID)
        print(f"{ZIP_DATA}")
