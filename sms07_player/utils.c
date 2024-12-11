#include "utils.h"

typedef unsigned char u8;
typedef unsigned short u16;

u8 buttons;
u8 buttons_old;

u8 i, j, k;      // general for-loops
u8 m, n, p;   // macro single and nested for-loops

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

//////////////////////////////////////////////////////////////////////////
// METASPRITES

void set_metasprite_tiles(u8 sprite_id, u8 tile_start, u8 number_of_tiles) {
    for (m = 0; m < number_of_tiles; m++) {
        set_sprite_tile(sprite_id + m, tile_start + m);
        // set_sprite_prop(sprite + m, tile_prop[m]);
    }
}

inline void set_metasprite_tiles4x4(u8 sprite_id, u8 tile_start) {
    set_sprite_tile(sprite_id, tile_start);
    sprite_id++; tile_start++;
    set_sprite_tile(sprite_id, tile_start);
    sprite_id++; tile_start++;
    set_sprite_tile(sprite_id, tile_start);
    sprite_id++; tile_start++;
    set_sprite_tile(sprite_id, tile_start);
}

inline void set_metasprite_tiles4x8x16(u8 sprite_id, u8 tile_start) {
    set_sprite_tile(sprite_id, tile_start);
    sprite_id++; tile_start+=2u;
    set_sprite_tile(sprite_id, tile_start);
    sprite_id++; tile_start+=2u;
    set_sprite_tile(sprite_id, tile_start);
    sprite_id++; tile_start+=2u;
    set_sprite_tile(sprite_id, tile_start);
}

void set_metasprite_pos(u8 sprite_id, u8 x, u8 y, u8 tiles_w, u8 tiles_h) {
    uint8_t xoffset = x;                            
    uint8_t yoffset = y;                            
    for (m = 0; m < tiles_w; m++) {                 
        for (n = 0; n < tiles_h; n++) {             
            move_sprite(sprite_id, xoffset, yoffset);     
            sprite_id++;                                  
            xoffset += 8u;                          
        }                                           
        xoffset = x;                                
        yoffset += 8u;                              
    }
}

inline void set_metasprite_pos4x4 (u8 sprite_id, u8 x, u8 y) {
    move_sprite(sprite_id, x, y);     
    sprite_id++;
    move_sprite(sprite_id, x+8u, y);     
    sprite_id++; y += 8u;
    move_sprite(sprite_id, x, y);     
    sprite_id++;
    move_sprite(sprite_id, x+8u, y); 
}

inline void set_metasprite_pos4x8x16 (u8 sprite_id, u8 x, u8 y) {
    move_sprite(sprite_id, x, y);     
    sprite_id++; n = y + 16u;
    move_sprite(sprite_id, x, n);
    sprite_id++; x += 8u;
    move_sprite(sprite_id, x, y);
    sprite_id++;;
    move_sprite(sprite_id, x, n);
}

//////////////////////////////////////////////////////////////////////////
// GENERAL

bool is_bitset(u8 value, u8 bit) {
    return ((value & bit) == bit);
}

void update(void) {
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