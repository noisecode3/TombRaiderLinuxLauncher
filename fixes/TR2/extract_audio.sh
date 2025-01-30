#!/bin/bash

# Input files
DAT_FILE="cdaudio.dat"
MP3_FILE="cdaudio.mp3"

# Check if required files exist
if [[ ! -f "$DAT_FILE" || ! -f "$MP3_FILE" ]]; then
    echo "Error: Missing $DAT_FILE or $MP3_FILE"
    exit 1
fi

# Function to convert sectors to HH:MM:SS.MS format
sector_to_time() {
    awk -v ms="$1" 'BEGIN {
        seconds = ms / 1000;
        hours = int(seconds / 3600);
        minutes = int((seconds % 3600) / 60);
        whole_seconds = int(seconds) % 60;
        milliseconds = ms % 1000;
        printf "%02d:%02d:%02d.%03d\n", hours, minutes, whole_seconds, milliseconds;
    }'
}

# Read the DAT file and extract track info
while read -r track start end name; do
    name="${name%%$'\r'}"  # Strip trailing \r from name
    echo "DEBUG: track='$track', start='$start', end='$end' name='$name'"
    start_time=$(sector_to_time "$start")
    end_time=$(sector_to_time "$end")
    track_num=$(printf "%02d" "$((10#$track))")
    output_file="Track${track_num}.ogg"
    echo "Extracting: $output_file (from $start_time to $end_time)"
    ffmpeg -nostdin -i "$MP3_FILE" -ss "$start_time" -to "$end_time" -c:a libvorbis -q:a 5 "$output_file"
done < "$DAT_FILE"

echo "Extraction complete!"

