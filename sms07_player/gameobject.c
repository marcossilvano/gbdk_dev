#include "gameobject.h"

inline void update_pos(GameObject* obj) {
    obj->right = obj->x + obj->width - 1;
    obj->bottom= obj->y + obj->height -1;
}