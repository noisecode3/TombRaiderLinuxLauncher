#!/bin/bash
cd "$(dirname "$0")" || exit 1

if [ -d "$1/Desktop-Debug" ]; then
    TOMBLL_USER_SHARE="$1/Desktop-Debug/tester_root/usr/share/TombRaiderLinuxLauncher"
else
    TOMBLL_USER_SHARE="$1/tester_root/usr/share/TombRaiderLinuxLauncher"
fi

mkdir -p "$TOMBLL_USER_SHARE"
mkdir -p "$1/tester_root/PROGRAM_FILES"
cp -f \
  ../database/data_factory.py \
  ../database/get_leaf_cert.py \
  ../database/https.py \
  ../database/main.py \
  ../database/make_tombll_database.py \
  ../database/scrape_common.py \
  ../database/scrape_trle.py \
  ../database/scrape_trle_download.py \
  ../database/tombll_common.py \
  ../database/tombll_create.py \
  ../database/tombll_delete.py \
  ../database/tombll_manage_data.py \
  ../database/tombll_read.py \
  ../database/tombll_update.py \
  ../database/tombll_view.py \
  "$TOMBLL_USER_SHARE"

cp ../database/tombll.db "$TOMBLL_USER_SHARE"

cd "$TOMBLL_USER_SHARE" || exit 1
python3 tombll_manage_data.py -sc

#TOMBLL_HOME="$1/tester_root/home/tester/.local/share/TombRaiderLinuxLauncher"
#mkdir -p "$TOMBLL_HOME"
#cp ../database/tombll.db "$TOMBLL_HOME"
