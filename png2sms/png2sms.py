'''
PNG to SMS 4bpp planar bytes

Usage:
    png2sms.py input.png new_name tile_width tile_height

'''

import sys
from PIL import Image

def round_color(c):
    return round(c / 85) * 85

def convert_to_planar(tile_pixels):
    planar_data = []
    for y in range(8):
        row = tile_pixels[y*8 : (y+1)*8]
        for plane in range(4):
            byte_value = 0
            for x in range(8):
                bit = (row[x] >> plane) & 0x01
                # Standard SMS/GG Bit Endianness
                if bit:
                    byte_value |= (1 << (7 - x))
            planar_data.append(byte_value)
    return planar_data

def main():
    if len(sys.argv) < 5:
        print("Usage: python png2sms.py input.png new_name tw th")
        return

    input_path, new_name = sys.argv[1], sys.argv[2]
    tw, th = int(sys.argv[3]), int(sys.argv[4])

    img = Image.open(input_path).convert('P')
    pixels = list(img.getdata())
    width, height = img.size

    # Palette Logic
    raw_pal = img.getpalette()
    sms_palette = []
    for i in range(16):
        if i == 0: # Force Index 0 to Green
            r, g, b = 0, 255, 0
        elif raw_pal and i*3+2 < len(raw_pal):
            r, g, b = [round_color(c) for c in raw_pal[i*3:i*3+3]]
        else:
            r, g, b = 0, 0, 0
        
        sms_color = ((b // 85) << 4) | ((g // 85) << 2) | (r // 85)
        sms_palette.append(sms_color)

    # Tile logic
    all_bytes = []
    for ty in range(th):
        for tx in range(tw):
            tile_px = []
            for y in range(8):
                for x in range(8):
                    px, py = tx*8+x, ty*8+y
                    tile_px.append(pixels[py*width+px] if px < width and py < height else 0)
            all_bytes.append(convert_to_planar(tile_px))

    # Write .h
    with open(f"{new_name}.h", "w") as f:
        f.write(f"#ifndef {new_name.upper()}_H\n#define {new_name.upper()}_H\n\n")
        f.write(f"#define {new_name.upper()}_TILES_COUNT {tw*th}\n\n")
        f.write(f"extern const uint8_t {new_name}_tiles[];\n")
        f.write(f"extern const uint8_t {new_name}_pal[];\n\n#endif")

    # Write .c
    with open(f"{new_name}.c", "w") as f:
        f.write(f'#include "{new_name}.h"\n\nconst uint8_t {new_name}_pal[] = {{\n')
        f.write(", ".join([f"0x{c:02X}" for c in sms_palette]) + "\n};\n\n")
        f.write(f"const uint8_t {new_name}_tiles[] = {{\n")
        for i, tile in enumerate(all_bytes):
            f.write(f"    // block {i}\n")
            for r in range(0, 32, 8):
                line = ", ".join([f"0x{b:02X}" for b in tile[r:r+8]])
                f.write(f"    {line}{',' if not (i == len(all_bytes)-1 and r == 24) else ''}\n")
        f.write("};")

if __name__ == "__main__":
    main()