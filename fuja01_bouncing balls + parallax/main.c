/**
 * SMS GBDK + Emulicious
 * 
 * Compile: Ctrl + Shift + B
 * Run: F5
 * Debug: launch.json -> "stopOnEntry": true
 */

#include <sms/sms.h>
#include <sms/hardware.h>
#include <gbdk/console.h>
#include <stdio.h>
#include <rand.h>
#include <gbdk/emu_debug.h>
#include <gbdk/font.h>

#include "utils.h"
#include "tiles_data.h"

#define BAT_FRAME_DELAY 10
#define BAT_TILE_IDX 16
#define METATILE 4

#define MAX_METASPR 16 // update loop stops at 0

typedef struct Bat {
    u8 idx;
    u8 x;
    u8 y;
    int8_t spx;
    int8_t spy;
    u8 tile_index;
    u8 frame_current;
    u8 frame_delay;
    u8 frame_total;
    u8 frame_count;
} Bat;

Bat bats[MAX_METASPR];
uint16_t p1 = 0;
uint8_t tile_idx = 0;   // keep track of next available tile idx in VRAM

// PARALLAX SCROLLIING

#define SCROLL_POS 15u
#define SCROLL_HEIGHT 1u
#define SCROLL_PIX_HEIGHT (SCROLL_HEIGHT * 8u)
#define SCROLL_POS_PIX_START (((SCROLL_POS + DEVICE_SCREEN_Y_OFFSET) * 8u) - 2u)
#define SCROLL_POS_PIX_END (((SCROLL_POS + DEVICE_SCREEN_Y_OFFSET) * 8u) + SCROLL_PIX_HEIGHT - 1u)

#define SCROLL_SIZE 4
uint8_t scroll_spd[] = {0x04, 0x03, 0x02, 0x01};
uint8_t scroll_pos[] = {0, 0, 0, 0};
uint8_t scroll_idx = 1;
uint8_t scroller_x = 0;

uint8_t scroll_halfscanline = 0;

uint8_t *p_scroll_pos = scroll_pos + 1;
uint8_t *p_scroll_spd = scroll_spd + 1;

inline void do_scroll(uint8_t x, uint8_t y) {
    y;
    __WRITE_VDP_REG_UNSAFE(VDP_RSCX, -x);    
}

uint8_t LYC_REG = 0;  // define the fake LYC_REG, we will use it as the interrupt routine state

// [CS] The importance of console logging functions to discover what is going on in your code.
void vblank_isr(void) {
    __WRITE_VDP_REG_UNSAFE(VDP_R10, 15);
    do_scroll(0, 0);
    // LYC_REG = scroll_offset[scroll_idx];
    scroll_idx = 1;

    // p_scroll_pos = scroll_pos + 1;
    // p_scroll_spd = scroll_spd + 1;

    EMU_printf("VBLANK! VBLANK! VBLANK!\n");
}

// [CS] How to run a code at a specific event? Interruptions
// [CS] Why do we need to modify screen during refresh? Raster Effects
// [CS] Hot to optimize access to arrays? Pointers. What are pointers?
void scanline_isr(void) {
    EMU_printf("VCOUNTER = %u\n", VCOUNTER);
    /*
VCOUNTER = 16
VCOUNTER = 31
VCOUNTER = 47
VCOUNTER = 63
VCOUNTER = 79
VCOUNTER = 95
VCOUNTER = 111
VCOUNTER = 127
VCOUNTER = 144
VCOUNTER = 159
VCOUNTER = 175
VCOUNTER = 191
    */
//    scroll_pos[0] += scroll_spd[0];
//    do_scroll(scroll_pos[0] >> 4, 0);

    switch ((uint8_t)VCOUNTER) {    
    // top clouds
    case 16u:
        scroll_pos[0] += scroll_spd[0];
        do_scroll(scroll_pos[0] >> 4, 0);
        break;
    
    case 32u:
        scroll_pos[1] += scroll_spd[1];
        do_scroll(scroll_pos[1] >> 4, 0);
        break;

    case 48u:
        scroll_pos[2] += scroll_spd[2];
        do_scroll(scroll_pos[2] >> 4, 0);
        break;

    case 64u:
        scroll_pos[3] += scroll_spd[3];
        do_scroll(scroll_pos[3] >> 4, 0);
        break;

    // bottom clouds
    case 112u:
        scroll_pos[3] += scroll_spd[3];
        do_scroll(scroll_pos[3] >> 4, 0);
        break;

    case 128u:
        scroll_pos[2] += scroll_spd[2];
        do_scroll(scroll_pos[2] >> 4, 0);
        break;

    case 144u:
        scroll_pos[1] += scroll_spd[1];
        do_scroll(scroll_pos[1] >> 4, 0);
        break;

    case 160u:
        scroll_pos[0] += scroll_spd[0];
        do_scroll(scroll_pos[0] >> 4, 0);
        break;

    default:
        do_scroll(0, 0);
    }
}

// UTILITY FUNCTIONS

inline u8 wrap(u8 value, u8 min, u8 max) {
    if (value > max) 
        return min;
    if (value < min) 
        return max;
    return value;
}

void init_bats(void) {
    // [CS] Why do we need to set a tile index?
    j = 0;
    for (i = 0; i < MAX_METASPR; i++) {
        Bat* bat = &bats[i];
        bat->x = rand() % (SCREEN_WIDTH-16);
        bat->y = rand() % (SCREEN_HEIGHT-16);

        bat->spx = rand() % 3 - 1;
        if (!bat->spx) bat->spx = 1;
        bat->spy = rand() % 3 - 1;
        
        bat->tile_index = BAT_TILE_IDX;
        bat->frame_delay = i % BAT_FRAME_DELAY + 1; // count = 10
        bat->frame_current = 0;
        bat->frame_total = 2;
        bat->frame_count = bat->frame_total;
        bat->idx = j;

        set_metasprite2x2_tiles(bat->idx, bat->tile_index + bat->frame_current);
        j += METATILE;
    }
}

// [CS] Why using pointer? Struct copy vs pointer
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
        bat->x = (u8)(SCREEN_WIDTH-16);
        bat->spx = -bat->spx;
    }

    if (bat->y < 4) {
        bat->y = 4;
        bat->spy = -bat->spy;
    } 
    else
    if (bat->y > SCREEN_HEIGHT-16) {
        bat->y = (u8)(SCREEN_HEIGHT-16);
        bat->spy = -bat->spy;
    }

    set_metasprite2x2_pos(bat->idx, bat->x, bat->y);        

    // animation
    // bat->frame_delay--;
    // if (!bat->frame_delay) {
    //     bat->frame_delay = BAT_FRAME_DELAY;
    //     bat->frame_current += METATILE;
    //     bat->frame_count--;
    //     if (!bat->frame_count) {
    //         bat->frame_current = 0;     // reset
    //         bat->frame_count = bat->frame_total;
    //     }
    //     set_metasprite2x2_tiles(bat->idx, bat->tile_index + bat->frame_current);
    // }
}

// [CS] Why do we need to keep track of tile idx in VRAM? Free space managing
void init_background(void) {
    set_tiledata(tile_idx, BGTILES_COUNT, bg_tiles);
    // palette 0 = background
    set_tilepal(0, 16, bg_pal); 

    // set_bkg_tiles(0, 0, 32, 24, bg_map);
    
    const u8 *p_map = bg_map;
    for (int y = 0; y < 24; ++y) {
        for (int x = 0; x < 32; ++x) {
            uint8_t idx = *p_map + tile_idx - 1;
            set_tile_xy(x, y, idx);
            p_map++;
        }
    }
    
    tile_idx += BGTILES_COUNT; // update next available tile idx in VRAM
}

void init_font(void) {
    font_t min_font;

    font_init();
    min_font = font_load(font_min); // font_spect
    font_set(min_font);

    tile_idx += 36; // font_min has 36 tiles
}

void init_hud(void) {
    gotoxy(0, 0);
    printf(" SCORE 000000   HIGHSCORE 000000");
}

void main(void) {
    u8 x = SCREEN_WIDTH/2-16, y = SCREEN_HEIGHT/2-16;
    u8 count = 10, frame = 0;

    DISPLAY_OFF;
    HIDE_SPRITES;
    HIDE_BKG;

    init_font();
    init_background();
    init_hud();

    set_sprite_4bpp_data(0, SPRITES_COUNT, sprites_data);
    // palette 1 = sprites
    set_tilepal(1, 16, sprites_palette); 

    init_bats();

    __WRITE_VDP_REG(VDP_R0, __READ_VDP_REG(VDP_R0) |= R0_LCB);     // left column blank
    
    DISPLAY_ON;

    CRITICAL {
        add_LCD(scanline_isr);
        add_VBL(vblank_isr);
        // LYC_REG = 0;
    }
    set_interrupts(VBL_IFLAG | LCD_IFLAG);

    SHOW_BKG;
    SHOW_SPRITES;

    while (1) {
        update();
        
        // first line
        // scroll_pos[scroll_idx] += scroll_spd[scroll_idx];
        // do_scroll(scroll_pos[scroll_idx] >> 4, 0);
        // scroll_idx++;

        // [CS] In a old and very limited CPU, it's easy to see the difference that we can make by tweeks and optimization tricks.
        // [CS] Why do we need to create an export tool for images and maps? Planar vs chunky pixels.
        // [CS] Why using pointers is faster? Index access vs address access? 
        // [CS] Does counting up or down have the same cost?
        Bat* bat = bats;
        for (i = MAX_METASPR; i; i--) {
            update_bat(bat);
            bat++;
        }

        //scroll_bkg(1,1);
        vsync();
    }
}