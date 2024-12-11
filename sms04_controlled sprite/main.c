/**
 * SMS GBDK + Emulicious
 * 
 * Compile: Ctrl + Shift + B
 * Run: F5
 * Debug: launch.json -> "stopOnEntry": true
 */

#include <sms/sms.h>
#include <stdio.h>
#include <stdlib.h>
#include "utils.h"
#include "tiles_data.h"

#define TILEIDX_FISH_FRAME1 20
#define TILEIDX_FISH_FRAME2 TILEIDX_FISH_FRAME1 + 4
#define TILEIDX_BAT_FRAME1 152

inline u8 wrap(u8 value, u8 min, u8 max) {
    if (value > max) 
        return min;
    if (value < min) 
        return max;
    return value;
}

void main(void) {
    u8 x = SCREEN_WIDTH/2-16, y = SCREEN_HEIGHT/2-16;
    u8 count = 10, frame = 0;

    set_tiledata(TILES1_BANK, TILES1_COUNT, tiles1_data);
    // palette 0 = background
    set_tilepal(0, 16, tiles1_palette); 

    set_sprite_4bpp_data(0, 16, sprites_data);
    // palette 1 = sprites
    set_tilepal(1, 16, sprites_palette); 
    
    set_metasprite_tiles(0, 0, 4);

    DISPLAY_ON;
    SHOW_BKG;

    while (1) {
        update();

        x += (key_down(JOY_P1_RIGHT) - key_down(JOY_P1_LEFT)) << 1;
        y += (key_down(JOY_P1_DOWN)  - key_down(JOY_P1_UP)) << 1;

        x = wrap(x, 2, SCREEN_WIDTH-8);
        y = wrap(y, 2, SCREEN_HEIGHT-8);

        set_metasprite_pos(0, x, y, 2, 2);

        if (!--count) {
            count = 10;
            frame = !frame? 4 : 0;
            set_metasprite_tiles(0, frame, 4);
        }

        vsync();
    }
}