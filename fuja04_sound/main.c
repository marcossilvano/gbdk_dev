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
#include "psg.h"
#include "tiles_data.h"

#define METATILE 2
#define SPRITE_WIDTH 16             // full hw sprite (the image)
#define SPRITE_HALF SPRITE_WIDTH/2
#define SPRITE_HIDDEN   200         // y value to put sprite off screen

#define FLICKER
#define FLICKER_GROUPS 2

#define MAX_BATS 29               // max = 29 (32-3) (player + turbine + item)

#define ENTITY_FRAME_DELAY 10
#define ENTITY_WIDTH 12             // logical sprite (the drawing)
#define ENTITY_HALF ENTITY_WIDTH/2

#define BAT_LAUNCH_DELAY 60*10
#define BAT_APPEAR_BLINK_INTERVAL 7
#define BAT_SPEED     1
#define BAT_MAX       32-3        // 32 metasprites - (player + item + jet)
#define BAT_SAT_START 6           // initial SAT entry for balls
#define BAT_TILE_IDX  0x28        // 8x8 idx in sprite png (4x 8x8)

#define BAT_TYPE_1 1
#define BAT_TYPE_2 2
#define BAT_TYPE_3 3

#define PLAYER_BOOST_SPEED  floatTofix(2.5f)
#define PLAYER_BOOST_TIME   20

#define PLAYER_SHIELD   4
#define PLAYER_SPEED    floatTofix(1.25f)
#define PLAYER_DELAY_INVINCIBLE 60
#define PLAYER_DELAY_EXPLOSION  30
#define PLAYER_SAT      0           // fixed SAT entry (2x 8x16)
#define PLAYER_TILE_IDX 0           // 8x8 idx in sprite png

#define EXPLOSION_TILE_IDX 0x34

#define JET_BOOST_TILE_IDX 20        // 8x8 idx in sprite png
#define JET_SAT         2           // fixed SAT entry (2x 8x16)
#define JET_TILE_IDX    16           // 8x8 idx in sprite png

#define DIR_NONE  0
#define DIR_LEFT  1
#define DIR_RIGHT 2
#define DIR_UP    3
#define DIR_DOWN  4

#define ITEM_DELAY_WAIT      180
#define ITEM_DELAY_APPEAR    45
#define ITEM_DELAY_DISAPPEAR 45
#define ITEM_DELAY_IDLE      240
#define ITEM_DELAY_SHOW_PTS  20

#define GAMESTATE_DELAY_READY 200//41*4

#define ITEM_FX_PAUSE 10
#define ITEM_POINTS   50
#define ITEM_SAT      4          // fixed SAT entry (2x 8x16)
#define ITEM_TILE_IDX     0x18   // 8x8 idx in sprite png
#define ITEM_PTS_TILE_IDX 0x20   // 8x8 idx in sprite png

typedef struct {
    u8 left;
    u8 right;
    u8 top;
    u8 bottom;
} Rect;

typedef struct gameObject {
    u8 visible;
    u8 active;
    u8 collidable;

    u8 x;
    u8 y;
    i8 spx;
    i8 spy;
    i8 dir;
    Rect rect;
    
    u16 fx;
    u16 fy;
    i16 fspx;
    i16 fspy;

    u8 sat_idx;
    u8 tile_index;
    u8 flicker_group;

    u8 state;
    u8 state_counter;
    void (*state_update)(struct gameObject*);

    u8 counter;         // general counter
    // animation
    u8 frame_current;
    u8 frame_delay;
    u8 frame_total;
    u8 frame_count;
} GameObject;

GameObject player;
u8 player_shield;
GameObject jet;
GameObject item;
GameObject bats[MAX_BATS];
u8 bats_count;
u16 bats_launch_counter;

u8 SAT_start = 0;// circular starting SAT index for flickering
u8 tile_idx = 0;     // keep track of next available tile idx in VRAM
u8 must_update = 0;  // flag to alternate objects logic update
u8 flicker_counter = 0;

u8 idx_bg;
u8 idx_title;

u8 weaver_colors[] = {0x24,0x39,0x3E};
u8 weaver_color_counter;

u16 score1;    // high word
u16 score2;    // low word
u16 highscore1;    
u16 highscore2;
u8 item_combo;
u8 boost_color_counter;

// VDP syncronization
u8 refresh_hud_score;
u8 refresh_hud_hiscore;
// [CS] How to handle concurrent accesss to VRAM by CPU and VDP? Semaphor for avoiding interrupt crashing when changing tiles
volatile u8 vdp_busy;        

u8 gamestate = 0xff;
u8 gamestate_counter;
void (*gamestate_update)(void);

// GAME STATES /////////////////////////////////////////////////////////////////

enum GameState {
    GAMESTATE_TITLE,         // index for gamestate functions array
    GAMESTATE_READY,
    GAMESTATE_RUN,
    GAMESTATE_GAMEOVER,
    GAMESTATE_ITEM,
    GAMESTATE_PAUSE,
    
    GAMESTATE_NUMBER_OF_STATES
};


void gamestate_fun_title(void);
void gamestate_fun_ready(void);
void gamestate_fun_run(void);
void gamestate_fun_gameover(void);
void gamestate_fun_item(void);
void gamestate_fun_pause(void);

const void const (*gamestate_fun_array[])(void) = {
    gamestate_fun_title,
    gamestate_fun_ready,
    gamestate_fun_run,
    gamestate_fun_gameover,
    gamestate_fun_item,
    gamestate_fun_pause
};

// ITEM STATES ////////////////////////////////////////////////////////////////

enum ItemState {
    ITEM_STATE_APPEAR,
    ITEM_STATE_DISAPPEAR,
    ITEM_STATE_IDLE,
    ITEM_STATE_WAIT,
    ITEM_STATE_COLLECTED,
    
    ITEM_NUMBER_OF_STATES
};

void item_change_state(u8 newstate, u8 counter);

void item_state_appear(GameObject*);
void item_state_disappear(GameObject*);
void item_state_idle(GameObject*);
void item_state_wait(GameObject*);
void item_state_collected(GameObject*);

const void const (*item_state_fun_array[])(GameObject*) = {
    item_state_appear,
    item_state_disappear,
    item_state_idle,
    item_state_wait,
    item_state_collected
};

// PLAYER STATES //////////////////////////////////////////////////////////////

enum JetState {
    JET_STATE_NORMAL,
    JET_STATE_BOOST,

    JET_NUMBER_OF_STATES
};

enum PlayerState {
    PLAYER_STATE_RUN,
    PLAYER_STATE_BOOST,
    PLAYER_STATE_HIT,
    PLAYER_STATE_INVINCIBLE,
    
    PLAYER_NUMBER_OF_STATES
};

void player_change_state(GameObject* ply, u8 newstate, u8 counter);

void player_state_run(GameObject*);
void player_state_boost(GameObject*);
void player_state_hit(GameObject*);
void player_state_invincible(GameObject*);

const void const (*player_state_fun_array[])(GameObject*) = {
    player_state_run,
    player_state_boost,
    player_state_hit,
    player_state_invincible
};

// ENEMY STATES ///////////////////////////////////////////////////////////////

// [CS] What are enums and when are they useful?
// [CS] How to avoid overflow of enum values? Convetion -> last item as N
enum BatState {
    BAT_STATE_APPEAR,
    BAT_STATE_RUN,
    BAT_STATE_DESTROY,

    BAT_NUMBER_OF_STATES
};

void bat_change_state(GameObject* bat, u8 newstate, u8 counter);

void bat_state_appear(GameObject*);
void bat_state_run(GameObject*);
void bat_state_destroy(GameObject*);

const void const (*bat_state_fun_array[])(GameObject*) = {
    bat_state_appear,
    bat_state_run,
    bat_state_destroy
};

// FUNCTION PROTOTYPES ////////////////////////////////////////////////////////

void init_title(void);
void init_ready(void);
void init_run(void);
void init_gameover(void);

void update_score(void);
void update_hud(void);
void update_hud2(u16, u16, u8, u8);

void game_change_state(u8 newstate, u8 counter);
void reset_all_sprites(void);

// PARALLAX SCROLLING /////////////////////////////////////////////////////////

#define SCROLL_POS 15u
#define SCROLL_HEIGHT 1u
#define SCROLL_PIX_HEIGHT (SCROLL_HEIGHT * 8u)
#define SCROLL_POS_PIX_START (((SCROLL_POS + DEVICE_SCREEN_Y_OFFSET) * 8u) - 2u)
#define SCROLL_POS_PIX_END (((SCROLL_POS + DEVICE_SCREEN_Y_OFFSET) * 8u) + SCROLL_PIX_HEIGHT - 1u)

#define SCROLL_SIZE 4

// [CS] How can we do floating point math? Fixed point (when needed -> flexible)
u8 scroll_spd[] = {0x04, 0x03, 0x02, 0x01};
u8 scroll_pos[] = {0, 0, 0, 0};

inline void do_scroll(u8 x, u8 y) {
    y;
    __WRITE_VDP_REG_UNSAFE(VDP_RSCX, -x);    
}

// [CS] The importance of console logging functions to discover what is going on in your code.
void vblank_isr(void) {
    if (vdp_busy) return;

    __WRITE_VDP_REG_UNSAFE(VDP_R10, 15);
    do_scroll(0, 0);
    // EMU_printf("VBLANK! VBLANK! VBLANK!\n");

    if (refresh_hud_score) {
        update_hud2(score1, score2, 4, 0);    
        refresh_hud_score = false;
    } 
    
    if (refresh_hud_hiscore) {
        update_hud2(highscore1, highscore2, 26, 0);
        refresh_hud_hiscore = false;
    }
}

// [CS] How to run a code at a specific event? Interruptions
// [CS] Why do we need to modify screen during refresh? Raster Effects
// [CS] Hot to optimize access to arrays? Pointers. What are pointers?
void scanline_isr(void) {
    if (vdp_busy) return;

    switch ((u8)VCOUNTER) {    
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

inline u16 wrap16(u16 value, u16 min, u16 max) {
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
u8 check_collision(GameObject* obj1, GameObject* obj2) {
    if (obj1->rect.left > obj2->rect.right ||
        obj2->rect.left > obj1->rect.right ||
        obj1->rect.top > obj2->rect.bottom ||
        obj2->rect.top > obj1->rect.bottom) {
        return false;
    }
    return true;
}

// ENTITY INIT FUNCTIONS ///////////////////////////////////////////////////////

void init_item(void) {
    item.active = true;
    item.visible = false;
    item.collidable = false;
    item.x = 0;
    item.y = SPRITE_HIDDEN;
    item.spy = 1;
    item.counter = 10;
    item.sat_idx = ITEM_SAT;
    item.tile_index = ITEM_TILE_IDX;
    item.state = 0xff;

    item_change_state(ITEM_STATE_WAIT, ITEM_DELAY_WAIT);
    
    set_sprite_tile(ITEM_SAT, ITEM_TILE_IDX);
    set_sprite_tile(ITEM_SAT+1, ITEM_TILE_IDX+2);
}

void init_jet(void) {
    jet.active = true;
    jet.visible = true;
    jet.x = 0;
    jet.y = SPRITE_HIDDEN;
    jet.sat_idx = JET_SAT;
    jet.tile_index = JET_TILE_IDX;
    jet.state = JET_STATE_NORMAL;
    
    set_sprite_tile(JET_SAT, JET_TILE_IDX);
    set_sprite_tile(JET_SAT+1, JET_TILE_IDX+2);
}

void init_player(void) {
    player.active  = true;
    player.visible = true;
    player.fx = intTofixU(SCREEN_WIDTH/2);
    player.fy = intTofixU(SCREEN_HEIGHT/2 + SCREEN_HEIGHT/4); // 1/2 + 1/4 = 3/4
    player.fspx = PLAYER_SPEED;
    // player.fspx = 0;
    player.fspy = 0;
    player.sat_idx = PLAYER_SAT;
    player.tile_index = PLAYER_TILE_IDX;
    player.dir = DIR_NONE;
    player.state = 0xff;

    player_shield = PLAYER_SHIELD;

    player_change_state(&player, PLAYER_STATE_RUN, 1);
    // player.state = PLAYER_STATE_RUN;
    // player.state_update = player_state_run;

    set_sprite_tile(PLAYER_SAT, PLAYER_TILE_IDX);
    set_sprite_tile(PLAYER_SAT+1, PLAYER_TILE_IDX+2);

    init_jet();
}

void init_bats(void) {
    // [CS] Why do we need to set a tile index?
    j = BAT_SAT_START;       // tile index (starts after player, jet and item)
    u8 visible = false;
    i8 enemy_level = 1;
    for (i = 0; i < MAX_BATS; ++i) {
        GameObject* bat = &bats[i];
        bat->active = false;
        #ifdef FLICKER
            bat->visible = visible;
            visible != visible; // this alternate enemies in list
        #else
            bat->visible = true;
        #endif
        
        // bat->x = 0;
        // bat->y = SPRITE_HIDDEN;
        bat->x = (rand() % (SCREEN_WIDTH-SCREEN_LEFT-SPRITE_WIDTH)) + (SCREEN_LEFT + SPRITE_WIDTH/2);
        bat->y = (rand() % (SCREEN_HEIGHT-SCREEN_TOP-SPRITE_WIDTH)) + (SCREEN_TOP  + SPRITE_WIDTH/2);
        
        // speed increases at each 1/3 of MAX_BATS
        if ((i+1) % (10) == 0) {
            ++enemy_level;
        }

        // define 1 of 4 different diagonals
        switch (enemy_level) {
        case BAT_TYPE_1:
            switch(rand() % 4) {
            case 0: bat->spx = 1;  bat->spy = 1;  break;
            case 1: bat->spx = -1; bat->spy = 1;  break;
            case 2: bat->spx = 1;  bat->spy = -1; break;
            case 3: bat->spx = -1; bat->spy = -1; break;
        }
        break;
        case BAT_TYPE_2:
        switch(rand() % 2) {
            case 0: bat->spx = 3;  bat->spy = 0;  break;
            case 1: bat->spx = -3; bat->spy = 0;  break;
            }
            break;
        break;
        case BAT_TYPE_3:
        switch(rand() % 2) {
            case 0: bat->spx = 0;  bat->spy = 3;  break;
            case 1: bat->spx = 0; bat->spy = -3;  break;
            }
            break;
        break;
        }

        // if (!bat->spx && !bat->spy) bat->spx = 3;
        // bat->spx = 0;
        // bat->spy = 0;
        
        bat->tile_index = BAT_TILE_IDX;
        bat->frame_delay = i % ENTITY_FRAME_DELAY + 1; // count = 10
        bat->frame_current = 0;
        bat->frame_total = 2;
        bat->frame_count = bat->frame_total;
        bat->sat_idx = j;
        bat->flicker_group = i % FLICKER_GROUPS;
        bat->state = 0xff;

        bat_change_state(bat, BAT_STATE_RUN, 1);
        
        set_sprite_tile(bat->sat_idx, BAT_TILE_IDX + (enemy_level-1)*4);
        set_sprite_tile(bat->sat_idx+1, BAT_TILE_IDX + 2 + (enemy_level-1)*4);
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
    jet.visible = !jet.visible;
    if (jet.visible) {
        move_sprite(JET_SAT, jet.x - SPRITE_HALF, jet.y - SPRITE_HALF);
        move_sprite(JET_SAT+1, jet.x - SPRITE_HALF + 8, jet.y - SPRITE_HALF);
    } else {
        move_sprite(JET_SAT, 0, SPRITE_HIDDEN);
        move_sprite(JET_SAT+1, 0, SPRITE_HIDDEN);
    }
}

void update_player(GameObject* ply) {
    // update current state
    ply->state_update(ply);

    // draw
    if (ply->visible) {
        move_sprite(PLAYER_SAT, ply->x - SPRITE_HALF, ply->y - SPRITE_HALF);
        move_sprite(PLAYER_SAT+1, ply->x - SPRITE_HALF + 8, ply->y - SPRITE_HALF);
    } else {
        move_sprite(PLAYER_SAT, 0, SPRITE_HIDDEN);
        move_sprite(PLAYER_SAT+1, 0, SPRITE_HIDDEN);
    }
}

void player_change_state(GameObject* ply, u8 newstate, u8 counter) {
    if (newstate != ply->state) {
        if (newstate >= PLAYER_NUMBER_OF_STATES) return;  // check for error
        ply->state = newstate;
        ply->state_counter = counter;
        ply->state_update = player_state_fun_array[newstate];
    }
}

void on_player_hit(GameObject* ply) {
    set_sprite_tile(PLAYER_SAT, EXPLOSION_TILE_IDX);
    set_sprite_tile(PLAYER_SAT+1, EXPLOSION_TILE_IDX+2);
    
    // hide jet
    move_sprite(JET_SAT, 0, SPRITE_HIDDEN);
    move_sprite(JET_SAT+1, 0, SPRITE_HIDDEN);
    
    --player_shield;
    refresh_hud_score = true;

    set_volume(3, VOL_MAX);
    play_noise(NOISE_MODE_WHITE, NOISE_SHIFT_SLOW);

    player_change_state(ply, PLAYER_STATE_HIT, PLAYER_DELAY_EXPLOSION);
}

void on_player_boost(GameObject* ply) {
    ply->fspx = PLAYER_BOOST_SPEED;
    player_change_state(ply, PLAYER_STATE_BOOST, PLAYER_BOOST_TIME);
    
    jet.state = JET_STATE_BOOST;
    set_sprite_tile(JET_SAT, JET_BOOST_TILE_IDX);
    set_sprite_tile(JET_SAT+1, JET_BOOST_TILE_IDX+2);

    set_volume(3, VOL_50);
    play_noise(NOISE_MODE_WHITE, NOISE_SHIFT_MEDIUM);
}

void player_reset(GameObject* ply) {
    ply->fx = intTofixU(SCREEN_WIDTH/2);
    ply->fy = intTofixU(SCREEN_HEIGHT/2 + SCREEN_HEIGHT/4); // 1/2 + 1/4 = 3/4
    ply->fspx = PLAYER_SPEED;
    ply->visible = true;
    ply->dir = DIR_NONE;
    player_state_run(ply);  // updates screen position
    set_volume(3, VOL_MUTE);
        
    set_sprite_tile(PLAYER_SAT, PLAYER_TILE_IDX);
    set_sprite_tile(PLAYER_SAT+1, PLAYER_TILE_IDX+2);
    
    player_change_state(ply, PLAYER_STATE_INVINCIBLE, PLAYER_DELAY_INVINCIBLE);
}

void player_state_hit(GameObject* ply) {
    ply->visible = !ply->visible;
    --ply->state_counter;
    if (!ply->state_counter) {
        set_volume(3, VOL_MUTE);
        if (!player_shield) {
            init_gameover();
        } else {
            player_reset(ply);
        }
    }
}

void player_state_boost(GameObject* ply) {
    --ply->state_counter;
    if (!ply->state_counter) {
        ply->fspx = PLAYER_SPEED;
        jet.state = JET_STATE_NORMAL;

        set_sprite_tile(JET_SAT, JET_TILE_IDX);
        set_sprite_tile(JET_SAT+1, JET_TILE_IDX+2);
        player_change_state(ply, PLAYER_STATE_RUN, 1);

        set_volume(3, VOL_MUTE);
        set_palette_entry(1, 6, 0b00110100); //blue 00BBGGRR
        // play_noise(NOISE_MODE_WHITE, NOISE_SHIFT_FAST);
    }
    player_state_run(ply);
}

void player_state_invincible(GameObject* ply) {
    ply->visible = !ply->visible;
    --ply->state_counter;
    if (!ply->state_counter) {
        ply->visible = true;
        player_change_state(ply, PLAYER_STATE_RUN, 1);
    }
    player_state_run(ply);
}

/**
 * Movement with constant speed in 4 directions.
 */
void player_state_run(GameObject* ply) {
    if (key_pressed(JOY_P1_LEFT)) {
        set_sprite_tile(PLAYER_SAT, PLAYER_TILE_IDX+8);
        set_sprite_tile(PLAYER_SAT+1, PLAYER_TILE_IDX+10);
        ply->dir = DIR_LEFT;
    } else
    if (key_pressed(JOY_P1_RIGHT)) {
        set_sprite_tile(PLAYER_SAT, PLAYER_TILE_IDX);
        set_sprite_tile(PLAYER_SAT+1, PLAYER_TILE_IDX+2);
        ply->dir = DIR_RIGHT;
    } else
    if (key_pressed(JOY_P1_UP)) {
        set_sprite_tile(PLAYER_SAT, PLAYER_TILE_IDX+4);
        set_sprite_tile(PLAYER_SAT+1, PLAYER_TILE_IDX+6);
        ply->dir = DIR_UP;
    } else
    if (key_pressed(JOY_P1_DOWN)) {
        set_sprite_tile(PLAYER_SAT, PLAYER_TILE_IDX+12);
        set_sprite_tile(PLAYER_SAT+1, PLAYER_TILE_IDX+14);
        ply->dir = DIR_DOWN;
    }

    // if (ply->dir) {
    //     set_volume(3, VOL_75);
    //     play_noise(NOISE_MODE_WHITE, NOISE_SHIFT_MEDIUM);
    // }

    // move
    switch (ply->dir) {
        case DIR_LEFT:  ply->fx -= ply->fspx; break;
        case DIR_RIGHT: ply->fx += ply->fspx; break;
        case DIR_UP:    ply->fy -= ply->fspx; break;
        case DIR_DOWN:  ply->fy += ply->fspx; break;
    }
    // cache 8 bit positions
    ply->x = fixToint(ply->fx);
    ply->y = fixToint(ply->fy);

    // screen wrap
    // player->fy = wrap16(player->fy, intTofixU(SCREEN_LEFT), intTofixU(SCREEN_BOTTOM));
    
    // screen wrap
    u8 wrapped = false;
    if (ply->x < (SCREEN_LEFT+ENTITY_HALF)) {
        ply->x = (u8)(SCREEN_WIDTH-ENTITY_HALF);
        wrapped = true;
    } 
    else
    if (ply->x > SCREEN_WIDTH-ENTITY_HALF) {
        ply->x = SCREEN_LEFT+ENTITY_HALF;
        wrapped = true;
    }
    else
    if (ply->y < (SCREEN_TOP+ENTITY_HALF)) {
        ply->y = (u8)(SCREEN_HEIGHT-ENTITY_HALF);
        wrapped = true;
    } 
    else
    if (ply->y > SCREEN_HEIGHT-ENTITY_HALF) {
        ply->y = SCREEN_TOP+ENTITY_HALF;
        wrapped = true;
    }

    if (wrapped) {
        // update FP position and make player boost
        ply->fx = intTofixU(ply->x);
        ply->fy = intTofixU(ply->y);
        on_player_boost(ply);
    }

    // // draw
    // if (ply->visible) {
    //     move_sprite(PLAYER_SAT, ply->x - SPRITE_HALF, ply->y - SPRITE_HALF);
    //     move_sprite(PLAYER_SAT+1, ply->x - SPRITE_HALF + 8, ply->y - SPRITE_HALF);
    // } else {
    //     move_sprite(PLAYER_SAT, 0, SPRITE_HIDDEN);
    //     move_sprite(PLAYER_SAT+1, 0, SPRITE_HIDDEN);
    // }

    update_jet();
}

/**
 * Movement with acceleration (Asteroids)
 */
// void update_player2(void) {
//     if (key_down(JOY_P1_LEFT)) {
//         set_sprite_tile(PLAYER_SAT, PLAYER_TILE_IDX+8);
//         set_sprite_tile(PLAYER_SAT+1, PLAYER_TILE_IDX+10);
//         player.dir = DIR_LEFT;
//         player.fspx -= floatTofix(0.05f);
//     } else
//     if (key_down(JOY_P1_RIGHT)) {
//         set_sprite_tile(PLAYER_SAT, PLAYER_TILE_IDX);
//         set_sprite_tile(PLAYER_SAT+1, PLAYER_TILE_IDX+2);
//         player.dir = DIR_RIGHT;
//         player.fspx += floatTofix(0.05f);
//     }

//     if (key_down(JOY_P1_UP)) {
//         set_sprite_tile(PLAYER_SAT, PLAYER_TILE_IDX+4);
//         set_sprite_tile(PLAYER_SAT+1, PLAYER_TILE_IDX+6);
//         player.dir = DIR_UP;
//         player.fspy -= floatTofix(0.05f);
//     } else
//     if (key_down(JOY_P1_DOWN)) {
//         set_sprite_tile(PLAYER_SAT, PLAYER_TILE_IDX+12);
//         set_sprite_tile(PLAYER_SAT+1, PLAYER_TILE_IDX+14);
//         player.dir = DIR_DOWN;
//         player.fspy += floatTofix(0.05f);
//     }

//     // move
//     player.fx += player.fspx;
//     player.fy += player.fspy;

// // screen wrap
//     player.fy = wrap16(player.fy, intTofixU(SCREEN_LEFT), intTofixU(SCREEN_BOTTOM));

//     // cache 8 bit positions
//     // player.x = fixToint(player.fx + floatTofixU(0.5f));
//     // player.y = fixToint(player.fy + floatTofixU(0.5f));
//     player.x = fixToint(player.fx);
//     player.y = fixToint(player.fy);

//     // draw
//     if (player.visible) {
//         move_sprite(PLAYER_SAT, player.x - SPRITE_HALF, player.y - SPRITE_HALF);
//         move_sprite(PLAYER_SAT+1, player.x - SPRITE_HALF + 8, player.y - SPRITE_HALF);
//     } else {
//         move_sprite(PLAYER_SAT, 0, SPRITE_HIDDEN);
//         move_sprite(PLAYER_SAT+1, 0, SPRITE_HIDDEN);
//     }

//     update_jet();
// }

// BAT STATE FUNCTIONS ////////////////////////////////////////////////////////

void bat_change_state(GameObject* bat, u8 newstate, u8 counter) {
    if (newstate != bat->state) {
        if (newstate >= BAT_NUMBER_OF_STATES) return;  // check for error
        bat->state = newstate;
        bat->state_counter = counter;
        bat->state_update = bat_state_fun_array[newstate];
    }
}

void update_bat(GameObject* bat) {   
    bat->state_update(bat);

    // draw
    if (bat->visible) {
        move_sprite(bat->sat_idx, bat->x - SPRITE_HALF, bat->y - SPRITE_HALF);
        move_sprite(bat->sat_idx+1, bat->x - SPRITE_HALF + 8, bat->y - SPRITE_HALF);
    } else {
        move_sprite(bat->sat_idx, 0, SPRITE_HIDDEN);
        move_sprite(bat->sat_idx+1, 0, SPRITE_HIDDEN);
    }
}

void bat_state_appear(GameObject* bat) {
    --bat->counter;
    if (!bat->counter) {
        bat->counter = BAT_APPEAR_BLINK_INTERVAL;
        bat->visible = !bat->visible;
    }
    
    --bat->state_counter;
    if (!bat->state_counter) {
        bat_change_state(bat, BAT_STATE_RUN, 1);
    }
}

void bat_state_destroy(GameObject* _bat) {

}

// // use 16 bit X,Y as 8 bit for faster math
// void bat_state_run2(GameObject* bat) {
//     u8 x = (u8)bat->x;
//     u8 y = (u8)bat->y;
    
//     // move bat
//     x += bat->spx;
//     y += bat->spy;

//     // bounce off screen
//     if (x < (ENTITY_HALF+SCREEN_LEFT)) {    // 8 = first column width (hidden)
//         x = (ENTITY_HALF+SCREEN_LEFT);
//         bat->spx = -bat->spx;
//     } 
//     else
//     if (x > SCREEN_WIDTH-ENTITY_HALF) {
//         x = (u8)(SCREEN_WIDTH-ENTITY_HALF);
//         bat->spx = -bat->spx;
//     }

//     if (y < (ENTITY_HALF+SCREEN_TOP)) {    // 8 = HUD height
//         y = (ENTITY_HALF+SCREEN_TOP);
//         bat->spy = -bat->spy;
//     } 
//     else
//     if (y > SCREEN_HEIGHT-ENTITY_HALF) {
//         y = (u8)(SCREEN_HEIGHT-ENTITY_HALF);
//         bat->spy = -bat->spy;
//     }

//     #ifdef FLICKER
//         if (bat->flicker_group == flicker_counter) {
//             bat->visible = true;
//         } else {
//             bat->visible = false;
//         }
//         // bat->visible = !bat->visible;
//     #endif

//     bat->x = x;
//     bat->y = y;   
// }


// [CS] Why using pointer as param? Struct copy vs pointer
void bat_state_run(GameObject* bat) {   
    #ifdef FLICKER
        if (bat->flicker_group == flicker_counter) {
            bat->visible = true;
        } else {
            bat->visible = false;
            
            // Do not update bat movement at every 30 fps.
            // This helps to reduce bat speed without using
            // fixed point math.
            return;
        }
        // bat->visible = !bat->visible;
    #endif

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

    // animation
    // --bat->frame_delay;
    // if (!bat->frame_delay) {
    //     bat->frame_delay = BAT_FRAME_DELAY;
    //     bat->frame_current += METATILE;
    //     --bat->frame_count;
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
        bat = bats + MAX_BATS/2;
    
        // flicker
        ++flicker_counter;
        if (flicker_counter == FLICKER_GROUPS) {
            flicker_counter = 0;
        }
    }
    u8 i = MAX_BATS/2;
    do {
    // for (i = MAX_BATS/2; i; --i) {
        if (bat->active) {
            update_bat(bat);
            update_rect(bat);
            if (bat->state == BAT_STATE_RUN && player.state == PLAYER_STATE_RUN) {
                if (check_collision(&player, bat)) {
                    on_player_hit(&player);
                }
            }
        }
        ++bat;
    // }
        --i;
    } while (i);
    
    must_update = !must_update;
}

// ITEM STATE UPDATE FUNCTIONS /////////////////////////////////////////////////

void update_item(void) {
    // // float effect
    // --item.counter;
    // if (!item.counter) {
    //     item.spy = -item.spy;
    //     item.counter = 10;
    // }
    // item.y += item.spy;

    if (item.visible) {
        move_sprite(ITEM_SAT, item.x - SPRITE_HALF, item.y - SPRITE_HALF);
        move_sprite(ITEM_SAT+1, item.x - SPRITE_HALF + 8, item.y - SPRITE_HALF);
    } else {
        move_sprite(ITEM_SAT, 0, SPRITE_HIDDEN);
        move_sprite(ITEM_SAT+1, 0, SPRITE_HIDDEN);
    }

    // [CS] How to implement a flexibe state machine? Function pointers
    item.state_update(&item);
}

void item_change_state(u8 newstate, u8 counter) {
    if (newstate != item.state) {
        if (newstate >= ITEM_NUMBER_OF_STATES) return;  // check for error
        item.state = newstate;
        item.state_counter = counter;
        item.state_update = item_state_fun_array[newstate];
    }
}

void item_state_wait(GameObject* itm){
    --itm->state_counter;
    if (!itm->state_counter) {
        set_sprite_tile(itm->sat_idx, ITEM_TILE_IDX);
        set_sprite_tile(itm->sat_idx+1, ITEM_TILE_IDX+2);
        
        // rand position
        itm->x = rand() % (SCREEN_WIDTH - SCREEN_LEFT - SPRITE_WIDTH*2) + SPRITE_WIDTH;
        itm->y = rand() % (SCREEN_HEIGHT - SCREEN_TOP - SPRITE_WIDTH*2) + SPRITE_WIDTH;
        
        itm->counter = 5;
        item_change_state(ITEM_STATE_APPEAR, ITEM_DELAY_APPEAR);
    }
}

void item_state_appear(GameObject* itm) {
    --itm->counter;
    if (!itm->counter) {
        itm->counter = 5;
        itm->visible = !itm->visible;
    }
    
    --itm->state_counter;
    if (!itm->state_counter) {
        itm->collidable = true;
        itm->visible = true;       
        update_rect(itm);
        item_change_state(ITEM_STATE_IDLE, ITEM_DELAY_IDLE);
    }
}

void item_state_disappear(GameObject* itm) {
    itm->visible = !itm->visible;
    --itm->state_counter;
    if (!itm->state_counter) {
        itm->visible = false;
        item_change_state(ITEM_STATE_WAIT, ITEM_DELAY_WAIT);
        item_combo = false;
    }
}

void item_state_idle(GameObject* itm) {
    --itm->state_counter;
    if (!itm->state_counter) {
        itm->collidable = false;
        item_change_state(ITEM_STATE_DISAPPEAR, ITEM_DELAY_DISAPPEAR);
    }
}

void item_state_collected(GameObject* itm) {
    --itm->state_counter;
    if (!itm->state_counter) {
        itm->visible = false;
        item_change_state(ITEM_STATE_WAIT, 1);
        item_combo = true;
    }
}

void collect_item(GameObject* itm) {
    item_change_state(ITEM_STATE_COLLECTED, ITEM_DELAY_SHOW_PTS);
    
    // change to POINTS sprite
    if (item_combo) {
        score2 += ITEM_POINTS*2;
        set_sprite_tile(itm->sat_idx, ITEM_PTS_TILE_IDX+4);
        set_sprite_tile(itm->sat_idx+1, ITEM_PTS_TILE_IDX+2+4);
    } else {
        score2 += ITEM_POINTS;
        set_sprite_tile(itm->sat_idx, ITEM_PTS_TILE_IDX);
        set_sprite_tile(itm->sat_idx+1, ITEM_PTS_TILE_IDX+2);
    }
    game_change_state(GAMESTATE_ITEM, ITEM_FX_PAUSE);
    on_player_boost(&player);
    play_soundfx(SOUNDFX_ITEM);
    update_score();
}

// GENERAL INIT/UPDATE FUNCTIONS ///////////////////////////////////////////////

void load_tiles(u8 tiles_count, u8* tiles_data) {
    set_tiledata(tile_idx, tiles_count, tiles_data);
    tile_idx += tiles_count; // update next available tile idx in VRAM
}

// [CS] Why do we need to keep track of tile idx in VRAM? Free space managing
void load_map(const u8* const map_data, u8 idx_offset, u8 x, u8 y, u8 w, u8 h) {
    // set_bkg_tiles(0, 0, 32, 24, bg_map);
    
    const u8 *p_map = map_data;
    for (u8 j = 0; j < h; ++j) {
        for (u8 i = 0; i < w; ++i) {
            u8 idx = *p_map + idx_offset - 1;
            set_tile_xy(i+x, j+y, idx);
            ++p_map;
        }
    }   
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
    
    boost_color_counter = true;
    gamestate = 0xff;
    vdp_busy = false;
    
    init_title();
}

void init_font(void) {
    font_t min_font;

    font_init();
    min_font = font_load(font_min); // font_spect
    font_set(min_font);

    tile_idx += 36; // font_min has 36 tiles
}

void init_sound(void) {
    set_volume(0, VOL_MUTE);
    set_volume(1, VOL_MUTE);
    set_volume(2, VOL_MUTE);
    set_volume(3, VOL_MUTE);
}

void init_hud(u8 y) {
    gotoxy(1, y);
    puts("1P 000000   SHIELD 4  HI 000000");
    // puts("1P 000000   SHIELD 4  HI 000000");
    // printf(" SCORE          HIGHSCORE       ");
}

void update_hud2(u16 pts1, u16 pts2, u8 pos_x, u8 pos_y) {
    static char buf[7];
    u16 s1 = pts1;
    u16 s2 = pts2;
    
    // Process High Word (score1) - 2 digits
    buf[0] = '0'; while (s1 >= 10) { s1 -= 10; buf[0]++; }
    buf[1] = s1 + '0';

    // Process Low Word (score2) - 4 digits
    buf[2] = '0'; while (s2 >= 1000) { s2 -= 1000; buf[2]++; }
    buf[3] = '0'; while (s2 >= 100)  { s2 -= 100;  buf[3]++; }
    buf[4] = '0'; while (s2 >= 10)   { s2 -= 10;   buf[4]++; }
    buf[5] = s2 + '0';
    buf[6] = '\0';

    gotoxy(pos_x, pos_y);
    puts(buf);    

    // update shield
    buf[0] = '0' + player_shield;
    buf[1] = '\0';
    gotoxy(20, 0);
    puts(buf);
}

void update_hud(void) {
    // static score_buffer[7];
    
    // reset score in HUD
    gotoxy(7, 0);
    puts("000000");

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
        ++score1;
    }

    // check limit: 999999
    if (score1 > 99) {
        score1 = 99;
        score2 = 9999;
    }
    refresh_hud_score = true;
    // update_hud();
}

// [CS] Shared resource -> exclusive mode
// Moving from a Shared Resource model to an Exclusive Access model by
// turning DISPLAY OFF.
void enable_vdp(u8 enable) {
    if (enable) {
        DISPLAY_ON;
        // set_interrupts(VBL_IFLAG | LCD_IFLAG);
        // vdp_busy = false;
        // vsync();
        // SHOW_BKG;
        // SHOW_SPRITES;
    } else {
        // vdp_busy = true;
        // set_interrupts(0);
        DISPLAY_OFF;
        // vsync();
        // HIDE_SPRITES;
        // HIDE_BKG;
    }
}

// [CS] Since we are going to change game states, we need to init before
//      entering the state
void init_gameover(void) {
    play_music(MUSIC_GAMEOVER);

    // check for hi-score
    if (score1 == highscore1) {
        if (score2 > highscore2) {
            highscore1 = score1;
            highscore2 = score2;
        }
    } else if (score1 > highscore1) {
        highscore1 = score1;
        highscore2 = score2;
    }
    refresh_hud_hiscore = true;
    
    vsync();
    // set_palette_entry(1, 0, 0x00);
    HIDE_SPRITES;
    // turn off parallax
    // __WRITE_VDP_REG_UNSAFE(VDP_R10, R10_INT_OFF);

    set_interrupts(0);
    vdp_busy = true;
    
    // print game over
    gotoxy((SCREEN_TILES_W - 9)/2+1, 10);
    puts("         ");
    gotoxy((SCREEN_TILES_W - 9)/2+1, 11);
    puts("         ");
    gotoxy((SCREEN_TILES_W - 9)/2+1, 12);
    puts("GAME OVER");
    gotoxy((SCREEN_TILES_W - 9)/2+1, 13);
    puts("         ");
    
    vdp_busy = false;
    set_interrupts(VBL_IFLAG | LCD_IFLAG);

    game_change_state(GAMESTATE_GAMEOVER, GAMESTATE_DELAY_READY);
}

void wait(u8 time) {
    u8 counter = time;
    do {
        --counter;
        vsync();
    } while (counter);
}

void init_title(void) {
    vdp_busy = true;
    enable_vdp(false);
    
    // palette 0 = background
    set_tilepal(0, 16, bg_pal); 
    clrscreen();
    load_map(title_map, idx_title, 4, 5, 25, 8);
    
    vsync();
    init_hud(1);
    update_hud2(highscore1, highscore2, 26, 1);
    // refresh_hud_hiscore = true;
    
    gotoxy(6, 22);
    puts("MARCOS SILVANO  2026");
    gotoxy(9, 23);
    puts("MADE WITH GBDK");

    gamestate_counter = 10;

    // for (i = 0; SOUNDFX_ITEM[i] != END; i++) {
    //     EMU_printf("%d ", SOUNDFX_ITEM[i]);
    // }

    // wait(120);
    enable_vdp(true);
    game_change_state(GAMESTATE_TITLE, 1);

    play_music(MUSIC_INTRO);
}

void init_ready(void) {
    stop_music();
    enable_vdp(false); // vsync(), DISPLAY_OFF, vdp_busy = true
    
    clrscreen();
    gotoxy(12, SCREEN_TILES_H/2);
    puts("GET READY");
    
    // wait(120);
    enable_vdp(true); // vsync(), vdp_busy = false, DISPLAY_ON
    game_change_state(GAMESTATE_READY, GAMESTATE_DELAY_READY);
    play_music(MUSIC_READY);
}

void init_run(void) {
    stop_music();
    enable_vdp(false);
    
    // palette 0 = background
    set_tilepal(0, 16, bg_pal); 
    clrscreen();
    load_map(bg_map, idx_bg, 0, 0, SCREEN_TILES_W, SCREEN_TILES_H);
       
    reset_all_sprites();
    
    init_player();
    init_item();
    init_bats();
    
    init_hud(0);
    // refresh_hud_hiscore = true;
    update_hud2(highscore1, highscore2, 26, 0);
    
    bats_count = 0;
    bats_launch_counter = BAT_LAUNCH_DELAY;
    weaver_color_counter = 5;
    refresh_hud_score = false;
    refresh_hud_hiscore = false;

    // wait(120);
    enable_vdp(true);
    SHOW_SPRITES;
    vdp_busy = false;
    game_change_state(GAMESTATE_RUN, BAT_LAUNCH_DELAY);
}

// MAIN FUNCTION ///////////////////////////////////////////////////////////////

// [CS] Function arrays avoid switches around the code
void game_change_state(u8 newstate, u8 counter) {
    if (newstate != gamestate) {
        if (newstate >= GAMESTATE_NUMBER_OF_STATES) return;  // check for error
        gamestate = newstate;
        gamestate_counter = counter;
        gamestate_update = gamestate_fun_array[newstate];
    }
}

void gamestate_fun_title(void) {
    static u8 visible = true;

    --gamestate_counter;
    if (!gamestate_counter) {
        if (visible) {
            gotoxy(10, 16);
            puts("PRESS START");
        } else {
            gotoxy(10, 16);
            puts("           ");
        }
        visible = !visible;
        gamestate_counter = 10;       
    }
    
    if (key_pressed(JOY_P1_SW1)) {
        init_ready();
    }

    static u8 shift= NOISE_SHIFT_FAST;
    static u8 mode = NOISE_MODE_PERIODIC;
    static u16 tone= C1;

    if (key_pressed(JOY_P1_LEFT)) {
        shift++;
        shift &= 0x03;
        set_volume(3, VOL_MAX);
        play_noise(mode, shift);
    } else
    if (key_pressed(JOY_P1_RIGHT)) {
        mode++;
        mode &= 0x01;
        set_volume(3, VOL_MAX);
        play_noise(mode, shift);
    } else
    if (key_pressed(JOY_P1_UP)) {
        // set_volume(0, VOL_MAX);
        // PSG = 0x8A;
        // PSG = 0x1F;        
       play_soundfx(SOUNDFX_ITEM);
    } else
    if (key_pressed(JOY_P1_DOWN)) {
        play_music(MUSIC_READY);
        // for (i = 0; MUSIC_READY[i] != END; i++) {
        //     EMU_printf("%d ", MUSIC_READY[i]);
        // __asm__("nop");
        // set_volume(0, VOL_MUTE);
    }
}

void gamestate_fun_ready(void) {
    --gamestate_counter;
    if (!gamestate_counter) {
        init_run();
    }
}

void reset_all_sprites(void) {
    u8 i = 63;
    do {
        move_sprite(i, 0, SPRITE_HIDDEN);
        --i;
    } while (i);
    
    // for (i = 63; i; --i) {
    //     move_sprite(i, 0, SPRITE_HIDDEN);
    // }
    move_sprite(0, 0, SPRITE_HIDDEN);
}

void gamestate_fun_gameover(void) {
    --gamestate_counter;
    if (!gamestate_counter) {
        init_title();
    }
}

void gamestate_fun_item(void) {
    --gamestate_counter;
    if (!gamestate_counter) {
        game_change_state(GAMESTATE_RUN, 1);
        return;
    }
}

void gamestate_fun_pause(void) {
    if (key_pressed(J_START)) {
        game_change_state(GAMESTATE_RUN, 1);
    }
}

void launch_bat(void) {
    if (bats_count < BAT_MAX) {
        GameObject* bat = &bats[bats_count];
        
        bat->active = true;
        bat->counter = BAT_APPEAR_BLINK_INTERVAL;

        bat_change_state(bat, BAT_STATE_APPEAR, 120);

        ++bats_count;
    }
}

void gamestate_fun_run(void) {
    // if (key_pressed(J_START)) {
    //     printf("PAUSE\n");
    //     change_state(GAMESTATE_PAUSE, NO_WAIT);
    //     return;
    // }

    // background color rotation
    --weaver_color_counter;
    if (!weaver_color_counter) {
        weaver_color_counter = 10;

        u8 temp = weaver_colors[2];
        weaver_colors[2] = weaver_colors[1];
        weaver_colors[1] = weaver_colors[0];
        weaver_colors[0] = temp;
        
        set_palette_entry(0, 9, weaver_colors[0]);
        set_palette_entry(0, 10, weaver_colors[1]);
        set_palette_entry(0, 11, weaver_colors[2]);
    }
    // boost color blink
    if (player.state == PLAYER_STATE_BOOST) {
        if (boost_color_counter)
            set_palette_entry(1, 6, 0b00000011); //red  00BBGGRR
        else
            set_palette_entry(1, 6, 0b00110100); //blue 00BBGGRR
        boost_color_counter = !boost_color_counter;
    }

    // launch ball!
    if (!bats_launch_counter) { 
        bats_launch_counter = BAT_LAUNCH_DELAY;
        launch_bat();
    }
    --bats_launch_counter;
    
    // player.state_update(&player);
    update_player(&player);
    update_rect(&player);
    update_item();

    // priorize item collection (and boost invincibility)
    if (item.state == ITEM_STATE_IDLE) {
        if (check_collision(&player, &item)) {
            collect_item(&item);
        }
    }
    // [CS] In a old and very limited CPU, it's easy to see the difference that we can make by tweeks and optimization tricks.
    // [CS] Why do we need to create an export tool for images and maps? Planar vs chunky pixels.
    // [CS] Why using pointers is faster? Index access vs address access? 
    // [CS] Does counting up or down have the same cost?
    update_bats();

    
    update_background();
}

// MAIN FUNCTION //////////////////////////////////////////////////////////////

void main(void) {
    DISPLAY_OFF;
    HIDE_SPRITES;
    HIDE_BKG;
    
    SPRITES_8x16;
    init_font();
    init_sound();
    
    // palette 0 = background
    set_tilepal(0, 16, bg_pal); 
    
    idx_bg = tile_idx;
    load_tiles(BKG_TILES_COUNT, bg_tiles);
    idx_title = tile_idx;
    load_tiles(TITLE_TILES_COUNT, title_tiles);   
    
    init_game();

    set_sprite_4bpp_data(0, SPR_TILES_COUNT, sprites_data);
    // palette 1 = sprites
    set_tilepal(1, 16, sprites_palette); 

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
        update_music();
        update_soundfx();
        gamestate_update();
        vsync();
    }
}