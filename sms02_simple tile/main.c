/**
 * SMS GBDK + Emulicious
 * 
 * Compile: Ctrl + Shift + B
 * Run: F5
 * Debug: launch.json -> "stopOnEntry": true
 */

#include <sms/sms.h>
#include <stdio.h>
#include "tiles_number.c"

void main() {
    set_bkg_data(0, 10, number_tiles);
    
    // fill background with first tile
    // init_bkg(0);
    SHOW_BKG;
    DISPLAY_ON;

    while (1) {
        scroll_bkg(1,1);

        vsync();
    }
}