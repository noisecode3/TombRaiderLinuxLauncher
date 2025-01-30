#!/bin/bash
for file in *.mp3; do ffmpeg -i "$file" -c:a libvorbis -q:a 4 "Track$(printf '%02d' "${file%.*}").ogg"; done
rm -f ./*.mp3
