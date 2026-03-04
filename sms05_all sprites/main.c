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
#include "tiles_data.h"

#define TILEIDX_FISH_FRAME1 20
#define TILEIDX_FISH_FRAME2 TILEIDX_FISH_FRAME1 + 4
#define TILEIDX_BAT_FRAME1 152

#define MAX_METASPR 16

typedef struct Bat {
    u8 x;
    u8 y;
    int8_t spx;
    int8_t spy;
} Bat;

Bat bats[MAX_METASPR];
uint16_t p1 = 0;

inline u8 wrap(u8 value, u8 min, u8 max) {
    if (value > max) 
        return min;
    if (value < min) 
        return max;
    return value;
}

void init_bats(void) {
    j = 0;
    for (i = 0; i < MAX_METASPR; i++) {
        bats[i].x = rand() % (SCREEN_WIDTH-16);
        bats[i].y = rand() % (SCREEN_HEIGHT-16);

        bats[i].spx = rand() % 5 - 2;
        if (!bats[i].spx) bats[i].spx = 1;
        bats[i].spy = rand() % 5 - 2;
        
        set_metasprite_tiles(j, 0, 4);
        j += 4;
    }
}

void update_bat(Bat* bat) {
    // move bat
    bat->x += bat->spx;
    bat->y += bat->spy;

    // bounce off screen
    if (bat->x < 8) {
        bat->x = 8;
        bat->spx = -bat->spx;
    } 
    else
    if (bat->x > SCREEN_WIDTH-16) {
        bat->x = SCREEN_WIDTH-16;
        bat->spx = -bat->spx;
    }

    if (bat->y < 8) {
        bat->y = 8;
        bat->spy = -bat->spy;
    } 
    else
    if (bat->y > SCREEN_HEIGHT-16) {
        bat->y = SCREEN_HEIGHT-16;
        bat->spy = -bat->spy;
    }
}

void main(void) {
    u8 x = SCREEN_WIDTH/2-16, y = SCREEN_HEIGHT/2-16;
    u8 count = 10, frame = 0;

    HIDE_SPRITES;
    HIDE_BKG;

    set_tiledata(TILES1_BANK, TILES1_COUNT, tiles1_data);
    // palette 0 = background
    set_tilepal(0, 16, tiles1_palette); 

    set_sprite_4bpp_data(0, 16, sprites_data);
    // palette 1 = sprites
    set_tilepal(1, 16, sprites_palette); 
    
    init_bats();

    __WRITE_VDP_REG(VDP_R0, __READ_VDP_REG(VDP_R0) |= R0_LCB);     // left column blank
    
    DISPLAY_ON;
    SHOW_BKG;
    SHOW_SPRITES;

    while (1) {
        update();

        j = 0;
        for (i = 0; i < MAX_METASPR; i++) {
            // update_bat(&bats[i]);
            bats[i].x += bats[i].spx;
            bats[i].y += bats[i].spy;
            bats[i].x = wrap(bats[i].x, 2, SCREEN_WIDTH-2);
            bats[i].y = wrap(bats[i].y, 2, SCREEN_HEIGHT-2);

            set_metasprite_pos(j, bats[i].x, bats[i].y, 2, 2);
            j += 4;
        }

        scroll_bkg(1,1);

        if (!--count) {
            count = 10;
            frame = !frame? 4 : 0;
            j = 0;
            for (i = 0; i < MAX_METASPR; i++) {
                set_metasprite_tiles(j, frame, 4);
                j += 4;
            }
        }

        vsync();
    }
}