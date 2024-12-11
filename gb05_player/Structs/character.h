#ifndef CHARACTER_H
#define CHARACTER_H

#include <gb/gb.h>
#include <stdio.h>
#include "../defines.h"

// A structure for game characters
typedef struct GameObj {
    uint8_t sprite_id;
    
    uint8_t tile_width;
    uint8_t tile_height;
    uint8_t tile_start;

    uint8_t total_frames;
    uint8_t current_frame;

    fixed x;
    fixed y;
    fixed velx;
    fixed vely;

    const unsigned char* tileset;
    const unsigned char* tileprop;
    const unsigned char* tilemap;
} GameObj;

void GameObj_load_frame(GameObj* self, uint8_t frame) {
    self->current_frame = frame;

    uint8_t number_of_sprites = self->tile_width * self->tile_height;

    for (uint8_t m = 0; m != number_of_sprites; m++) {
        set_sprite_tile(self->sprite_id + m, self->tilemap[m]);
        set_sprite_prop(self->sprite_id + m, self->tileprop[m]);
    }
}

void GameObj_setup(GameObj* self, uint8_t sprite_id, 
                   uint8_t tile_width, uint8_t tile_height, uint8_t tile_start, 
                   uint8_t total_frames, const unsigned char* tileset, 
                   const unsigned char* tilemap, const unsigned char* tileprop) {
    self->sprite_id = sprite_id;

    self->tile_width = tile_width;
    self->tile_height = tile_height;
    
    self->tile_start = tile_start;
    self->total_frames = total_frames;

    self->x.w = 0;
    self->y.w = 0;
    
    self->tileset = tileset;
    self->tilemap = tilemap;
    self->tileprop = tileprop;

    GameObj_load_frame(self, 0);
}

void GameObj_update_metasprite(GameObj* self) {
    uint8_t idx = self->sprite_id;
    uint8_t xoffset = self->x.h;
    uint8_t yoffset = self->y.h;
    for (uint8_t m = 0; m < self->tile_width; m++) {
        for (uint8_t n = 0; n < self->tile_height; n++) {
            move_sprite(idx, xoffset, yoffset);
            idx++;
            xoffset += 8u;
        }
        xoffset = self->x.h;
        yoffset += 8u;
    }
}

void GameObject_keep_inside_screen(GameObj* self) {
    if (self->x.h < START_X) {
        self->x.h = START_X;
    } else
    if (self->x.h > SCREEN_W+START_X-(self->tile_width << 3)) { // width * 8
        self->x.h = SCREEN_W+START_X-(self->tile_width << 3);
    }

    if (self->y.h < START_Y) {
        self->y.h = START_Y;
    } else
    if (self->y.h > SCREEN_H+START_Y-(self->tile_height << 3)) {
        self->y.h = SCREEN_H+START_Y-(self->tile_height << 3);
    }
}

#endif