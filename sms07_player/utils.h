#ifndef UTILS_H
#define UTILS_H

#include <sms/sms.h>
#include <stdbool.h>
#include "gameobject.h"

#define SCREEN_WIDTH  256
#define SCREEN_HEIGHT 192
// 32 tiles wide
#define SCREEN_TILES_W SCREEN_WIDTH/8
// 24 tiles tall
#define SCREEN_TILES_H SCREEN_HEIGHT/8

typedef unsigned char u8;
typedef unsigned short u16;

extern u8 buttons;
extern u8 buttons_old;

//General iterators
extern u8 i, j, k;   // general for-loops
extern u8 m, n, p;   // macro single and nested for-loops

void clrscreen(void);
void set_tiledata(u8 tilestart, u8 tilecount, const u8* data);
void set_tilepal(u8 palstart, u8 colorcount, const u8* data);
void set_tile(u8 x, u8 y, u8 tile);
void set_tile2x2(u8 x, u8 y, u8 tilestart); // sets 16x16 metatile (4 tiles length)

// META SPRITES //////////////////////////////////////////////////////////
void set_metasprite_tiles(u8 sprite_id, u8 tile_start, u8 number_of_tiles);
void set_metasprite_pos(u8 sprite, u8 x, u8 y, u8 tiles_w, u8 tiles_h);

// specific for meta sprites of 4x4 sprites
void set_metasprite_tiles4x4(u8 sprite_id, u8 tile_start);
void set_metasprite_pos4x4 (u8 sprite_id, u8 x, u8 y);

// specific for meta sprites of 4x 8x16 sprites
void set_metasprite_tiles4x8x16(u8 sprite_id, u8 tile_start);
void set_metasprite_pos4x8x16 (u8 sprite_id, u8 x, u8 y);

// GENERAL ///////////////////////////////////////////////////////////////
bool is_bitset(u8 value, u8 bit);
void update(void);

// INPUT /////////////////////////////////////////////////////////////////
bool key_down(u8 key);
bool key_pressed(u8 key);
bool key_released(u8 key);

#endif