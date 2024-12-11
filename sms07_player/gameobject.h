#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H

#include "utils.h"

typedef struct GameObject {
    uint8_t x;
    uint8_t y;
    uint8_t nextx;
    uint8_t nexty;

    int8_t spdx;
    int8_t spdy;

    uint8_t right;
    uint8_t bottom;
    uint8_t width;
    uint8_t height;

    uint8_t tile_start;
    uint8_t tile_count;

    uint8_t sprite_id;
} GameObject;

inline void update_pos(GameObject* obj);

#endif