/**
 * GBDK + Emulicious
 * 
 * Compile: Ctrl + Shift + B
 * Run: F5
 * Debug: launch.json -> "stopOnEntry": true
 */

#include <gb/gb.h>
#include <stdio.h>
#include "NumberTiles.c"

void main() {
    // load tileset into GB memory
    set_bkg_data(0, 10, NumberTiles);

    // fill the background with 1st tile
    init_bkg(0);
    
    // show backghround
    SHOW_BKG;
    DISPLAY_ON;

    while(1) {
        scroll_bkg(1,0);
        delay(60);
    }
}