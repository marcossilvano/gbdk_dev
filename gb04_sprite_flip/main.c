/**
 * GBDK + Emulicious
 * 
 * Compile: Ctrl + Shift + B
 * Run: F5
 * Debug: launch.json -> "stopOnEntry": true
 */

#include <gb/gb.h>
#include <stdio.h>
#include "city_tileset.h"
//#include "player_tileset.h"
#include "player_16tileset.h"

// DEFINITIONS ////////////////////////////////////////////////////////

#define SCREEN_W 160
#define SCREEN_H 144

#define START_X 8
#define START_Y 16

#define PLAYER_SPEED 2
#define PLAYER_TOTAL_TILES player_tilemap_width*player_tilemap_height

#define ENEMY_MAX 10
//#define ENEMY_MULT 20

// MACROS /////////////////////////////////////////////////////////////

#define KEEP_SCROLL_IN_SCREEN(x, y, w, h)           \
    if (x < START_X || x > SCREEN_W+START_X-w) {    \
        x -= player_velx;                           \
        player_velx = 0;                            \
    }                                               \
    if (y < START_Y || y > SCREEN_H+START_Y-h) {    \
        y -= player_vely;                           \
        player_vely = 0;                            \
    }                                               \

#define KEEP_POS_IN_SCREEN(x, y, w, h)          \
    if (x < START_X) {                          \
        x = START_X;                            \
    } else                                      \
    if (x > SCREEN_W+START_X-w) {               \
        x = SCREEN_W+START_X-w;                 \
    }                                           \
    if (y < START_Y) {                          \
        y = START_Y;                            \
    } else                                      \
    if (y > SCREEN_H+START_Y-h) {               \
        y = SCREEN_H+START_Y-h;                 \
    }                                           

#define SET_METASPRITE_POS(sprite, x, y, tiles_w, tiles_h)    \
    uint8_t xoffset = x;                            \
    uint8_t idx = sprite;                           \
    uint8_t yoffset = y;                            \
    for (m = 0; m < tiles_w; m++) {                 \
        for (n = 0; n < tiles_h; n++) {             \
            move_sprite(idx, xoffset, yoffset);     \
            idx++;                                  \
            xoffset += 8u;                          \
        }                                           \
        xoffset = x;                                \
        yoffset += 8u;                              \
    }

#define SCROLL_METASPRITE_POS(sprite, velx, vely, number_of_tiles)  \
    for (m = 0; m < number_of_tiles; m++) {         \
        scroll_sprite(sprite + m, velx, vely);      \
    }

#define SET_SPRITE_TILE(sprite, tile_map, tile_prop, number_of_tiles)  \
    for (m = 0; m < number_of_tiles; m++) {             \
        set_sprite_tile(sprite + m, tile_map[m]);       \
        set_sprite_prop(sprite + m, tile_prop[m]);      \
    }

#define SET_SPRITE_TILE_OFFSET(sprite, offset, tile_map, number_of_tiles)  \
    n = offset;                                         \
    for (m = 0; m < number_of_tiles; m++) {             \
        set_sprite_tile(sprite + m, tile_map[n]);       \
        n++;                                            \
        if (n == number_of_tiles) n = 0;                \
    }

// GLOBALS ////////////////////////////////////////////////////////////

int8_t  player_velx = 0;
int8_t  player_vely = 0;

uint8_t enemy_x[ENEMY_MAX];
uint8_t enemy_y[ENEMY_MAX];

uint8_t player_idx = 0;
uint8_t enemy_sprite[ENEMY_MAX];

// Iterators
uint8_t i;      // general for-loops
uint8_t m, n;   // macro single and nested for-loops

// LCD INTERRUPT HANDLERS /////////////////////////////////////////////

void lcd_interrupt(void) {
    SET_METASPRITE_POS(0, enemy_x[i], enemy_y[i]+LY_REG, player_tilemap_width, player_tilemap_height)
    refresh_OAM();

    // if (LY_REG == 80u) {
    //     for (UBYTE i = 0; i < ENEMY_MAX; i++) {
    //         SET_METASPRITE_POS(player_tilemap_width*player_tilemap_height*i, 
    //                         enemy_x[i], enemy_y[i]+80u, player_tilemap_width, player_tilemap_height)

    //     }
    // }
    
    // if (LY_REG < 90u) {
    //     SCX_REG = p1 >> 2;
    //     SCY_REG = 0;
    // }
}

void vbl_interrupt(void) {
    //p1++;
}

// MAIN FUNCTION //////////////////////////////////////////////////////

inline void print_byte(uint8_t byte) {
    for (i = 0; i < 8; i++) {
        printf("%d", (byte & 0b10000000) != 0);
        byte = byte << 1;
    }
    printf("\n");
}

void main() {
    HIDE_SPRITES;
    HIDE_BKG;
    STAT_REG = 8;

    set_bkg_data(0, city_tileset_size, city_tileset);   // load tileset into GB memory
    set_bkg_tiles(0, 0, 32, 18, city_tilemap);          // fill screen with splashscreen map
    
    set_sprite_data(0, player_tileset_size, player_tileset);   

    for (i = 0; i < ENEMY_MAX; i++) {
         enemy_sprite[i] = player_tilemap_width*player_tilemap_height*i;
         SET_SPRITE_TILE(enemy_sprite[i], player_tilemap, player_tileprop, player_tilemap_width * player_tilemap_height);
    }

    disable_interrupts();
    CRITICAL {
        STAT_REG = STATF_LYC;
        LYC_REG = 70u;
        add_LCD(lcd_interrupt);
        //add_VBL(vbl_interrupt);
    }
    //set_interrupts(IE_REG | LCD_IFLAG);
    //set_interrupts(IE_REG | LCD_IFLAG | VBL_IFLAG);

    for (i = 0; i < ENEMY_MAX; i++) {
        enemy_x[i] = i * SCREEN_W/ENEMY_MAX + START_X;
        enemy_y[i] = i * SCREEN_H/ENEMY_MAX + START_Y;
        SET_METASPRITE_POS(player_tilemap_width*player_tilemap_height*i, 
                           enemy_x[i], enemy_y[i], player_tilemap_width, player_tilemap_height)
    }
   
    SHOW_BKG;
    SHOW_SPRITES;
    DISPLAY_ON;
    uint8_t frame = 0;
    uint8_t buttons = 0;
    uint8_t last_buttons;
    
    fixed scroll_spd;
    scroll_spd.w = 0x00FF/3;//0x007F;
    fixed scroll_pos;
    scroll_pos.w = 0;

    while(1) {
        // if (frame == 0) {
            last_buttons = buttons;
            buttons = joypad();

            player_velx = 0;
            player_vely = 0;

            if (buttons & J_A && !(last_buttons & J_A)) {
                player_idx++;
                if (player_idx == ENEMY_MAX) {
                    player_idx = 0;
                }
            }

            if (buttons & J_B && !(last_buttons & J_B)) {
                uint8_t prop;
                for (m = enemy_sprite[player_idx]; m < enemy_sprite[player_idx]+4; m++) {
                    prop = get_sprite_prop(m);

                    // reverse 6th bit (vertical flip)
                    if ((prop & 0b01000000) != 0)
                        prop = prop & 0b10111111;
                    else
                        prop = prop | 0b01000000;

                    set_sprite_prop(m, prop);
                }
            }

            if (buttons & J_LEFT) {
                player_velx = -PLAYER_SPEED;
            } else 
            if (buttons & J_RIGHT) {
                player_velx = PLAYER_SPEED;
            }

            if (buttons & J_UP) {
                player_vely = -PLAYER_SPEED;
            } else 
            if (buttons & J_DOWN) {
                player_vely = PLAYER_SPEED;
            }

            enemy_x[player_idx] += player_velx;
            enemy_y[player_idx] += player_vely;
        // } 
        // frame++;
        // if (frame == 1u) {
            KEEP_POS_IN_SCREEN(enemy_x[player_idx], enemy_y[player_idx], player_tilemap_width*8u, player_tilemap_height*8u);

            SET_METASPRITE_POS(player_tilemap_width*player_tilemap_height*player_idx, 
                               enemy_x[player_idx], enemy_y[player_idx], player_tilemap_width, player_tilemap_height)

            //SCROLL_METASPRITE_POS(enemy_sprite[player_idx], player_velx, player_vely, PLAYER_TOTAL_TILES)
        // }
        
        // frame++;
        // if (frame == 2u) {
        //     frame = 0;
        // }

        // scroll background
        scroll_pos.w += scroll_spd.w;
        SCX_REG = scroll_pos.h;
        vsync();
    }
}