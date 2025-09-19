#ifndef UTILS_H
#define UTILS_H

#include <sms/sms.h>
#include <stdbool.h>

#define SCREEN_WIDTH  256
#define SCREEN_HEIGHT 192
#define SCREEN_TILES_W SCREEN_WIDTH/8
#define SCREEN_TILES_H SCREEN_HEIGHT/8

typedef unsigned char u8;
typedef unsigned short u16;

extern u8 buttons;
extern u8 buttons_old;

void clrscreen(void);
void set_tiledata(u8 tilestart, u8 tilecount, const u8* data);
void set_tilepal(u8 palstart, u8 colorcount, const u8* data);
void set_tile(u8 x, u8 y, u8 tile);
void set_tile2x2(u8 x, u8 y, u8 tilestart); // sets 16x16 metatile (4 tiles length)

bool is_bitset(u8 value, u8 bit);
void key_update(void);

bool key_down(u8 key);
bool key_pressed(u8 key);
bool key_released(u8 key);

#endif