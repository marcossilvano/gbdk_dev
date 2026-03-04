#ifndef TILES_NUMBER_H
#define TILES_NUMBER_H

#include <sms/sms.h>
#include <stdbool.h>

#define BKG_TILES_COUNT 21
#define SPR_TILES_COUNT 4*13
#define TITLE_TILES_COUNT 28

#define TITLE_WIDH   25
#define TITLE_HEIGHT 8

extern const uint8_t const sprites_palette[];
extern const uint8_t const sprites_data[];

extern const uint8_t const bg_pal[];
extern const uint8_t const bg_tiles[];
extern const uint8_t const bg_map[];

extern const uint8_t const title_tiles[];
extern const uint8_t const title_map[];

#endif