#!/bin/sh

# For Linux
#vlc="/usr/bin/vlc"
# For OSX
vlc="/Applications/VLC.app/Contents/MacOS/VLC"

if [ ! -e "$vlc" ]; then
    echo " Command '$vlc' does not exist"
    exit 1
fi

for file in "$@"; do
    echo "=> Extracting frames from '$file'... "
    
    # Create a destination directory named after the video file to house the images cleanly
    dst_dir="$(dirname "$file")/$(basename "$file" | sed 's@\.[a-z][a-z][a-z]$@@')_frames"
    mkdir -p "$dst_dir"

    # Get the frame rate of the video to capture exactly 1 frame per second.
    # Defaulting to a scene-ratio of 24 if your video is roughly 24fps. 
    # Change this ratio value to match your video's frame rate (e.g., 30 or 60).
    frame_rate=24 

    $vlc -I dummy -q "$file" \
        --avcodec-hw=none \
        --aout=adummy \
        --video-filter="bluescreen:scene" \
        --bluescreen-u=60 --bluescreen-v=45 \
        --scene-format=png \
        --scene-ratio=$frame_rate \
        --scene-prefix="" \
        --scene-path="$dst_dir" \
        vlc://quit



    # Rename the files to match sequential 3-digit padding (001.png, 002.png...)
    echo "=> Padding filenames..."
    count=1
    # VLC outputs names like 00001.png or matching its internal tick count depending on settings
    for img in "$dst_dir"/*.png; do
        [ -e "$img" ] || continue
        new_name=$(printf "%03d.png" "$count")
        mv "$img" "$dst_dir/$new_name"
        count=$((count + 1))
    done

    echo "Finished! Transparent frames saved in: $dst_dir"
    echo
done

