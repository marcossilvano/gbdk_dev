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
#include <gbdk/emu_debug.h>
#include "utils.h"
#include "tiles_data.h"

#define TILEIDX_FISH_FRAME1 20
#define TILEIDX_FISH_FRAME2 TILEIDX_FISH_FRAME1 + 4
#define TILEIDX_BAT_FRAME1 152

#define MAX_METASPR 10

typedef struct Bat {
    uint8_t x;
    uint8_t y;
    int8_t spx;
    int8_t spy;
} Bat;

Bat bats[MAX_METASPR];
uint16_t p1 = 0;
Bat* bat;
uint8_t count = 10, frame = 0;

inline uint8_t wrap(uint8_t value, uint8_t min, uint8_t max) {
    if (value > max) 
        return min;
    if (value < min) 
        return max;
    return value;
}

void init_bats(void) {
    j = 0;
    bat = bats;
    for (i = 0; i < MAX_METASPR; i++) {
        bat->x = rand() % (SCREEN_WIDTH-16);
        bat->y = rand() % (SCREEN_HEIGHT-16);

        bat->spx = rand() % 5 - 2;
        if (!bat->spx) bat->spx = 1;
        bat->spy = rand() % 5 - 2;
        
        set_metasprite_tiles(j, 0, 4);
        j += 4;
        bat++;
    }
}

void lcd_interrupt(void) {
    // if (VDP_STATUS & STATF_INT_VBL == 0) {
    // }
    // EMU_printf("VCOUNTER %d\n", VCOUNTER);

    if (VCOUNTER < 42u) {
        __WRITE_VDP_REG(VDP_RSCX, p1 >> 2);
    } else
    // if (VCOUNTER > 40u && VCOUNTER < 85u) {
    //     __WRITE_VDP_REG(VDP_RSCX, p1 >> 1);
    // } else
    if (VCOUNTER > 80) {
        __WRITE_VDP_REG(VDP_RSCX, p1);
    }
}

inline void init_line_interrupts() {
    disable_interrupts();
    CRITICAL {
        // __WRITE_VDP_REG(VDP_R0, __READ_VDP_REG(VDP_R0) |= R0_IE1);
        __WRITE_VDP_REG(VDP_R1, __READ_VDP_REG(VDP_R1) = R1_IE_OFF);
        __WRITE_VDP_REG(VDP_R10, 40);
        add_LCD(lcd_interrupt);
        // add_VBL(...)
    }
    set_interrupts(LCD_IFLAG); // VBL_IFLAG);
    enable_interrupts();
}

void main(void) {
    HIDE_SPRITES;
    HIDE_BKG;

    set_tiledata(TILES1_BANK, TILES1_COUNT, tiles1_data);
    set_tilepal(0, 16, tiles1_palette); // palette 0 = background

    set_sprite_4bpp_data(0, 16, sprites_data);
    set_tilepal(1, 16, sprites_palette); // palette 1 = sprites
    
    init_bats();

    init_line_interrupts();

    // LEFT COLUMN BLANK
    __WRITE_VDP_REG(VDP_R0, __READ_VDP_REG(VDP_R0) |= R0_LCB);
    // __WRITE_VDP_REG(VDP_R1, __READ_VDP_REG(VDP_R1) |= R1_SPR_8X16);

    DISPLAY_ON;
    SHOW_BKG;
    SHOW_SPRITES;

    while (true) {
        vsync();
        __WRITE_VDP_REG(VDP_R0, __READ_VDP_REG(VDP_R0) &= ~R0_IE1);

        // EMU_TEXT("loop");

        update_keys();      
        // scroll_bkg(-1,0);
        --count;
        p1++;

        j = 0;
        bat = bats;
        for (i = 0; i < MAX_METASPR; i++) {
            bat->x += bat->spx;
            bat->y += bat->spy;
            bat->x = wrap(bat->x, 8, SCREEN_WIDTH-8);
            bat->y = wrap(bat->y, 8, SCREEN_HEIGHT-8);

            set_metasprite_pos4x4(j, bat->x, bat->y);

            if (!count) {
                set_metasprite_tiles4x4(j, frame);
            }
            j += 4;
            bat++;
        }

        // __WRITE_VDP_REG(VDP_RSCX, __READ_VDP_REG(VDP_RSCX) + 1);

        if (!count) {
            count = 10;
            frame = !frame? 4 : 0;
            // j = 0;
            // for (i = 0; i < MAX_METASPR; i++) {
            //     set_metasprite_tiles4x4(j, frame);
            //     j += 4;
            // }
        }
        __WRITE_VDP_REG(VDP_R0, __READ_VDP_REG(VDP_R0) |= R0_IE1);
    }
}