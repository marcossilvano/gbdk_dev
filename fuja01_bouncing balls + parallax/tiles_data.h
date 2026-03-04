#ifndef TILES_NUMBER_H
#define TILES_NUMBER_H

#include <sms/sms.h>
#include <stdbool.h>

// first slot to use for this "tiles bank"
#define BGTILES_COUNT 22
#define SPRITES_COUNT 7*4

extern const uint8_t const sprites_palette[];
extern const uint8_t const sprites_data[];

extern const uint8_t const bg_pal[];
extern const uint8_t const bg_tiles[];
extern const uint8_t const bg_map[];

#endif