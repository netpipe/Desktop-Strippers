#!/bin/bash
for file in "$@"; do
    echo "=> Processing '$file' with FFmpeg..."
    
    dst_dir="$(dirname "$file")/$(basename "$file" | sed 's@\.[a-z][a-z][a-z]$@@')_frames"
    mkdir -p "$dst_dir"

    # FFmpeg chroma-key filter extracts 1 frame per second (-r 1)
    # colorkey parameters: 0x00FF00 (Pure Green), 1.3 (similarity), 0.2 (blend smoothness)
    ffmpeg -i "$file" -r 5 -vf "colorkey=0x00FF00:0.3:0.1" "$dst_dir/%03d.png"

    echo "Finished! Frames saved in: $dst_dir"
done
