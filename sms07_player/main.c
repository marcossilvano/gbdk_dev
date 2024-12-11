/**
 * SMS GBDK + Emulicious
 * 
 * Compile: Ctrl + Shift + B
 * Run: F5
 * Debug: launch.json -> "stopOnEntry": true
 */

#include <sms/sms.h>
#include <sms/hardware.h>
#include <stdio.h>
#include <rand.h>
#include "utils.h"
#include "tiles_map1.h"
#include "spr_player.h"
#include "map.h"
#include "gameobject.h"

#define TILEIDX_FISH_FRAME1 20
#define TILEIDX_FISH_FRAME2 TILEIDX_FISH_FRAME1 + 4
#define TILEIDX_BAT_FRAME1 152

#define MAX_METASPR 16

GameObject player;
uint16_t p1 = 0;

inline u8 wrap(u8 value, u8 min, u8 max) {
    if (value > max) 
        return min;
    if (value < min) 
        return max;
    return value;
}

inline void init_player(void) {
    player.x = 2 << 4;
    player.y = 24 << 4;
    player.nextx = player.x;
    player.nexty = player.y;
    player.width = 16;
    player.height = 32;
    player.tile_start = 8;
    player.tile_count = 4;
    player.sprite_id = 0;
    // set_metasprite_tiles4x4(0, player.tile_start);
    set_metasprite_tiles4x8x16(player.sprite_id, player.tile_start);
}

inline bool check_tile_collision(GameObject* obj) {
    if (map_check_obj(obj)) {
        //                color 0b--BBGGRR
        set_palette_entry(0, 1, 0b00000011);
        set_metasprite_tiles(player.sprite_id, obj->tile_start+4, 4);
        return true;
    } else {
        set_palette_entry(0, 1, tiles1_palette[1]);
        set_metasprite_tiles(player.sprite_id, obj->tile_start, 4);
        return false;
    }
}

inline bool move_and_slide(GameObject* obj) {
    set_palette_entry(0, 1, tiles1_palette[1]);

    n = (obj->y + obj->height) >> 4;
    p = (obj->y + (obj->height >> 1)) >> 4;

    if (obj->spdx > 0) {      // moving right
        m = (obj->nextx + obj->width -1) >> 4;
        if (map[(obj->y+1) >> 4][m] || map[p][m] || map[n][m]) {
            obj->nextx = (m << 4) - obj->width;
            set_palette_entry(0, 1, 0b00000011);
        }
    }
    else if (obj->spdx < 0) { // moving left
        m = obj->nextx >> 4;
        if (map[(obj->y+1) >> 4][m] || map[p][m] || map[n][m]) {
            obj->nextx = (m + 1) << 4;
            set_palette_entry(0, 1, 0b00000011);
        }
    }

    m = (obj->nextx + obj->width -1) >> 4; // update next x position

    if (obj->spdy > 0) {      // moving down
        n = (obj->nexty + obj->height) >> 4;
        if (map[n][obj->nextx >> 4] || map[n][m]) {
            obj->nexty = (n << 4) - obj->height - 1;
            set_palette_entry(0, 1, 0b00000011);
        }
    }
    else if (obj->spdy < 0) { // moving up
        n = (obj->nexty+1) >> 4;
        if (map[n][obj->nextx >> 4] || map[n][m]) {
            obj->nexty = ((n + 1) << 4) - 1;
            set_palette_entry(0, 1, 0b00000011);
        }
    }
}

/*
inline bool move_and_slide(GameObject* obj) {
    set_palette_entry(0, 1, tiles1_palette[1]);

    n = (obj->y + obj->height) >> 4;

    if (obj->spdx > 0) {      // moving right
        m = (obj->nextx + obj->width -1) >> 4;
        if (map[(obj->y+1) >> 4][m] || map[n][m]) {
            obj->nextx = (m << 4) - obj->width;
            set_palette_entry(0, 1, 0b00000011);
        }
    }
    else if (obj->spdx < 0) { // moving left
        m = obj->nextx >> 4;
        if (map[(obj->y+1) >> 4][m] || map[n][m]) {
            obj->nextx = (m << 4) + obj->width;
            set_palette_entry(0, 1, 0b00000011);
        }
    }

    m = (obj->nextx + obj->width -1) >> 4; // update next x position

    if (obj->spdy > 0) {      // moving down
        n = (obj->nexty + obj->height) >> 4;
        if (map[n][obj->nextx >> 4] || map[n][m]) {
            obj->nexty = (n << 4) - obj->height - 1;
            set_palette_entry(0, 1, 0b00000011);
        }
    }
    else if (obj->spdy < 0) { // moving up
        n = (obj->nexty+1) >> 4;
        if (map[n][obj->nextx >> 4] || map[n][m]) {
            obj->nexty = (n << 4) + obj->height - 1;
            set_palette_entry(0, 1, 0b00000011);
        }
    }
}
*/

void main(void) {
    u8 x = SCREEN_WIDTH/2-16, y = SCREEN_HEIGHT/2-16;
    u8 count = 1, frame = 0;

    set_tiledata(TILES1_BANK, TILES1_COUNT, tiles1_data);
    // palette 0 = background
    set_tilepal(0, 16, tiles1_palette); 

    set_sprite_4bpp_data(0, 16, spr_player_data);
    // palette 1 = sprites
    set_tilepal(1, 16, spr_player_palette); 
    
    init_player();
    map_setlevel();

    
    __WRITE_VDP_REG(VDP_R0, __READ_VDP_REG(VDP_R0) |= R0_LCB);     // left column blank
    __WRITE_VDP_REG(VDP_R1, __READ_VDP_REG(VDP_R1) |= R1_SPR_8X16);// 8x16 sprites mode

    DISPLAY_ON;
    SHOW_BKG;
    SHOW_SPRITES;

    u8 moved = 0;
    set_metasprite_pos4x8x16(player.sprite_id, player.x, player.y);

    while (1) {
        update();

        // if (key_down(JOY_P1_LEFT)) {
        //     player.spdx = -1;
        // } else if (key_down(JOY_P1_RIGHT)) {
        //     player.spdx = 1;
        // } else {
        //     moved = 0;
        //     set_metasprite_tiles(0, player.tile_start, 4);
        // }

        if (key_down(JOY_P1_MD_A)) {
            player.spdx = key_pressed(JOY_P1_RIGHT) - key_pressed(JOY_P1_LEFT);
            player.spdy = key_pressed(JOY_P1_DOWN) - key_pressed(JOY_P1_UP);             
        } else {
            player.spdx = key_down(JOY_P1_RIGHT) - key_down(JOY_P1_LEFT);
            player.spdy = key_down(JOY_P1_DOWN) - key_down(JOY_P1_UP);
        }
        
        if (player.spdx) {
            if (!--count) {
                count = 7;
                frame = frame == player.tile_start? player.tile_start-8 : player.tile_start;
                set_metasprite_tiles4x8x16(player.sprite_id, frame);
            }
        }

        // check_tile_collision();
        player.nextx += player.spdx;
        player.nexty += player.spdy;
        
        move_and_slide(&player);

        player.x = player.nextx;
        player.y = player.nexty;

        set_metasprite_pos4x8x16(player.sprite_id, player.x, player.y);

        // scroll_bkg(1,1);
        vsync();
    }
}