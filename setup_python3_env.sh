#!/bin/bash
cd "$(dirname "$0")" || exit 1
rm -fr .venv
python3 -m venv .venv
source .venv/bin/activate
pip install pycurl types-pycurl tqdm types-tqdm cryptography types-cryptography \
    beautifulsoup4 types-beautifulsoup4 pillow types-pillow evdev \
    debugpy mypy pylint bandit ruff pydocstyle flake8 pytest
