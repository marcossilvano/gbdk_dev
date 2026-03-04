/**
 * SMS GBDK + Emulicious
 * 
 * Compile: Ctrl + Shift + B
 * Run: F5
 * Debug: launch.json -> "stopOnEntry": true
 */

#include <sms/sms.h>
#include <gbdk/emu_debug.h>
#include <stdio.h>
#include <gbdk/font.h>

#define PRINT_BUTTON(BTN) { \
    if (buttons & BTN) {    \
        printf("* ");       \
    } else {                \
        printf("  ");       \
    }                       \
}

void main(void) {
    font_t min_font;
    uint8_t buttons;

    font_init();
    min_font = font_load(font_min); // font_spect
    font_set(min_font);

    // printf("Master System Programming!\n\nPress any button:\n");
    printf("Score 000140    HighScore 096750\n");

    EMU_TEXT("Hello");

    while (1) {
        buttons = joypad();
        printf("U D L R A B S T\n");
        
        PRINT_BUTTON(J_UP)
        PRINT_BUTTON(J_DOWN)
        PRINT_BUTTON(J_LEFT)
        PRINT_BUTTON(J_RIGHT)
        PRINT_BUTTON(J_A)
        PRINT_BUTTON(J_B)
        // PRINT_BUTTON(J_SELECT)
        // PRINT_BUTTON(J_START)

        printf("\n");

        while (joypad() == buttons); // hold until a different button is pressed
    }
}