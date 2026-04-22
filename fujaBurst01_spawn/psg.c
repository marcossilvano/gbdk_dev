#include <sms/sms.h>
#include "utils.h"
#include "psg.h"

// static u8  psg_volume[] = {0x0f, 0x0f, 0x0f, 0x0f};
// static u16 psg_tone[] = {0, 0, 0};
// static u8  psg_mode = 0;
// static u8  psg_shift= 0;

static u8 noise_mode;
static u8 noise_shift;

static u8 music_wait    = 0;
static u8 music_playing = false;
static u8* music_pointer = 0;
static u8* music_address = 0;
static i16 music_bpm;

#define SOUNDFX_DELAY 2

static u8* soundfx_cmd[4] = {0,0,0,0};
static u8* soundfx_adr[4] = {0,0,0,0};
static u8  soundfx_wait[4]= {0,0,0,0};
static u8 soundfx_playing = 0;
static u8 soundfx_bpm = SOUNDFX_DELAY;

static u8 vol0;
static u8 vol1;
static u8 vol2;
static u8 vol3;
static u8 decay_counter;

#define DECAY_DELAY 0
// #define MUSIC_READY_BPM floatTofix(100.0f)
#define MUSIC_READY_BPM 3

/**
 * https://www.smspower.org/Development/SN76489?from=Development.PSG
 * 
 * PSG = port 0x7f is the PSG on the Master System
 *
 * PSG registers:
 *  4 x 4 bit volume registers
 *  3 x 10 bit tone registers
 *  1 x 3 bit noise register
 */

// [CS] How can we build something complex? 
// We start with the lower abstraction layers (play_tone, set_volume...) and then
// with this already working, we rely on this foundation to buid the next step (abstraction level)
// and we iterate this until we have what we need
// [CS] How can we understand and test before generalize?
// Make the example with literal hardcoded values. At successs, procede to generalization.
// E.g: PSG = 0b10110000, set max volume (0000) for channel 01
void play_tone(uint8_t channel, uint16_t tone) {
    // Tone Data (low 4 bits)
    // [1][cc][0][tttt] = Flag (1) + Set Tone (0) + Low 4 bits of tone
    PSG = 0b10000000 | (channel << 5) | (tone & 0b00001111);
    
    // Tone Data (high 6 bits)
    // [0][x][tttttt] = Flag(0) + Unnused + High 6 bits of tone
    PSG = (tone >> 4) & 0b00111111;
}

void set_volume(uint8_t channel, uint8_t volume) {
    // Latch Volume
    // [1][cc][1][vvvv] = Flag (1) + Channel + Set Volume (1) + Volume
    // PSG = 0b10010000 | (channel << 5) | (volume & 0x0f);
    PSG = 0x90 | (channel << 5) | (volume & 0x0f);
}

#define PSG_NOISE_REG 0xE0 // 1110 0000 (Channel 3, Noise Type)

// mode: 0 for Periodic, 1 for White Noise
// shift: 0 (Fast), 1 (Medium), 2 (Slow), 3 (Follow Channel 2)
void play_noise(uint8_t mode, uint8_t shift) {
    if (mode != noise_mode || shift != noise_shift) {
        noise_mode = mode;
        noise_shift= shift;
        // Play Noise
        // [1][11][0][0][mss] = Flag (1) + Channel 3 (11) + Type tone (0) + Unused + Mode + Shift
        PSG = 0b11100000 | (mode << 2) | (shift & 0b00000011);
    }
}

void play_tone_raw(u8 byte1, u8 byte2) {
    PSG = byte1;
    PSG = byte2;
}

void play_music(u8* pointer) {
    music_address = pointer;
    music_pointer = pointer;
    music_bpm = MUSIC_READY_BPM;
    
    vol0 = VOL_OFF;
    vol1 = VOL_OFF;
    vol2 = VOL_OFF;
    vol3 = VOL_OFF;
    decay_counter = DECAY_DELAY;
    
    music_playing = true;
}

void stop_music(void) {
    music_playing = false;
    // set_volume(0, VOL_MUTE);
    // set_volume(1, VOL_MUTE);
    // set_volume(2, VOL_MUTE);
    // set_volume(3, VOL_MUTE);
}

void volume_decay(void) {
    // volume decay
    if (decay_counter) {
        --decay_counter;
    } else {
        if (vol0 < 0x0f) vol0++;
        if (vol1 < 0x0f) vol1++;
        if (vol2 < 0x0f) vol2++;
        if (vol3 < 0x0f) vol3++;
        set_volume(0, vol0);
        set_volume(1, vol1);
        set_volume(2, vol2);
        set_volume(3, vol3);
        decay_counter = DECAY_DELAY;
    }
}

void update_music(void) {
    // bpm
    if (music_bpm > 0) {
        // music_bpm -= floatTofix(40);
        music_bpm -= 1;
        return;
    }
    music_bpm = MUSIC_READY_BPM;
    
    if (!music_playing) return;
    
    if (music_wait > 0) {
        --music_wait;
        return;
    }

    while (1) {
        u8 cmd = *music_pointer++;

        if (cmd == END) {           // end of song
            music_playing = false;
            break;
        }
        if (cmd == LOOP) {
            play_music(music_address);
            cmd = *music_pointer++;
        }
        
        if (!(cmd & 0x80)) {        // bit7 = 0 -> WAIT command
            music_wait = (cmd > 0)? cmd - 1 : 0;
            break;
        } 
        else {                      // bit7 = 1 -> note
            //play_tone_raw(cmd, *music_pointer);
            PSG = cmd;            // tone byte 1
            PSG = *music_pointer; // tone byte 2
            ++music_pointer;
            PSG = *music_pointer; // volume
            
            // [1][CC][1][vvvv]
            u8 ch = (*music_pointer & 0x60) >> 5;
            u8 vv = *music_pointer & 0x0f;
            // if (vv < 0x0e) vv += 2;
            switch (ch) {
                case 0: vol0 = vv; break;
                case 1: vol1 = vv; break;
                case 2: vol2 = vv; break;
            }
            ++music_pointer;
        }
    }
}

void play_soundfx(u8* pointer) {
    u8 channel = (*pointer & 0x60) >> 5;
    soundfx_cmd[channel] = pointer;
    soundfx_adr[channel] = pointer;
    soundfx_playing |= 1 << channel;
}

void stop_sound_fx(u8 channel) {
    // set_volume(channel, VOL_MUTE);
    soundfx_playing &= ~(1 << channel);
}

void update_soundfx(void) {
    --soundfx_bpm;
    if (soundfx_bpm) return;
    soundfx_bpm = SOUNDFX_DELAY;

    // point to last element
    // u8** ptr_cmd  = soundfx_cmd+3;
    u8* ptr_wait = soundfx_wait+3;

    // loop through all channels: 3,2,1,0
    u8 channel = 4;
    do {
        --channel;
        if (!(soundfx_playing & (1 << channel))) continue;

        if (*ptr_wait > 0) {
            --(*ptr_wait);
            return;
        }

        // u8* ptr = *ptr_cmd;
        u8* ptr = soundfx_cmd[channel];
        u8 cmd = *ptr;

        if (cmd == END) {
            soundfx_playing &= ~(1 << channel);
            set_volume(channel, VOL_OFF);
            continue;
        } 

        if (cmd == LOOP) {
            ptr = soundfx_adr[channel];
            soundfx_cmd[channel] = ptr;
            cmd = *ptr;
        }

        if (!(cmd & 0x80)) {        // bit7 = 0 -> WAIT command
            soundfx_wait[channel] = (cmd > 0)? cmd - 1 : 0;
            break;
        } 
        else {                      // bit7 = 1 -> note
            // play soundFX
            PSG = cmd;      // tone byte 1
            ++ptr;
            if ((*ptr & 0b01100000) != 0b01100000) { // no byte 2 for channel 3
                PSG = *ptr;
                ++ptr;
            }
            PSG = *ptr; // volume

            // prepare channel volume controll for decay
            // [1][CC][1][vvvv]
            u8 vv = *ptr & 0x0f;
            // if (vv == 0x0f) vv -= 2;
            switch (channel) {
                case 0: vol0 = vv; break;
                case 1: vol1 = vv; break;
                case 2: vol2 = vv; break;
                case 3: vol3 = vv; break;
            }
        }
        ++ptr;
        soundfx_cmd[channel] = ptr;
        // *ptr_cmd = ptr;
        // --ptr_cmd;
        --ptr_wait;

    } while (channel);
}

