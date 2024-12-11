#ifndef MAP_H
#define MAP_H

#include "utils.h"
#include "gameobject.h"

#define MAP_TILE_W 16
#define MAP_TILE_H 16
#define MAP_WIDTH  SCREEN_WIDTH/MAP_TILE_W
#define MAP_HEIGHT SCREEN_HEIGHT/MAP_TILE_H

extern const u8 const map[MAP_HEIGHT][MAP_WIDTH];

void map_setlevel(void);

inline bool map_has_tile(u8 x, u8 y);
inline bool map_check_obj(GameObject* obj);

#endif