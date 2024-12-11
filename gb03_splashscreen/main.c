/**
 * GBDK + Emulicious
 * 
 * Compile: Ctrl + Shift + B
 * Run: F5
 * Debug: launch.json -> "stopOnEntry": true
 */

#include <gb/gb.h>
//#include <nes/nes.h>
#include <stdio.h>
#include "splashscreen_tileset.h"

void main() {
    set_bkg_data(0, splashscreen_tileset_size, splashscreen_tileset);   // load tileset into GB memory
    set_bkg_tiles(0, 0, 20, 18, splashscreen_tilemap);          // fill screen with splashscreen map
    
    SHOW_BKG;
    DISPLAY_ON;

    uint8_t buttons;

    while(1) {
        uint8_t buttons = joypad();

        if (buttons & J_LEFT) {
            
        } else 
        if (buttons & J_RIGHT) {
            
        }
        wait_vbl_done();
    }
}