 
 #!/bin/sh                                                                                                                                                     
 #https://wiki.videolan.org/VLC_HowTo/Transcode_multiple_videos/
 vcodec="mp4v"
 acodec="mp4a"
 bitrate="1024"
 arate="128"
 mux="mp4"
 
 # For Linux                                                                                                                                                   
 vlc="/usr/bin/vlc"                                                                                                                                           
 # For OSX                                                                                                                                                     
 #vlc="/Applications/Utilities/VLC.app/Contents/MacOS/VLC"
 
 if [ ! -e "$vlc" ]; then
     echo "Command '$vlc' does not exist"
     exit 1
 fi
 
 for file in "$@"; do
     echo "=> Transcoding '$file'... "
 
     dst=`dirname "$file"`
     new=`basename "$file" | sed 's@\.[a-z][a-z][a-z]$@@'`.$mux
 
     $vlc -I dummy -q "$file" \
        --sout "#transcode{vcodec=mp4v,vb=1024,acodec=mp4a,ab=128}:standard{mux=mp4,dst=\"$dst/$new\",access=file}" \
        vlc://quit
     ls -lh "$file" "$dst/$new"
     echo
 done
