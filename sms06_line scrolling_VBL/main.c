/**
 * SMS GBDK + Emulicious
 * 
 * Compile: Ctrl + Shift + B
 * Run: F5
 * Debug: launch.json -> "stopOnEntry": true
 */

//  #include <gb/gb.h>
 #include <sms/sms.h>
 #include <stdio.h>
 #include "city_tileset.h"
 
 unsigned char wave[] = {
     0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
     0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,
     0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,
     0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,
     0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,
     0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,
     0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,
     0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,
     0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,
     0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,
     0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
 };
 
 UBYTE wave_count = 0;
 uint16_t p1 = 0;
 
 void lcd_interrupt(void) {
     SCY_REG = 0;
     
     if (LY_REG < 90u) {
         SCX_REG = p1 >> 2;
         //SCY_REG = wave[wave_count] >> 1; // pq nao armazena o valor jah multiplicado por 2?
     } else
     if (LY_REG > 94u && LY_REG < 100u) {
         SCX_REG = p1 >> 1;
     } else
     if (LY_REG > 120u) {
         SCX_REG = p1;
     }
 }
 
 void vbl_interrupt(void) {
     p1++;
     wave_count++;
     if (wave_count > 87) {
         wave_count = 0;
     }
 }
 
 void main() {
     HIDE_SPRITES;
     HIDE_BKG;
     STAT_REG = 8;
 
     set_bkg_data(0, city_tileset_size, city_tileset);   // load tileset into GB memory
     set_bkg_tiles(0, 0, 32, 18, city_tilemap);          // fill screen with splashscreen map
     
     disable_interrupts();
     CRITICAL {
         STAT_REG = STATF_MODE00;
         add_LCD(lcd_interrupt);
         add_VBL(vbl_interrupt);
     }
     set_interrupts(LCD_IFLAG | VBL_IFLAG);
 
     SHOW_BKG;
     DISPLAY_ON;
 
     //uint8_t buttons;
 
     while(1) {
         uint8_t buttons = joypad();
 
         if (buttons & J_LEFT) {
             p1 += 1;
         } else 
         if (buttons & J_RIGHT) {
             p1 -= 3;
         }
         wait_vbl_done();
     }
 }