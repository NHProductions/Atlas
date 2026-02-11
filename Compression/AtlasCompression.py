from ctypes import sizeof
import re
import struct
import ffmpeg as ffmpeg
from PIL import Image
import io as io
import numpy as np
import subprocess
import os

# globals & Input
dir = os.path.dirname(os.path.realpath(__file__)) # dir of this script
filePath = input("Enter file path:") # mp4 file path.
vidName = ""
frameRate = input("Enter fps");
while True:
    try:
        vidName = input("Enter video name (max 5 characters, letters/numbers only):")
        if re.match(r"^[A-Za-z0-9]{1,5}$", vidName):
            break
        else:
            print("Invalid name. Please use only letters and numbers, max 8 characters.")
    except Exception as e:
        print("") # invalid input
ffmpeg_path = r'ffmpeg.exe' # fmmpeg executable path.
dims = (100, 80) # Dimensions on the TI-84 CE.
palette_path = os.path.join(dir, f"{vidName}PAL.png") # path to palette file.
binDir = "bins" # bins directory
frames_dir = os.path.join(dir, "frames") # frames directory.
os.makedirs(frames_dir, exist_ok=True)

# generates the palette file by using ffmpeg.
paletteCommand = [
    ffmpeg_path,
    "-i", filePath,
    "-y",
    "-vf", "palettegen=max_colors=256",
    palette_path
]
subprocess.run(paletteCommand, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
print("Palette generated.")

# --- Frame generation via indexed PNGs ---
print("Generating frames as indexed PNGs")
# Removes any stuff from frames, binDir, & ti_files.
for f in os.listdir(frames_dir):
    if f.endswith(".png"):
        os.remove(os.path.join(frames_dir, f))
for f in os.listdir(binDir):
    if f.endswith(".bin"):
        os.remove(os.path.join(binDir, f))
for f in os.listdir("ti_files"):
    os.remove(os.path.join("ti_files", f))
# Puts the frames in the frames directory.
imageCMD = [
    ffmpeg_path,
    "-i", filePath,
    "-i", palette_path,
    "-y",
    "-filter_complex",
    (
        f"[0:v]scale={dims[0]}:{dims[1]},fps=8,format=rgb24[v];"
        f"[v][1:v]paletteuse=dither=none"
    ),
    os.path.join(frames_dir, "frame_%05d.png")
]

subprocess.run(imageCMD, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)

# Converts frames into pallete index format.
print("RGB frames -> Palete index frames.")

frames = []

png_files = sorted(
    f for f in os.listdir(frames_dir) if f.endswith(".png")
)

for fname in png_files:
    img = Image.open(os.path.join(frames_dir, fname))

    if img.mode != "P":
        raise RuntimeError(f"{fname} is not indexed (mode={img.mode})")

    frame = np.array(img, dtype=np.uint8)

    if frame.shape != (dims[1], dims[0]):
        raise RuntimeError(f"Unexpected frame shape: {frame.shape}")

    frames.append(frame)

print(f"Frames generated: {len(frames)}")

frameBytes = [frame.tobytes() for frame in frames] # could do some compression, but I don't want to go through that & it works good enough for short videos.



os.makedirs(binDir, exist_ok=True)
os.makedirs("ti_files", exist_ok=True)
framesPerBin = 4 # Amount of frames for each bin file. 
for i in range(0, len(frameBytes), framesPerBin):
    chunk = frameBytes[i:i+framesPerBin]
    with open(f"bins/{vidName}{i//framesPerBin:03}.bin","wb") as f:
        f.write(struct.pack("<H", len(chunk)))  # number of frames
        for frame in chunk:
            f.write(struct.pack("<I", len(frame)))
            f.write(frame)

def bin_to_ti_files(bin_dir,out_dir,convbin_path,output_type="8xv" ):
    # converts all bin files into .8xv files using convbin.exe (Ti-84 CE C Toolchain)
    os.makedirs(out_dir, exist_ok=True)

    for fname in sorted(os.listdir(bin_dir)):
        if not fname.lower().endswith(".bin"):
            continue

        bin_path = os.path.join(bin_dir, fname)

        # Create TI-safe name
        name = os.path.splitext(fname)[0]
        name = re.sub(r"[^A-Za-z0-9]", "", name).upper()
        name = name[:8]

        if not name:
            raise ValueError(f"Invalid TI name derived from {fname}")

        out_path = os.path.join(out_dir, f"{name}.{output_type}")

        cmd = [
            convbin_path,
            "-i", bin_path,
            "-j", "bin",
            "-k", output_type,
            "-o", out_path,
            "-n", name
        ]

        print(f"Converting {fname} -> {out_path}")
        subprocess.run(cmd, check=True)

    print("All files converted.")
# Converts color to correct format for the TI-84 CE.
def rgb888_to_ti555(r, g, b):
    r5 = (r >> 3) & 0x1F
    g6 = (g >> 3) & 0x1F
    b5 = (b >> 3) & 0x1F
    return (r5 << 10) | (g6 << 5) | b5


img = Image.open(palette_path).convert("RGB") 
data = list(img.getdata())[:256]

os.makedirs('bins', exist_ok=True)
with open(os.path.join('bins', f"{vidName}PAL.bin"), "wb") as f:
    for r, g, b in data:
        val = rgb888_to_ti555(r, g, b)
        f.write(struct.pack("<H", val))

with open(os.path.join('bins', f"{vidName}DAT.bin"), "wb") as f:
    name_bytes = vidName.encode('ascii')[:5].ljust(5, b'\x00')
    frame_count = np.uint16(len(frames))
    frame_bin_amount = (frame_count + framesPerBin - 1) // framesPerBin
    frame_rate = 1000//int(frameRate)  # 8 fps = 125 out of 1000
    f.write("DAT".encode('ascii')) # 3 bytes
    f.write(name_bytes) # 5 bytes
    f.write(struct.pack("<H", frame_count)) # 2
    f.write(struct.pack("<B", frame_bin_amount)) # 1 
    f.write(struct.pack("<H", frame_rate)) # 2
    

bin_to_ti_files(
    bin_dir="bins",
    out_dir="ti_files",
    convbin_path=r"convbin.exe",
    output_type="8xv")