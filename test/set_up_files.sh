#!/bin/bash
cd "$(dirname "$0")" || exit 1

TOMBLL_USER_SHARE="$1/tester_root/usr/share/TombRaiderLinuxLauncher"
mkdir -p "$TOMBLL_USER_SHARE"
cp \
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
  ../database/tombll.db \
  "$TOMBLL_USER_SHARE"

#TOMBLL_HOME="$1/tester_root/home/tester/.local/share/TombRaiderLinuxLauncher"
#mkdir -p "$TOMBLL_HOME"
#cp ../database/tombll.db "$TOMBLL_HOME"
