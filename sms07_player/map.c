#include "map.h"
#include "gameobject.h"

// 16 x 12
const u8 const map[MAP_HEIGHT][MAP_WIDTH] = {
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,1,0,1,1,1,0,0},
    {0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0},
    {0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,1,1,1,0,0,1,1,1,1,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
};

void map_setlevel(void) {
    for (i = 0; i < MAP_HEIGHT; i++) {
        for (j = 0; j < MAP_WIDTH; j++) {
            if (map[i][j] == 1) set_tile2x2(j<<1, i<<1, 1);
        }
    }
}

inline bool map_has_tile(u8 x, u8 y) {
    return map[y>>4][x>>4];
}

inline bool map_check_obj(GameObject* obj) {
    m = (obj->x + obj->width -1)>>4;
    n = (obj->y + obj->height-1)>>4;
    return map[obj->y>>4][obj->x>>4] || map[obj->y>>4][m] || map[n][obj->x>>4] || map[n][m];
}

// inline bool map_get_colliding_tile(GameObject* obj, u8* tile_x, u8* tile_y) {
//     m = (obj->x + obj->width -1)>>4;
//     n = (obj->y + obj->height-1)>>4;
//     if (map[obj->y>>4][obj->x>>4]) {
//         *col_y = obj->y>>4;

//         return true;
//     }
//         || map[obj->y>>4][m] || map[n][obj->x>>4] || map[n][m];
// }