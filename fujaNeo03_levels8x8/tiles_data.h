#ifndef TILES_NUMBER_H
#define TILES_NUMBER_H

#include <sms/sms.h>
#include <stdbool.h>

#define BKG_TILES_COUNT 29
#define SPR_TILES_COUNT 2*38
#define TITLE_TILES_COUNT 28

#define TITLE_WIDH   25
#define TITLE_HEIGHT 8

#define BG_ARRAY_SIZE 9  // size of top and bottom compact bg array
#define BG_BLACK_TILE 9  // black tile idx
#define BG_PIRAMID    4  // dimension of bg piramid

extern const uint8_t const sprites_palette[];
extern const uint8_t const sprites_data[];

extern const uint8_t const bg_pal[];
extern const uint8_t const bg_tiles[];
// extern const uint8_t const bg_map[];

extern const uint8_t const bg_top[];
extern const uint8_t const bg_bottom[];
extern const uint8_t const bg_piramid[];

extern const uint8_t const white_pal[];

extern const uint8_t const title_tiles[];
extern const uint8_t const title_map[];

extern const uint8_t const level_map[];

#endif