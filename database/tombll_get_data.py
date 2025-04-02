"""
Grab raw data from trle.net/TRCustoms.org and put it in a data.json file
"""
import sys
import json
import logging

import scrape_trle
import data_factory

# Set up logging
logging.basicConfig(level=logging.DEBUG, format='%(asctime)s %(levelname)s:%(message)s')
logging.getLogger("requests").setLevel(logging.DEBUG)

if __name__ == '__main__':
    if len(sys.argv) != 2:
        logging.error("Usage: python3 tombll_get_data.py lid")
        sys.exit(1)
    else:
        LID = sys.argv[1]
        DATA = data_factory.make_trle_tombll_data()
        URL = f"https://www.trle.net/sc/levelfeatures.php?lid={LID}"
        SOUP = scrape_trle.scrape_common.get_soup(URL)
        scrape_trle.get_trle_level(SOUP, DATA)
        with open('data.json', mode='w', encoding='utf-8') as json_file:
            json.dump(DATA, json_file)
