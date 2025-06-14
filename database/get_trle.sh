#!/bin/bash
cd "$(dirname "$0")" || exit 1

# Check if megadl or mega-get is installed
if command -v megadl &> /dev/null; then
    downloader="megadl"
elif command -v mega-get &> /dev/null; then
    downloader="mega-get"
else
    echo "Neither megatools (megadl) nor megacmd (mega-get) is installed."
    echo "Please install one of them to proceed."
    exit 1
fi

rm -f tombll.db

# Define the download link
url1="https://mega.nz/file/2MESHZSa#9J6VF5FlOzc-FZLjdxi-4wB-fio8N3nEZK2Fp8Nn4z4"

# Download the file using the available tool
echo "Using $downloader to download the file..."
$downloader "$url1"

# Verify the checksum (assuming you want to compare it to the expected checksum)
echo "Verifying checksum..."
md5sum1="10da17189319c0b4ea73e30b54945101"
md5sum1_dl=$(md5sum tombll.db | cut -d' ' -f1)

if [[ $md5sum1 == "$md5sum1_dl" ]]
    then
        echo "sbopkg download OK"
    else
        echo "sbopkg download faild"
        rm -f tombll.db
        exit 1
fi

echo "Download and extraction complete."
