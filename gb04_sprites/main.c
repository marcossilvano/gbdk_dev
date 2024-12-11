/**
 * Development Platform: Windows
 * GBDK + Emulicious
 * 
 * Compile: Ctrl + Shift + B
 * Run: F5
 * 
 * Debug: launch.json -> "stopOnEntry": true
 * 
 * References:
 *      AngelSix: https://www.youtube.com/playlist?list=PLrW43fNmjaQVmjvIj3Ho3rzW46GEw14F9
 *      Tutorials: https://laroldsjubilantjunkyard.com/tutorials/
 *      GamingMonsters: https://www.youtube.com/playlist?list=PLeEj4c2zF7PaFv5MPYhNAkBGrkx4iPGJo
 *      GamingMonsters examples: https://github.com/gingemonster/GamingMonstersGameBoySampleCode
 *      PaddleCars game: https://blog.ty-porter.dev/development/2021/04/04/writing-a-gameboy-game-in-2021-pt-0.html
 *      GBDK material: https://gbdk-2020.github.io/gbdk-2020/docs/api/docs_links_and_tools.html
 */

#include <gb/gb.h>
#include <stdio.h>
#include "city_tileset.h"
//#include "player_tileset.h"
#include "player_16tileset.h"

#define SCREEN_W 160
#define SCREEN_H 144

#define START_X 8
#define START_Y 16

#define PLAYER_SPEED 2
#define PLAYER_TOTAL_TILES player_tilemap_width*player_tilemap_height

#define ENEMY_MAX 10
#define ENEMY_MULTI 20

#define KEEP_SCROLL_IN_SCREEN(x, y, w, h)           \
    if (x < START_X || x > SCREEN_W+START_X-w) {    \
        x -= player_velx;                           \
        player_velx = 0;                            \
    }                                               \
    if (y < START_Y || y > SCREEN_H+START_Y-h) {    \
        y -= player_vely;                           \
        player_vely = 0;                            \
    }                                               \

#define KEEP_INSIDE_SCREEN(x, y, w, h)          \
    outside_screen = 0;                         \
    if (x < 8) {                                \
        x = 8;                                  \
        outside_screen = 1;                     \
    } else                                      \
    if (x > 160+8-w) {                          \
        x = 8+160-w;                            \
        outside_screen = 1;                     \
    }                                           \
    if (y < 16) {                               \
        y = 16;                                 \
        outside_screen = 1;                     \
    } else                                      \
    if (y > 144+16-h) {                         \
        y = 144+16-h;                           \
        outside_screen = 1;                     \
    }                                           

#define INIT_METASPRITE_POSITION(sprite, x, y, tiles_w, tiles_h)    \
    for (int m = 0; m < tiles_w; m++) {                     \
        for (int n = 0; n < tiles_h; n++) {                 \
            move_sprite(sprite + m*player_tilemap_width + n, x+n*8, y+m*8); \
        }                                                   \
    }

#define SCROLL_METASPRITE(sprite, velx, vely, number_of_tiles)  \
    for (int m = 0; m < number_of_tiles; m++) {         \
        scroll_sprite(sprite + m, velx, vely);                   \
    }

#define SET_SPRITE_TILE(sprite, tile_map, number_of_tiles)  \
    for (int m = 0; m < number_of_tiles; m++) {             \
        set_sprite_tile(sprite + m, tile_map[m]);           \
    }

uint16_t p1 = 0;

void lcd_interrupt(void) {
    SCY_REG = 0;
    
    if (LY_REG < 90u) {
        SCX_REG = p1 >> 2;
        //SCY_REG = wave[wave_count] >> 1; // pq nao armazena o valor jah multiplicado por 2?
    } else
    if (LY_REG > 94u && LY_REG < 100u) {
        SCX_REG = p1 >> 1;
    } else
    if (LY_REG > 120u) {
        SCX_REG = p1;
    }
}

void lcd_interrupt2(void) {
    SCY_REG = 0;
    
    if (LY_REG < 90u) {
        SCX_REG = p1 >> 2;
        //SCY_REG = wave[wave_count] >> 1; // pq nao armazena o valor jah multiplicado por 2?
    } else
    if (LY_REG > 94u && LY_REG < 100u) {
        SCX_REG = p1 >> 1;
    } else
    if (LY_REG > 120u) {
        SCX_REG = p1;
    }
}

void vbl_interrupt(void) {
    p1++;
}

void main() {
    HIDE_SPRITES;
    HIDE_BKG;
    STAT_REG = 8;

    set_bkg_data(0, city_tileset_size, city_tileset);   // load tileset into GB memory
    set_bkg_tiles(0, 0, 32, 18, city_tilemap);          // fill screen with splashscreen map
    
    set_sprite_data(0, player_tileset_size, player_tileset);   
    //SET_SPRITE_TILE(0, player_tilemap, player_tilemap_width * player_tilemap_height)

    UBYTE enemy_sprite[ENEMY_MAX];
    for (int i = 0; i < ENEMY_MAX; i++) {
         enemy_sprite[i] = player_tilemap_width*player_tilemap_height*i;
         SET_SPRITE_TILE(player_tilemap_width*player_tilemap_height*i, player_tilemap, player_tilemap_width * player_tilemap_height);
    }

    disable_interrupts();
    CRITICAL {
        STAT_REG = STATF_MODE00;
        add_LCD(lcd_interrupt);
        add_VBL(vbl_interrupt);
    }
    set_interrupts(IE_REG | LCD_IFLAG);
    //set_interrupts(IE_REG | LCD_IFLAG | VBL_IFLAG);
    // enable_interrupts();

    UBYTE player_x = 8;
    UBYTE player_y = 16;
    int8_t  player_velx = 0;
    int8_t  player_vely = 0;
    uint8_t outside_screen = 0;

    UBYTE enemy_x[ENEMY_MAX];
    UBYTE enemy_y[ENEMY_MAX];
    UBYTE player_idx = 0;
    // BYTE enemy_velx[ENEMY_MAX];
    // BYTE enemy_vely[ENEMY_MAX];

    //INIT_METASPRITE_POSITION(0, player_x, player_y, 3, 3)
    
    for (int i = 0; i < ENEMY_MAX; i++) {
        enemy_x[i] = i * SCREEN_W/ENEMY_MAX + START_X;
        enemy_y[i] = i * SCREEN_H/ENEMY_MAX + START_Y;
        INIT_METASPRITE_POSITION(player_tilemap_width*player_tilemap_height*i, 
                                 enemy_x[i], enemy_y[i], player_tilemap_width, player_tilemap_height)
    }
   
    SHOW_BKG;
    SHOW_SPRITES;
    DISPLAY_ON;
    UBYTE frame = 0;
    uint8_t buttons = 0;
    uint8_t last_buttons;
    
    fixed scroll_spd;
    scroll_spd.w = 0x00FF/3;//0x007F;
    fixed scroll_pos;
    scroll_pos.w = 0;

    while(1) {
        if (frame == 0) {
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
        } 
        frame++;
        if (frame == 1) {
            //player_x += player_velx;
            //player_y += player_vely;

            KEEP_SCROLL_IN_SCREEN(enemy_x[player_idx], enemy_y[player_idx], player_tilemap_width*8, player_tilemap_height*8);
            
            //if (!outside_screen)
            SCROLL_METASPRITE(enemy_sprite[player_idx], player_velx, player_vely, PLAYER_TOTAL_TILES)
        }
        frame++;
        if (frame == 2) {
            frame = 0;
        }

        scroll_pos.w += scroll_spd.w;
        //SCX_REG = scroll_pos.h;
        p1++;
        vsync();
    }
}