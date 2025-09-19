#include "utils.h"

typedef unsigned char u8;
typedef unsigned short u16;

u8 buttons;
u8 buttons_old;

void clrscreen(void) {
    fill_bkg_rect(0, 0, SCREEN_TILES_W, SCREEN_TILES_H, 0); // 32x24 tiles
}

void set_tiledata(u8 tilestart, u8 tilecount, const u8* data) {
    set_bkg_4bpp_data(tilestart, tilecount, data);
}

void set_tilepal(u8 palstart, u8 colorcount, const u8* data) {
    palstart &= 0x01; // bit mask
    u8 i = 0;
    for (i = 0; i < colorcount; i++) {
        set_palette_entry(palstart, i, data[i]);
    }
}

void set_tile(u8 x, u8 y, u8 tile) {
    set_attributed_tile_xy(x, y, tile);
}

// sets 16x16 metatile (4 tiles length)
void set_tile2x2(u8 x, u8 y, u8 tilestart) {
    set_attributed_tile_xy(x, y, tilestart);    // top left
    tilestart++;
    set_attributed_tile_xy(x+1, y, tilestart);  // top right
    tilestart++;
    y++;
    set_attributed_tile_xy(x, y, tilestart);  // bottom left
    tilestart++;
    set_attributed_tile_xy(x+1, y, tilestart);// bottom right
}

bool is_bitset(u8 value, u8 bit) {
    return ((value & bit) == bit);
}

void key_update(void) {
    // update joypad input
    buttons_old = buttons;
    buttons = joypad();
}

bool key_down(u8 key) {
    return is_bitset(buttons, key);
}

bool key_pressed(u8 key) {
    return is_bitset(buttons, key) && !is_bitset(buttons_old, key);
}

bool key_released(u8 key) {
    return !is_bitset(buttons, key) && is_bitset(buttons_old, key);
}