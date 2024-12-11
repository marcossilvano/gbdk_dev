/**
 * SMS GBDK + Emulicious
 * 
 * Compile: Ctrl + Shift + B
 * Run: F5
 * Debug: launch.json -> "stopOnEntry": true
 */

#include <sms/sms.h>
#include <stdio.h>

#define PRINT_BUTTON(BTN) { \
    if (buttons & BTN) {    \
        printf("* ");       \
    } else {                \
        printf("  ");       \
    }                       \
}

void main() {
    uint8_t buttons;

    printf("Master System Programming!\n\nPress any button:\n");

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