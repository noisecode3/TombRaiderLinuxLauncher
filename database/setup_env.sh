#!/bin/bash
rm -fr .env
python3 -m venv .env
source .env/bin/activate
pip install pycurl types-pycurl tqdm types-tqdm cryptography beautifulsoup4 pillow
