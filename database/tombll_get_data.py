"""
Grab raw data from trle.net/TRCustoms.org and put it in a data.json file
"""
import sys
import json
import logging

import scrape
import data_factory

# Set up logging
logging.basicConfig(level=logging.DEBUG, format='%(asctime)s %(levelname)s:%(message)s')
logging.getLogger("requests").setLevel(logging.DEBUG)

if __name__ == '__main__':
    if len(sys.argv) != 2:
        logging.error("Usage: python3 getData.py URL")
        sys.exit(1)
    else:
        URL = sys.argv[1]
        HOST = scrape.url_domain(URL)
        DATA = data_factory.make_trle_tombll_data()
        if HOST == "trle.net":
            DATA = data_factory.make_trle_tombll_data()
            SOUP = scrape.get_soup(URL)
            scrape.get_trle_level(SOUP, DATA)
            with open('data.json', mode='w', encoding='utf-8') as json_file:
                json.dump(DATA, json_file)
        if HOST == "trcustoms.org":
            scrape.get_trcustoms_level(URL, DATA)
            with open('data.json', mode='w', encoding='utf-8') as json_file:
                json.dump(DATA, json_file)
