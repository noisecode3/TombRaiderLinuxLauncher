#!/bin/bash
for file in *.wma; do ffmpeg -i "$file" -c:a libvorbis -q:a 4 "${file%.*}.ogg"; done
i=12
for file in *.ogg; do
    mv "$file" "Track$(printf %02d $i).ogg"
    ((i++))
done
rm -f ./*.wma
