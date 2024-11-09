#!/bin/bash

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

# Define the download link
url="https://mega.nz/file/xXkV3JqJ#1Ejtd9enidYYpV3FRLO5KSzcUg7-_Jg-vNi66RKo8aI"

# Download the file using the available tool
echo "Using $downloader to download the file..."
$downloader "$url"

# Verify the checksum (assuming you want to compare it to the expected checksum)
echo "Verifying checksum..."
echo "29e7e89bc11ebe77eafbd1c78ca3f1a7  trle.tar.gz" | md5sum -c -

# Extract the tar.gz file
echo "Extracting the archive..."
tar xzf trle.tar.gz

echo "Download and extraction complete."
