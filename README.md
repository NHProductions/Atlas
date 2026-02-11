# Atlas
Ti-84 CE Video pipeline &amp; player.


# Stats
Python side converts the .mp4 files into a collection of .bin files representing 4 frames each. These frames are then converted into TI-84 CE's appvar files, for the video player to play.
Along w/ the frames, there's also 2 other appvars, a PAT appvar, which has the palette for the video, & a DAT appvar, which has crucial metadata for the video.

*** IMPORTANT: The ffmpeg.exe was too big to upload, so, you have to download it and put it in the same directory as AtlasCompression.py for it to work ***


# VIDEO METADATA:
All videos metadata are stored within a 13 byte metadata file (On the calc, it shows as 29 bytes,I'm not entirely sure why, but I'm assuming it's cause of something w/ how Ti-84 CE stores them) , which is formatted as the following:
| Bytes 0-3  | Bytes 4-8 | Bytes 9-10 | Byte 11 | Bytes 12-13
| ------------- | ------------- | ------------- |------------- |------------- |
| "DAT" Header  | Name  | Frame Count | Frame Appvar Amount | Frame rate (in MS)

# VIDEO PLAYER:
I went with a kind of minimalist player, to use it just press up/down to select a video, and enter to play it. If there's no videos, it automatically quits. You can make a nicer looking one if you want, but UI/UX design rlly isn't my thing.

<details>
  <summary> [Examples: ] (click to expand) </summary>
Also these look way better on the actual calculator; the emulator makes them look a little strange at moments.
<video src=https://github.com/user-attachments/assets/685aa15f-0b50-4d2d-97f4-3ab5d87dffd8> </video>
(I'm sorry, I had to lol)
<video src=https://github.com/user-attachments/assets/7e6c6c5c-26da-4f03-9208-e5b17aa8f18c> </video>
</details>

