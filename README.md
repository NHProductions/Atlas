# Atlas
Ti-84 CE Video pipeline &amp; player.


# Stats
Python side converts the .mp4 files into a collection of .bin files representing 4 frames each. These frames are then converted into TI-84 CE's appvar files, for the video player to play.
Along w/ the frames, there's also 2 other appvars, a PAT appvar, which has the palette for the video, & a DAT appvar, which has crucial metadata for the video.

# VIDEO METADATA:
All videos metadata are stored within a 13 byte metadata file (On the calc, it shows as 29 bytes,I'm not entirely sure why, but I'm assuming it's cause of something w/ how Ti-84 CE stores them) , which is formatted as the following:
| Bytes 0-3  | Bytes 4-8 | Bytes 9-10 | Byte 11 | Bytes 12-13
| ------------- | ------------- | ------------- |------------- |------------- |
| "DAT" Header  | Name  | Frame Count | Frame Appvar Amount | Frame rate (in MS)
