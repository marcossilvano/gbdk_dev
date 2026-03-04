/**
 * SMS GBDK + Emulicious
 * 
 * Compile: Ctrl + Shift + B
 * Run: F5
 * Debug: launch.json -> "stopOnEntry": true
 * 
 * Export sprites with TCS8 (tgd=8x16):
 * $ .\TCS8.exe -sms .\sprites.png 16 16 7 -tgd -tid
 */

#include <sms/sms.h>
#include <sms/hardware.h>
#include <gbdk/console.h>
#include <rand.h>
#include <gbdk/emu_debug.h>
#include <gbdk/font.h>
#include <stdio.h>
#include <stdlib.h>

#include "utils.h"
#include "tiles_data.h"

#define METATILE 2
#define SPRITE_WIDTH 16             // full hw sprite (the image)
#define SPRITE_HALF SPRITE_WIDTH/2
#define SPRITE_HIDDEN   200         // y value to put sprite off screen

#define FLICKER
#define FLICKER_GROUPS 2

#define MAX_METASPR 29               // max = 29 (32-3) (player + turbine + item)

#define ENTITY_FRAME_DELAY 10
#define ENTITY_WIDTH 12             // logical sprite (the drawing)
#define ENTITY_HALF ENTITY_WIDTH/2

#define BAT_LAUNCH_DELAY 120

#define BAT_MAX         32-3        // 32 metasprites - (player + item + jet)
#define BAT_SAT         6           // initial SAT entry for balls
#define BAT_TILE_IDX    16          // 8x8 idx in sprite png (4x 8x8)

// [CS] What are enums and when are they useful?
enum BatState {
    BAT_STATE_APPEAR,
    BAT_STATE_RUN
};

#define PLAYER_BOOST_SPEED  4
#define PLAYER_BOOST_TIME   20

#define PLAYER_SPEED    2
#define PLAYER_SAT      0           // fixed SAT entry (2x 8x16)
#define PLAYER_TILE_IDX 0           // 8x8 idx in sprite png

enum PlayerState {
    PLAYER_STATE_RUN,
    PLAYER_STATE_BOOST
};

#define ITEM_COLLECT_DELAY 5
#define ITEM_POINTS     50
#define ITEM_SAT        4           // fixed SAT entry (2x 8x16)
#define ITEM_TILE_IDX   20          // 8x8 idx in sprite png

#define JET_STATE_NORMAL 0
#define JET_STATE_BOOST  1
#define JET_BOOST_TILE_IDX 28       // 8x8 idx in sprite png
#define JET_SAT         2           // fixed SAT entry (2x 8x16)
#define JET_TILE_IDX    24          // 8x8 idx in sprite png

#define DIR_NONE  0
#define DIR_LEFT  1
#define DIR_RIGHT 2
#define DIR_UP    3
#define DIR_DOWN  4

#define ITEM_STATE_APPEAR       0
#define ITEM_STATE_DISAPPEAR    1
#define ITEM_STATE_IDLE         2
#define ITEM_STATE_WAIT         3

#define ITEM_WAIT_DELAY      120
#define ITEM_APPEAR_DELAY    60
#define ITEM_DISAPPEAR_DELAY 60
#define ITEM_IDLE_DELAY      240

typedef struct {
    uint8_t left;
    uint8_t right;
    uint8_t top;
    uint8_t bottom;
} Rect;

typedef struct {
    uint8_t visible;
    uint8_t active;
    uint8_t collidable;

    uint8_t x;
    uint8_t y;
    int8_t spx;
    int8_t spy;
    int8_t dir;
    Rect rect;

    uint8_t sat_idx;
    uint8_t tile_index;
    uint8_t flicker_group;

    uint8_t state;
    uint8_t state_counter;
    void (*state_update)(void);

    // animation
    uint8_t frame_current;
    uint8_t frame_delay;
    uint8_t frame_total;
    uint8_t frame_count;
} GameObject;

GameObject player;
GameObject jet;
GameObject item;
GameObject bats[MAX_METASPR];
uint8_t bats_count;

uint8_t SAT_start = 0;// circular starting SAT index for flickering
uint8_t tile_idx = 0;     // keep track of next available tile idx in VRAM
uint8_t must_update = 0;  // flag to alternate objects logic update
uint8_t flicker_counter = 0;

uint16_t score1;    // high word
uint16_t score2;    // low word
uint16_t highscore1;    
uint16_t highscore2;
// char score_buffer[12]; // support full conversion of a 32 bit integer
uint8_t score[6] = {0};

uint8_t gamestate = 0xff;
uint8_t gamestate_counter;
void (*gamestate_update)(void);

// GAME STATES /////////////////////////////////////////////////////////////////

#define GAMESTATE_TITLE    0    // index for gamestate functions array
#define GAMESTATE_READY    1
#define GAMESTATE_RUN      2
#define GAMESTATE_GAMEOVER 3
#define GAMESTATE_ITEM     4

#define GAMETATES_MAX      5

void gamestate_fun_title(void);
void gamestate_fun_ready(void);
void gamestate_fun_run(void);
void gamestate_fun_gameover(void);
void gamestate_fun_item(void);

void (*gamestate_fun_array[])(void) = {
    gamestate_fun_title,
    gamestate_fun_ready,
    gamestate_fun_run,
    gamestate_fun_gameover,
    gamestate_fun_item
};

// FUNCTION PROTOTYPES /////////////////////////////////////////////////////////

void item_state_wait(void);
void item_state_appear(void);
void item_state_disappear(void);
void item_state_idle(void);

void update_player(void);
void update_player_boost(void);

void update_score(void);
void change_state(uint8_t newstate, uint8_t counter);

// PARALLAX SCROLLIING ////////////////////////////////////////////
#define SCROLL_POS 15u
#define SCROLL_HEIGHT 1u
#define SCROLL_PIX_HEIGHT (SCROLL_HEIGHT * 8u)
#define SCROLL_POS_PIX_START (((SCROLL_POS + DEVICE_SCREEN_Y_OFFSET) * 8u) - 2u)
#define SCROLL_POS_PIX_END (((SCROLL_POS + DEVICE_SCREEN_Y_OFFSET) * 8u) + SCROLL_PIX_HEIGHT - 1u)

#define SCROLL_SIZE 4
// [CS] How can we do floating point math? Fixed point (when needed -> flexible)
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
    scroll_idx = 1;
    // EMU_printf("VBLANK! VBLANK! VBLANK!\n");
}

// [CS] How to run a code at a specific event? Interruptions
// [CS] Why do we need to modify screen during refresh? Raster Effects
// [CS] Hot to optimize access to arrays? Pointers. What are pointers?
void scanline_isr(void) {
    // EMU_printf("VCOUNTER = %u\n", VCOUNTER);

    switch ((uint8_t)VCOUNTER) {    
        // top clouds
        case 15u: do_scroll(scroll_pos[0] >> 4, 0); break;
        case 31u: do_scroll(scroll_pos[1] >> 4, 0); break;
        case 47u: do_scroll(scroll_pos[2] >> 4, 0); break;
        case 63u: do_scroll(scroll_pos[3] >> 4, 0); break;
        
        // bottom clouds
        case 111u: do_scroll(scroll_pos[3] >> 4, 0); break;
        case 127u: do_scroll(scroll_pos[2] >> 4, 0); break;
        case 143u: do_scroll(scroll_pos[1] >> 4, 0); break;
        case 159u: do_scroll(scroll_pos[0] >> 4, 0); break;
        
        default:   do_scroll(0, 0);
    }
}

// UTILITY FUNCTIONS ///////////////////////////////////////////////////////////

inline u8 wrap(u8 value, u8 min, u8 max) {
    if (value > max) 
        return min;
    if (value < min) 
        return max;
    return value;
}

void update_rect(GameObject* obj) {
    obj->rect.left  = obj->x - (ENTITY_HALF - 2);
    obj->rect.right = obj->x + (ENTITY_HALF - 2);
    obj->rect.top   = obj->y - (ENTITY_HALF - 2);
    obj->rect.bottom= obj->y + (ENTITY_HALF - 2);
}

/**
 * Check rectangle collision between two GameObject Rects
 * Note: must update_rect(obj1,obj2) first.
 */
uint8_t check_collision(GameObject* obj1, GameObject* obj2) {
    if (obj1->rect.left > obj2->rect.right ||
        obj2->rect.left > obj1->rect.right ||
        obj1->rect.top > obj2->rect.bottom ||
        obj2->rect.top > obj1->rect.bottom) {
        return FALSE;
    }
    return TRUE;
}

// ENTITy INIT FUNCTIONS ///////////////////////////////////////////////////////

void init_item(void) {
    item.active = TRUE;
    item.visible = FALSE;
    item.collidable = FALSE;
    item.x = 0;
    item.y = SPRITE_HIDDEN;
    item.sat_idx = ITEM_SAT;
    item.tile_index = ITEM_TILE_IDX;

    item.state = ITEM_STATE_WAIT;
    item.state_counter = ITEM_WAIT_DELAY;
    item.state_update = item_state_wait;
    
    set_sprite_tile(ITEM_SAT, ITEM_TILE_IDX);
    set_sprite_tile(ITEM_SAT+1, ITEM_TILE_IDX+2);
}

void init_jet(void) {
    jet.active = TRUE;
    jet.visible = TRUE;
    jet.x = 0;
    jet.y = SPRITE_HIDDEN;
    jet.sat_idx = JET_SAT;
    jet.tile_index = JET_TILE_IDX;
    jet.state = JET_STATE_NORMAL;
    
    set_sprite_tile(JET_SAT, JET_TILE_IDX);
    set_sprite_tile(JET_SAT+1, JET_TILE_IDX+2);
}

void init_player(void) {
    player.active  = TRUE;
    player.visible = TRUE;
    player.x = SCREEN_WIDTH/2;
    player.y = SCREEN_HEIGHT/2 + SCREEN_HEIGHT/4; // 1/2 + 1/4 = 3/4
    player.spx = PLAYER_SPEED;
    player.spy = 0;
    player.sat_idx = PLAYER_SAT;
    player.tile_index = PLAYER_TILE_IDX;
    player.dir = DIR_NONE;

    player.state = PLAYER_STATE_RUN;
    player.state_update = update_player;

    set_sprite_tile(PLAYER_SAT, PLAYER_TILE_IDX);
    set_sprite_tile(PLAYER_SAT+1, PLAYER_TILE_IDX+2);

    init_jet();
}

void init_bats(void) {
    // [CS] Why do we need to set a tile index?
    j = METATILE * 3;       // tile index (starts after player and item)
    uint8_t visible = 0;
    for (i = 0; i < MAX_METASPR; i++) {
        GameObject* bat = &bats[i];
        bat->active = FALSE;
        #ifdef FLICKER
            bat->visible = visible;
            visible != visible; // this alternate enemies in list
        #else
            bat->visible = TRUE;
        #endif

        bat->x = rand() % (SCREEN_WIDTH-ENTITY_WIDTH);
        bat->y = rand() % (SCREEN_HEIGHT-ENTITY_WIDTH);

        bat->spx = rand() % 3 - 1;
        if (!bat->spx) bat->spx = 1;
        bat->spy = rand() % 3 - 1;
        // bat->spx = 0;
        // bat->spy = 0;
        
        bat->tile_index = BAT_TILE_IDX;
        bat->frame_delay = i % ENTITY_FRAME_DELAY + 1; // count = 10
        bat->frame_current = 0;
        bat->frame_total = 2;
        bat->frame_count = bat->frame_total;
        bat->sat_idx = j;
        bat->flicker_group = i % FLICKER_GROUPS;

        #if METATILE == 4
            set_metasprite2x2_tiles(bat->idx, bat->tile_index + bat->frame_current);
        #elif METATILE == 2
            set_sprite_tile(bat->sat_idx, BAT_TILE_IDX);
            set_sprite_tile(bat->sat_idx+1, BAT_TILE_IDX+2);
        #elif METATILE == 1
            set_sprite_tile(bat->idx, bat->tile_index);
        #endif
        j += METATILE;
    }
}

// UPDATE FUNCTIONS ///////////////////////////////////////////////////////////

void update_jet(void) {
    if (player.dir == DIR_NONE) return;

    // update jet
    switch (player.dir)
    {
        case DIR_LEFT:
            if (jet.state == JET_STATE_BOOST) 
                jet.y = player.y;
            else
                jet.y = player.y + 1;
            jet.x = player.x + 6;
            break;

        case DIR_RIGHT:
            if (jet.state == JET_STATE_BOOST) 
                jet.y = player.y;
            else
                jet.y = player.y + 1;
            jet.x = player.x - 6;
            break;
        
        case DIR_UP:
            jet.y = player.y + 6;               
            jet.x = player.x;
            break;

        case DIR_DOWN:
            if (jet.state == JET_STATE_BOOST) 
                jet.y = player.y - 6;
            else
                jet.y = player.y - 5;
            jet.x = player.x;
            break;
    }

    // draw jet
    if (jet.visible) {
        move_sprite(JET_SAT, jet.x - SPRITE_HALF, jet.y - SPRITE_HALF);
        move_sprite(JET_SAT+1, jet.x - SPRITE_HALF + 8, jet.y - SPRITE_HALF);
    } else {
        move_sprite(JET_SAT, 0, SPRITE_HIDDEN);
        move_sprite(JET_SAT+1, 0, SPRITE_HIDDEN);
    }

    jet.visible = !jet.visible;
}

void player_boost(void) {
    player.spx = PLAYER_BOOST_SPEED;
    player.state_counter = PLAYER_BOOST_TIME;
    player.state_update = update_player_boost;
    
    jet.state = JET_STATE_BOOST;
    set_sprite_tile(JET_SAT, JET_BOOST_TILE_IDX);
    set_sprite_tile(JET_SAT+1, JET_BOOST_TILE_IDX+2);
}

void update_player_boost(void) {
    if (!player.state_counter) {
        player.state = PLAYER_STATE_RUN;
        player.state_update = update_player;
        player.spx = PLAYER_SPEED;

        jet.state = JET_STATE_NORMAL;
        set_sprite_tile(JET_SAT, JET_TILE_IDX);
        set_sprite_tile(JET_SAT+1, JET_TILE_IDX+2);
    }
    player.state_counter--;
    update_player();
}

void update_player(void) {
    if (key_pressed(JOY_P1_LEFT)) {
        set_sprite_tile(PLAYER_SAT, PLAYER_TILE_IDX+8);
        set_sprite_tile(PLAYER_SAT+1, PLAYER_TILE_IDX+10);
        player.dir = DIR_LEFT;
    } else
    if (key_pressed(JOY_P1_RIGHT)) {
        set_sprite_tile(PLAYER_SAT, PLAYER_TILE_IDX);
        set_sprite_tile(PLAYER_SAT+1, PLAYER_TILE_IDX+2);
        player.dir = DIR_RIGHT;
    } else
    if (key_pressed(JOY_P1_UP)) {
        set_sprite_tile(PLAYER_SAT, PLAYER_TILE_IDX+4);
        set_sprite_tile(PLAYER_SAT+1, PLAYER_TILE_IDX+6);
        player.dir = DIR_UP;
    } else
    if (key_pressed(JOY_P1_DOWN)) {
        set_sprite_tile(PLAYER_SAT, PLAYER_TILE_IDX+12);
        set_sprite_tile(PLAYER_SAT+1, PLAYER_TILE_IDX+14);
        player.dir = DIR_DOWN;
    }

    // move
    switch (player.dir) {
        case DIR_LEFT:  player.x -= player.spx; break;
        case DIR_RIGHT: player.x += player.spx; break;
        case DIR_UP:    player.y -= player.spx; break;
        case DIR_DOWN:  player.y += player.spx; break;
    }
    // player.x += player.spx;
    // player.y += player.spy;

    // player.x = wrap(player.x, SCREEN_LEFT, SCREEN_RIGHT);
    player.y = wrap(player.y, SCREEN_TOP, SCREEN_BOTTOM);

    if (player.visible) {
        move_sprite(PLAYER_SAT, player.x - SPRITE_HALF, player.y - SPRITE_HALF);
        move_sprite(PLAYER_SAT+1, player.x - SPRITE_HALF + 8, player.y - SPRITE_HALF);
    } else {
        move_sprite(PLAYER_SAT, 0, SPRITE_HIDDEN);
        move_sprite(PLAYER_SAT+1, 0, SPRITE_HIDDEN);
    }

    update_jet();
}

// [CS] Why using pointer? Struct copy vs pointer
void update_bat(GameObject* bat) {
    // move bat
    bat->x += bat->spx;
    bat->y += bat->spy;

    // bounce off screen
    if (bat->x < (ENTITY_HALF+SCREEN_LEFT)) {    // 8 = first column width (hidden)
        bat->x = (ENTITY_HALF+SCREEN_LEFT);
        bat->spx = -bat->spx;
    } 
    else
    if (bat->x > SCREEN_WIDTH-ENTITY_HALF) {
        bat->x = (u8)(SCREEN_WIDTH-ENTITY_HALF);
        bat->spx = -bat->spx;
    }

    if (bat->y < (ENTITY_HALF+SCREEN_TOP)) {    // 8 = HUD height
        bat->y = (ENTITY_HALF+SCREEN_TOP);
        bat->spy = -bat->spy;
    } 
    else
    if (bat->y > SCREEN_HEIGHT-ENTITY_HALF) {
        bat->y = (u8)(SCREEN_HEIGHT-ENTITY_HALF);
        bat->spy = -bat->spy;
    }

    #ifdef FLICKER
        if (bat->flicker_group == flicker_counter) {
            bat->visible = TRUE;
        } else {
            bat->visible = FALSE;
        }
        // bat->visible = !bat->visible;
    #endif

    #if METATILE == 4
        set_metasprite2x2_pos(bat->idx, bat->x, bat->y);        
    #elif METATILE == 2
        // uint8_t idx1 = SAT_start + bat->sat_idx;
        // if (idx1 > 62) {
        //     idx1 = 6; // 0-5: reserved for player + jet + item
        // }
        // uint8_t idx2 = idx1 + 1;

        if (bat->visible) {
            move_sprite(bat->sat_idx, bat->x - SPRITE_HALF, bat->y - SPRITE_HALF);
            move_sprite(bat->sat_idx+1, bat->x - SPRITE_HALF + 8, bat->y - SPRITE_HALF);
        } else {
            move_sprite(bat->sat_idx, 0, SPRITE_HIDDEN);
            move_sprite(bat->sat_idx+1, 0, SPRITE_HIDDEN);
        }
    #elif METATILE == 1
        move_sprite(bat->idx, bat->x, bat->y);
    #endif

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

void update_bats(void) {
    GameObject* bat;
    if (must_update) {
        bat = bats;
    } else {
        bat = bats + MAX_METASPR/2;
    
        // flicker
        flicker_counter++;
        if (flicker_counter == FLICKER_GROUPS) {
            flicker_counter = 0;
        }
    }
    for (i = MAX_METASPR/2; i; i--) {
        if (bat->active) {
            update_bat(bat);
            update_rect(bat);
            if (check_collision(&player, bat)) {
                gamestate = GAMESTATE_GAMEOVER;
            }
        }
        bat++;
    }

    must_update = !must_update;

}

// ITEM STATE UPDATE FUNCTIONS /////////////////////////////////////////////////

void update_item(void) {
    if (item.visible) {
        move_sprite(ITEM_SAT, item.x - SPRITE_HALF, item.y - SPRITE_HALF);
        move_sprite(ITEM_SAT+1, item.x - SPRITE_HALF + 8, item.y - SPRITE_HALF);
    } else {
        move_sprite(ITEM_SAT, 0, SPRITE_HIDDEN);
        move_sprite(ITEM_SAT+1, 0, SPRITE_HIDDEN);
    }

    // [CS] How to implement a flexibe state machine? Function pointers
    item.state_update();
}

void item_state_wait(void){
    item.state_counter--;
    if (!item.state_counter) {
        item.state_counter = ITEM_APPEAR_DELAY;
        item.state = ITEM_STATE_APPEAR;
        item.state_update = item_state_appear;
        
        // rand position
        item.x = rand() % (SCREEN_WIDTH - SCREEN_LEFT - SPRITE_WIDTH*2) + SPRITE_WIDTH;
        item.y = rand() % (SCREEN_HEIGHT - SCREEN_TOP - SPRITE_WIDTH*2) + SPRITE_WIDTH;
    }
}

void item_state_appear(void) {
    item.visible = !item.visible;
    item.state_counter--;
    if (!item.state_counter) {
        item.state_counter = ITEM_IDLE_DELAY;
        item.state = ITEM_STATE_IDLE;
        item.state_update = item_state_idle;
        item.collidable = TRUE;
        item.visible = TRUE;

        update_rect(&item);
    }
}

void item_state_disappear(void) {
    item.visible = !item.visible;
    item.state_counter--;
    if (!item.state_counter) {
        item.state_counter = ITEM_WAIT_DELAY;
        item.state = ITEM_STATE_WAIT;
        item.state_update = item_state_wait;
        item.visible = FALSE;
    }
}

void item_state_idle(void) {
    item.state_counter--;
    if (!item.state_counter) {
        item.state_counter = ITEM_DISAPPEAR_DELAY;
        item.state = ITEM_STATE_DISAPPEAR;
        item.state_update = item_state_disappear;
        item.collidable = FALSE;
    }   
}

void collect_item(void) {
    item.state_counter = ITEM_WAIT_DELAY;
    item.state = ITEM_STATE_WAIT;
    item.state_update = item_state_wait;
    item.visible = FALSE;

    // change_state(GAMESTATE_ITEM, ITEM_COLLECT_DELAY);

    player_boost();

    score2 += ITEM_POINTS;
    update_score();
}

// GENERAL INIT/UPDATE FUNCTIONS ///////////////////////////////////////////////

// [CS] Why do we need to keep track of tile idx in VRAM? Free space managing
void init_background(void) {
    set_tiledata(tile_idx, BGTILES_COUNT, bg_tiles);
    // palette 0 = background
    set_tilepal(0, 16, bg_pal); 

    // set_bkg_tiles(0, 0, 32, 24, bg_map);
    
    const u8 *p_map = bg_map;
    for (j = 0; j < 24; ++j) {
        for (int i = 0; i < 32; ++i) {
            uint8_t idx = *p_map + tile_idx - 1;
            set_tile_xy(i, j, idx);
            p_map++;
        }
    }
    
    tile_idx += BGTILES_COUNT; // update next available tile idx in VRAM
}

void update_background(void) {
    scroll_pos[3] += scroll_spd[3];
    scroll_pos[2] += scroll_spd[2];
    scroll_pos[1] += scroll_spd[1];
    scroll_pos[0] += scroll_spd[0];
}

void init_game(void) {
    score1 = 0;
    score2 = 0;
    highscore1 = 0;
    highscore2 = 0;
    bats_count = 0;

    change_state(GAMESTATE_TITLE, 0);
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
    // printf(" SCORE          HIGHSCORE       ");
}

void update_hud2(void) {
    static char buf[7];
    uint16_t s1 = score1;
    uint16_t s2 = score2;
    
    // Process High Word (score1) - 2 digits
    buf[0] = '0'; while (s1 >= 10) { s1 -= 10; buf[0]++; }
    buf[1] = s1 + '0';

    // Process Low Word (score2) - 4 digits
    buf[2] = '0'; while (s2 >= 1000) { s2 -= 1000; buf[2]++; }
    buf[3] = '0'; while (s2 >= 100)  { s2 -= 100;  buf[3]++; }
    buf[4] = '0'; while (s2 >= 10)   { s2 -= 10;   buf[4]++; }
    buf[5] = s2 + '0';
    buf[6] = '\0';

    gotoxy(7, 0);
    printf(buf);    
}

void update_hud(void) {
    // static score_buffer[7];
    
    // reset score in HUD
    gotoxy(7, 0);
    printf("000000");

    // print high word
    if (score1 != 0) {
        if (score1 > 9) { 
            gotoxy(7+0, 0); // high word has 2 digits
        } else {
            gotoxy(7+1, 0); // high word has 1 digit
        }
        printf("%u", score1);
    }
    
    // print low word
    if (score2 != 0) {
        if (score2 > 999) { 
            gotoxy(7+2, 0); // low word has 4 digits
        } else
        if (score2 > 99) { 
            gotoxy(7+3, 0); // low word has 3 digits
        } else
        if (score2 > 9) { 
            gotoxy(7+4, 0); // low word has 2 digits
        } else {
            gotoxy(7+5, 0); // low word has 1 digit
        }
        printf("%u", score2);
    }
}

void update_score(void) {
    // update high and low word
    if (score2 > 9999) {
        score2 -= 10000;
        score1++;
    }

    // check limit: 999999
    if (score1 > 99) {
        score1 = 99;
        score2 = 9999;
    }
    update_hud2();
}

// MAIN FUNCTION ///////////////////////////////////////////////////////////////

// [CS] Function arrays avoid switches around the code
void change_state(uint8_t newstate, uint8_t counter) {
    if (newstate != gamestate) {
        if (newstate >= GAMETATES_MAX) return;  // check for error
        gamestate = newstate;
        gamestate_counter = counter;
        gamestate_update = gamestate_fun_array[newstate];
    }
}

void gamestate_fun_title(void) {
    change_state(GAMESTATE_RUN, BAT_LAUNCH_DELAY);
}

void gamestate_fun_ready(void) {}

void launch_ball(void) {
    if (bats_count < BAT_MAX) {
        bats[bats_count].active = TRUE;
        bats[bats_count].state = BAT_STATE_APPEAR;
        
        bats_count++;
    }
}

void gamestate_fun_run(void) {
    if (!gamestate_counter) { // launch ball!
        gamestate_counter = BAT_LAUNCH_DELAY;
        launch_ball();
    }
    gamestate_counter--;
    
    player.state_update();
    update_rect(&player);
    update_item();

    // [CS] In a old and very limited CPU, it's easy to see the difference that we can make by tweeks and optimization tricks.
    // [CS] Why do we need to create an export tool for images and maps? Planar vs chunky pixels.
    // [CS] Why using pointers is faster? Index access vs address access? 
    // [CS] Does counting up or down have the same cost?
    update_bats();

    if (item.state == ITEM_STATE_IDLE) {
        if (check_collision(&player, &item)) {
            collect_item();
        }
    }
    
    update_background();

    // @ collision debug
    if (gamestate == GAMESTATE_GAMEOVER) {
        set_palette_entry(1, 0, 0x03); // --BBGGRR
        gamestate = GAMESTATE_RUN;
    } else {
        set_palette_entry(1, 0, 0x00);
    }
}

void gamestate_fun_gameover(void) {}

void gamestate_fun_item(void) {
    if (!gamestate_counter) {
        change_state(GAMESTATE_RUN, 0);
        return;
    }
    gamestate_counter--;
}

void main(void) {
    DISPLAY_OFF;
    HIDE_SPRITES;
    HIDE_BKG;
    
    #if METATILE == 2
        SPRITES_8x16;
    #endif
    init_game();
    init_font();
    init_background();
    init_hud();

    set_sprite_4bpp_data(0, SPRITES_COUNT, sprites_data);
    // palette 1 = sprites
    set_tilepal(1, 16, sprites_palette); 

    init_player();
    init_item();
    init_bats();

    __WRITE_VDP_REG(VDP_R0, __READ_VDP_REG(VDP_R0) |= R0_LCB);     // left column blank
    
    DISPLAY_ON;

    CRITICAL {
        add_LCD(scanline_isr);
        add_VBL(vblank_isr);
    }
    set_interrupts(VBL_IFLAG | LCD_IFLAG);

    SHOW_BKG;
    SHOW_SPRITES;

    while (1) {
        key_update();
        gamestate_update();
        vsync();
    }
}