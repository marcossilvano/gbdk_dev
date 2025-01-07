import os
from io import TextIOWrapper
from PIL import Image
from PIL import ImageFile

log = None

def open_and_check(filepath: str, frame_width: int, frame_height: int):
    image: ImageFile = None
    try:
        image = Image.open(filepath)
    except:
        log("[ERROR]", "Could not load image: %s" % filepath)
        return [False, image]

    if image.width % 8 != 0 or image.height % 8 != 0:
        log("[ERROR]", "Image must have dimensions multiple of 8: %s" % filepath)
        return [False, image]

    if image.width % frame_width != 0 or image.height % frame_height != 0:
        log("[ERROR]", "Image must have dimensions multiple of frame width and height: %s" % filepath)
        return [False, image]

    if not image.palette:
        log("[ERROR]", 'Image must have indexed color mode: %s' % filepath)
        return [False, image]

    log("[INFO]", 'Found %d colors in: %s' % (len(image.palette.colors), filepath))
    if len(image.palette.colors) != 16:
        log("[ERROR]", 'Palette must have 16 colors: %s' % filepath)
        return [False, image]
    
    return True, image


def create_array_file(destfolder: str, new_name: str) -> TextIOWrapper:
    destpath: str = destfolder + '/' + new_name
    # print(destpath)

    file: TextIOWrapper = open(destpath, 'w')
    if not file:
         log('[ERROR] could not create array file: %s', destpath)
    
    return file


def extract_filename(filepath: str) -> str:
    return os.path.splitext(os.path.basename(filepath))[0]


def build_str_from_data(data: list[int], break_at: int) -> str:
    palette_str: str = ''
    for i in range(len(data)):
        if (i) % (break_at) == 0:
            palette_str += '\n'
        palette_str += data[i] + ','

    return palette_str


def is_multiple_of_85(color: tuple[int, int, int]) -> bool:
    return not(color[0] % 85 != 0 or color[1] % 85 != 0 or color[2] % 85 != 0)


def write_palette(file_c: TextIOWrapper, palette: list, obj_name: str, number_of_tiles: int) -> bool:   
    content: str = '''
#include "%s.h"

const uint8_t %s_number_of_tiles = %d;

// 16 color palette
const uint8_t const %s_palette[] =
{%s
};

'''
    # PALETTE FORMAT: 00BBGGRR (2 bits per color)
    # 00 = 0
    # 01 = 85       << increments of 85
    # 10 = 170
    # 11 = 255

    RED  = 0
    GREEN= 1
    BLUE = 2
    sms_palette: list[str] = []
    for color in palette:
        if not is_multiple_of_85(color):
            log('[ERROR]', 'Color %d: RGB must be multiple of 85' % color)        
            return False

        BBGGRR: int = 0
        BBGGRR += (color[BLUE] // 85) << 4
        BBGGRR += (color[GREEN]// 85) << 2
        BBGGRR += (color[RED]  // 85)
        sms_palette.append('0x%02X' % BBGGRR)

    palette_str: str = build_str_from_data(sms_palette, 8)
    content = content % (
        obj_name,                   # include .h
        obj_name, number_of_tiles,  # number of tiles
        obj_name, palette_str[:-1]  # palette array
        )
    try:
        file_c.write(content)
    except:
        log('[ERROR]', 'Cannot write header file')
        return False
    
    # idx: int = 1
    # for color in palette:
    #     print(idx, color)
    #     idx += 1
    return True


def get_tile_data(pixels, pos_x: int, pos_y: int):
    print("Tile data:")
    sms_pixels: list[str] = []
    for y in range(pos_y, pos_y+8):
        for shift in range(4):
            px: int = 0
            for i in range(8):
                print((pixels[pos_x+i,y]) & (0x01 << shift), end='')
                bit = ((pixels[pos_x+i,y]) & (0x01 << shift)) != 0
                px = px | (bit << (7-i))
            sms_pixels.append('0x%02X' % px)
            print(' ', end='')
            # print(sms_pixels[-1], end=' ')
        print()

    return sms_pixels



def write_tiles(file_c: TextIOWrapper, image: ImageFile, obj_name: str, 
                frame_width: int, frame_height: int, mode8x16: bool) -> bool:   
    content: str = '''
const uint8_t const %s_data[] =
{%s
};
'''
    pixels = image.load()
    data_str: str = ''
    for frm_y in range(image.height//frame_height):
        for frm_x in range(image.width//frame_width):
            pos_x = frm_x*frame_width
            pos_y = frm_y*frame_height

            sms_pixels: list[str] = get_tile_data(pixels, pos_y, pos_x)

            data_str += build_str_from_data(sms_pixels, frame_width//2) + '\n'
            sms_pixels = []
    
    print("\nImage pixels: ")
    for y in range(pos_y, pos_y+8):
        for x in range(pos_x, pos_x+8):
            print('%02X' % (pixels[x,y]), end=', ')
        print()
    print()

    content = content % (obj_name, data_str[:-2])
    try:
        file_c.write(content)
    except:
        log('[ERROR]', 'Cannot write c file')
        return False    

    # pixels = image.load()
    # for y in range(image.height):
    #     for x in range(image.width):
    #         pixels[x,y]
    #     print()
    return True


def write_header_file(file_h: TextIOWrapper, obj_name: str) -> bool:
    content: str = '''
#ifndef TILES_%s_H
#define TILES_%s_H

#include <sms/sms.h>

extern const uint8_t %s_number_of_tiles;

extern const uint8_t const %s_palette[];
extern const uint8_t const %s_data[];

#endif'''

    try:
        file_h.write(content % (
            obj_name.upper(), obj_name.upper(), # include guard
            obj_name,                           # number of tiles
            obj_name, obj_name))                # variable names
    except:
        log('[ERROR]', 'Cannot write header file')
        return False
    
    return True

def png_to_array(destfolder: str, filepath: str, frame_width: int, 
                 frame_height: int, mode8x16: bool, log_func) -> bool:
    global log
    log = log_func

    # initial checkings
    status, image = open_and_check(filepath, frame_width, frame_height)
    if not status:
        return False
    
    new_name: str = extract_filename(filepath)
    new_path: str = destfolder + '/' + new_name
        
    file_h: TextIOWrapper = create_array_file(destfolder, new_name + '.h')
    number_of_tiles: int = image.width/8 * image.height/8
    if not write_header_file(file_h, new_name):
        return False
    
    log('[INFO]', 'Header file generated: \'%s.h\'' % (new_path))
    file_h.close()

    file_c: TextIOWrapper = create_array_file(destfolder, new_name + '.c')
    if not write_palette(file_c, image.palette.colors.keys(), new_name, number_of_tiles):
        return False
    
    log('[INFO]', '16 colors written into: \'%s.c\'' % new_path)

    if not write_tiles(file_c, image, new_name, frame_width, frame_height, mode8x16):
        return False

    log('[INFO]', '%d tiles written into: \'%s.c\'' % (image.width/8 * image.height/8, new_path))
    file_c.close()

    log('[INFO]', "Image successfully converted\n")
    image.close()
    return True