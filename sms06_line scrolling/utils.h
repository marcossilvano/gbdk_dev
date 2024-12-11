#ifndef UTILS_H
#define UTILS_H

#include <sms/sms.h>
#include <stdbool.h>

#define SCREEN_WIDTH  256u
#define SCREEN_HEIGHT 192u
#define SCREEN_TILES_W SCREEN_WIDTH/8
#define SCREEN_TILES_H SCREEN_HEIGHT/8

typedef unsigned char u8;
typedef unsigned short u16;

extern u8 buttons;
extern u8 buttons_old;

//General iterators
extern u8 i, j;      // general for-loops
extern u8 m, n;   // macro single and nested for-loops

void clrscreen(void);
void set_tiledata(u8 tilestart, u8 tilecount, const u8* data);
void set_tilepal(u8 palstart, u8 colorcount, const u8* data);
void set_tile(u8 x, u8 y, u8 tile);
void set_tile2x2(u8 x, u8 y, u8 tilestart); // sets 16x16 metatile (4 tiles length)

// META SPRITES //////////////////////////////////////////////////////////
inline void set_metasprite_tiles(u8 sprite_id, u8 tile_start, u8 number_of_tiles);
inline void set_metasprite_tiles4x4(u8 sprite_id, u8 tile_start);
inline void set_metasprite_pos(u8 sprite_id, u8 x, u8 y, u8 tiles_w, u8 tiles_h);
inline void set_metasprite_pos4x4 (u8 sprite_id, u8 x, u8 y);

// GENERAL ///////////////////////////////////////////////////////////////
inline bool is_bitset(u8 value, u8 bit);
inline void update_keys(void);

// INPUT /////////////////////////////////////////////////////////////////
inline bool key_down(u8 key);
inline bool key_pressed(u8 key);
inline bool key_released(u8 key);

#endif