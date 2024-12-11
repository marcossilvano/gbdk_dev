/**
 * Moving sprites on screen
 * ------------------------------------------------------
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
#include "Structs/character.h"
#include "defines.h"

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

#define IS_PRESSED(btn) buttons & btn
#define IS_JUST_PRESSED(btn) (buttons & btn) && !(last_buttons & btn)

#define SCROLL_METASPRITE_POS(sprite, velx, vely, number_of_tiles)  \
    for (m = 0; m < number_of_tiles; m++) {         \
        scroll_sprite(sprite + m, velx, vely);      \
    }

#define SET_SPRITE_TILE_OFFSET(sprite, offset, tile_map, number_of_tiles)  \
    n = offset;                                         \
    for (m = 0; m < number_of_tiles; m++) {             \
        set_sprite_tile(sprite + m, tile_map[n]);       \
        n++;                                            \
        if (n == number_of_tiles) n = 0;                \
    }

// GLOBALS ////////////////////////////////////////////////////////////

GameObj player;
GameObj enemies[ENEMY_MAX];

uint8_t player_idx = 0;
GameObj* current = &enemies[0];

uint8_t buttons = 0;
uint8_t last_buttons;

// Iterators
uint8_t i;      // general for-loops
uint8_t m, n;   // macro single and nested for-loops

// MAIN FUNCTION //////////////////////////////////////////////////////

inline void print_byte(uint8_t byte) {
    for (i = 0; i < 8; i++) {
        printf("%d", (byte & 0b10000000) != 0);
        byte = byte << 1;
    }
    printf("\n");
}

void main(void) {
    HIDE_SPRITES;
    HIDE_BKG;
    STAT_REG = 8;

    set_bkg_data(0, city_tileset_size, city_tileset);   // load tileset into GB memory
    set_bkg_tiles(0, 0, 32, 18, city_tilemap);          // fill screen with splashscreen map
    
    set_sprite_data(0, player_tileset_size, player_tileset);   

    current = &enemies[0];
    for (i = 0; i < ENEMY_MAX; i++) {
        GameObj_setup(current, player_tilemap_width*player_tilemap_height*i, 2, 2, 0, 0, player_tileset, player_tilemap, player_tileprop);       
        current->x.h = i * SCREEN_W/ENEMY_MAX + START_X;
        current->y.h = i * SCREEN_H/ENEMY_MAX + START_Y;
        GameObj_update_metasprite(current);
        current++;
    }
   
    SHOW_BKG;
    SHOW_SPRITES;
    DISPLAY_ON;
    
    fixed scroll_spd;
    scroll_spd.w = 0x00FF/3;//0x007F;
    fixed scroll_pos;
    scroll_pos.w = 0;
    current = &enemies[0];

    while(1) {
        last_buttons = buttons;
        buttons = joypad();

        current->velx.w = 0;
        current->vely.w = 0;

        if (IS_JUST_PRESSED(J_A)) {
            player_idx++;
            current++;
            if (player_idx == ENEMY_MAX) {
                player_idx = 0;
                current = &enemies[0];
            }
        }

        if (IS_JUST_PRESSED(J_B)) {
            uint8_t prop;
            for (m = current->sprite_id; m < current->sprite_id+4; m++) {
                prop = get_sprite_prop(m);

                // reverse 6th bit (vertical flip)
                if ((prop & 0b01000000) != 0)
                    prop = prop & 0b10111111;
                else
                    prop = prop | 0b01000000;

                set_sprite_prop(m, prop);
            }
        }

        if (IS_PRESSED(J_LEFT)) {
            current->velx.w = -PLAYER_SPEED;
        } else 
        if (IS_PRESSED(J_RIGHT)) {
            current->velx.w = PLAYER_SPEED;
        } 
        
        if (IS_PRESSED(J_UP)) {
            current->vely.w = -PLAYER_SPEED;
        } else 
        if (IS_PRESSED(J_DOWN)) {
            current->vely.w = PLAYER_SPEED;
        }

        current->x.w += current->velx.w;
        current->y.w += current->vely.w;

        GameObject_keep_inside_screen(current);
        GameObj_update_metasprite(current);

        // scroll background
        scroll_pos.w += scroll_spd.w;
        SCX_REG = scroll_pos.h;
        vsync();
    }
}