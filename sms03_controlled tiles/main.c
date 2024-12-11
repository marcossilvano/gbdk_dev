/**
 * SMS GBDK + Emulicious
 * 
 * Compile: Ctrl + Shift + B
 * Run: F5
 * Debug: launch.json -> "stopOnEntry": true
 */

#include <sms/sms.h>
#include <stdio.h>
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

inline void restore_bg(u8 x, u8 y) {
    set_tile(x, y, 0);
    set_tile(x+1, y, 0);
    set_tile(x, y+1, 0);
    set_tile(x+1, y+1, 0);
}

void main(void) {
    u8 x = SCREEN_TILES_W/2, y = SCREEN_TILES_H/2;
    u8 count = 0, frame = TILEIDX_BAT_FRAME1, delay = 10;


    set_tiledata(TILES1_BANK, TILES1_COUNT, tiles1_data);
    // tile 0 is always for background and tiles 1, for sprites
    set_tilepal(0, 16, tiles1_palette); 

    DISPLAY_ON;
    SHOW_BKG;

    set_tile(0, 0, 1);
    set_tile2x2(5, 5, TILEIDX_FISH_FRAME1);
    set_tile2x2(7, 7, TILEIDX_FISH_FRAME2);
    set_tile2x2(x, y, TILEIDX_BAT_FRAME1);

    while (1) {
        update();

        if (!--delay) {
            delay = 3;
            restore_bg(x, y);

            x += key_down(JOY_P1_RIGHT) - key_down(JOY_P1_LEFT);
            y += key_down(JOY_P1_DOWN)  - key_down(JOY_P1_UP);

            if (key_pressed(JOY_P1_MD_A))
                clrscreen();

            x = wrap(x, 1, SCREEN_TILES_W-1);
            y = wrap(y, 1, SCREEN_TILES_H-1);

        }
        if (count++ > 10) {
            count = 0;
            frame = frame == TILEIDX_BAT_FRAME1? TILEIDX_BAT_FRAME1+4 : TILEIDX_BAT_FRAME1;
        }
        set_tile2x2(x, y, frame);

        vsync();
    }
}