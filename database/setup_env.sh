#!/bin/bash
cd "$(dirname "$0")" || exit 1
rm -fr .env
python3 -m venv .env
source .env/bin/activate
pip install pycurl types-pycurl tqdm types-tqdm cryptography types-cryptography \
    beautifulsoup4 types-beautifulsoup4 pillow types-pillow evdev \
    debugpy mypy
