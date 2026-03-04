#ifndef PSG_H
#define PSG_H

#include "utils.h"

#define VOL_MAX  0x00
#define VOL_75   0x04
#define VOL_50   0x08
#define VOL_25   0x0C
#define VOL_MUTE 0x0f

enum NoiseMode {
  NOISE_MODE_PERIODIC,
  NOISE_MODE_WHITE,
  
  NOISE_MODE_MAX
};

enum NodeShift {
  NOISE_SHIFT_FAST,
  NOISE_SHIFT_MEDIUM,
  NOISE_SHIFT_SLOW,
  NOISE_SHIFT_FOLLOW2,

  NOISE_SHIFT_MAX
};

// mode: 0 for Periodic, 1 for White Noise
// shift: 0 (Fast), 1 (Medium), 2 (Slow), 3 (Follow Channel 2)

// [CS] Indexed access is too slow in Z80.
// Is there a way to ask for the compiler to replace psgNotes[C4] 
// directly by the value 0x01A9? Put both in a .h file and define as static const.
/*
INDEXED ACCESS IN Z80:

ld hl, #_zSndNotes    ; Base address
ld a, #12             ; Index
add a, a              ; Multiply by 2 (u16)
ld e, a
ld d, #0
add hl, de            ; Add to base
ld a, (hl)            ; Fetch Low Byte
inc hl
ld h, (hl)            ; Fetch High Byte

CONSTANT VALUE:
ld hl, $01A9
*/

// [CS] How ensure that will be stored in ROM? Inlining and constant propagation
/*
Inlining and Constant Propagation:
- When you put a CONST array in a header file, every .c file that includes it 
  gets its own "view" of that data. If the compiler sees psg_notes[C4], 
  it performs Compile-Time Evaluation.
- static vs extern
  To ensure the compiler can actually "see" the values and optimize them, 
  you should declare the array as static const in your header file:

  // notes.h
  static const u16 psg_notes[] = {
    0x0353, 0x0323, ... 
  };
  Why static? Without static, if you include the header in multiple .c files, 
  the linker will complain about "Multiple Definitions" of the same array.
  With static, each .c file treats the array as its own private constant. 
  The compiler can then look at the code, see psg_notes[12], and replace it 
  with 0x01A9 immediately.
*/

// static const u16 const notes[]=
// {
//                                                                0x03F9,0x03C0,0x038A, //octave 0
// 0x0353,0x0323,0x02F6,0x02CB,0x02A3,0x027D,0x0259,0x0238,0x0218,0x01FA,0x01DD,0x01C2, //octave 1
// 0x01A9,0x0191,0x017B,0x0165,0x0151,0x013E,0x012D,0x011C,0x010C,0x00FD,0x00EF,0x00E1, //octave 2
// 0x00D4,0x00C8,0x00BD,0x00B2,0x00A8,0x009F,0x0097,0x008E,0x0086,0x007F,0x0078,0x0070, //octave 3
// 0x006A,0x0064,0x005E,0x0059,0x0054,0x0050,0x004C,0x0047,0x0043,0x0040,0x003C,0x0038, //octave 4
// 0x0035,0x0032,0x002F,0x002D,0x002A,0x0028,0x0026,0x0024,0x0022,0x0020,0x001E,0x001C, //octave 5
// 0x001A,0x0019,0x0018,0x0017,0x0016,0x0015,0x0013,0x0012,0x0011,0x0010,0x000F,0x000E  //octave 6
// };

// enum NotesLabels
// {
//                                           A0, As0, B0,
//   C1, Cs1, D1, Ds1, E1, F1, Fs1, G1, Gs1, A1, As1, B1,
//   C2, Cs2, D2, Ds2, E2, F2, Fs2, G2, Gs2, A2, As2, B2,
//   C3, Cs3, D3, Ds3, E3, F3, Fs3, G3, Gs3, A3, As3, B3,
//   C4, Cs4, D4, Ds4, E4, F4, Fs4, G4, Gs4, A4, As4, B4,
//   C5, Cs5, D5, Ds5, E5, F5, Fs5, G5, Gs5, A5, As5, B5,
//   C6, Cs6, D6, Ds6, E6, F6, Fs6, G6, Gs6, A6, As6, B6,
//   END
// };

enum NotesLabels {
  /* Octave 0 */
  A0 = 0x03F9, As0 = 0x03C0, B0 = 0x038A,

  /* Octave 1 */
  C1 = 0x0353, Cs1 = 0x0323, D1 = 0x02F6, Ds1 = 0x02CB, E1 = 0x02A3, F1 = 0x027D,
  Fs1 = 0x0259, G1 = 0x0238, Gs1 = 0x0218, A1 = 0x01FA, As1 = 0x01DD, B1 = 0x01C2,

  /* Octave 2 */
  C2 = 0x01A9, Cs2 = 0x0191, D2 = 0x017B, Ds2 = 0x0165, E2 = 0x0151, F2 = 0x013E,
  Fs2 = 0x012D, G2 = 0x011C, Gs2 = 0x010C, A2 = 0x00FD, As2 = 0x00EF, B2 = 0x00E1,

  /* Octave 3 */
  C3 = 0x00D4, Cs3 = 0x00C8, D3 = 0x00BD, Ds3 = 0x00B2, E3 = 0x00A8, F3 = 0x009F,
  Fs3 = 0x0097, G3 = 0x008E, Gs3 = 0x0086, A3 = 0x007F, As3 = 0x0078, B3 = 0x0070,

  /* Octave 4 */
  C4 = 0x006A, Cs4 = 0x0064, D4 = 0x005E, Ds4 = 0x0059, E4 = 0x0054, F4 = 0x0050,
  Fs4 = 0x004C, G4 = 0x0047, Gs4 = 0x0043, A4 = 0x0040, As4 = 0x003C, B4 = 0x0038,

  /* Octave 5 */
  C5 = 0x0035, Cs5 = 0x0032, D5 = 0x002F, Ds5 = 0x002D, E5 = 0x002A, F5 = 0x0028,
  Fs5 = 0x0026, G5 = 0x0024, Gs5 = 0x0022, A5 = 0x0020, As5 = 0x001E, B5 = 0x001C,

  /* Octave 6 */
  C6 = 0x001A, Cs6 = 0x0019, D6 = 0x0018, Ds6 = 0x0017, E6 = 0x0016, F6 = 0x0015,
  Fs6 = 0x0013, G6 = 0x0012, Gs6 = 0x0011, A6 = 0x0010, As6 = 0x000F, B6 = 0x000E
};

// [CS] Macros as a way to pack data efficiently

// 16 bit (8+8) play_tone() PSG format + 8 bit latcc volume
// PSG Format: [1][cc][0][nnnn], [0][x][nnnnnn] << 8
// Vol Format: 
#define NOTE(note, vol, ch) \
  (u8)(0x80 | ((ch) << 5) | ((note) & 0x0F)), /* [1][cc][0][tttt] */ \
  (u8)(((note) >> 4) & 0x3F),                 /* [0][x][tttttt]   */ \
  (u8)(0x90 | (ch << 5) | (vol & 0x0f))       /* [1][cc][1][vvvv] */

#define NOTEA(note, vol, ch) (0x80 | ((ch) << 5) | ((note) & 0x0F))
#define NOTEB(note, vol, ch) (((note) >> 4) & 0x3F)

// 8 bit for Duration (4 bits) and Silence (4 bits)
// #define TIME(dur, sil) (((dur) << 4) | ((sil) & 0x0F))

// 8 bit wait instruction
// FORMAT: [0][ttttttt]
#define WAIT(dur) ((dur) & 0x7f)

#define END 0xff
#define LOOP 0xef

// [CS] Multiplexing problem? 3 streams of data (3 channels) and one single hw output (PSG)
static const u8 const MUSIC_READY[] = {
  NOTE(A1, VOL_MAX, 0), 
  WAIT(1),
  NOTE(A0, VOL_MAX, 1),
  WAIT(5),
  
  NOTE(A1, VOL_MAX, 0), 
  WAIT(1),
  NOTE(A0, VOL_MAX, 1),
  WAIT(1),
  NOTE(A1, VOL_MAX, 0), 
  WAIT(1),
  NOTE(A0, VOL_MAX, 1),
  WAIT(1),
  NOTE(A1, VOL_MAX, 0), 
  WAIT(1),
  NOTE(A0, VOL_MAX, 1),
  WAIT(1),
  NOTE(C2, VOL_MAX, 0), 
  WAIT(1),
  NOTE(C1, VOL_MAX, 1),
  WAIT(5),
  
  NOTE(C2, VOL_MAX, 0), 
  WAIT(1),
  NOTE(C1, VOL_MAX, 1),
  WAIT(1),
  NOTE(C2, VOL_MAX, 0), 
  WAIT(1),
  NOTE(C1, VOL_MAX, 1),
  WAIT(1),
  NOTE(C2, VOL_MAX, 0), 
  WAIT(1),
  NOTE(C1, VOL_MAX, 1),
  WAIT(1),
  NOTE(D2, VOL_MAX, 0), 
  WAIT(1),
  NOTE(D1, VOL_MAX, 1),
  WAIT(5),

  NOTE(D2, VOL_MAX, 0), 
  WAIT(1),
  NOTE(D1, VOL_MAX, 1),
  WAIT(1),
  NOTE(D2, VOL_MAX, 0), 
  WAIT(1),
  NOTE(D1, VOL_MAX, 1),
  WAIT(1),
  NOTE(D2, VOL_MAX, 0), 
  WAIT(1),
  NOTE(D1, VOL_MAX, 1),
  WAIT(1),
  NOTE(D2, VOL_MAX, 0), 
  WAIT(1),
  NOTE(D1, VOL_MAX, 1),
  WAIT(5),

  END
};

static const u8 const MUSIC_GAMEOVER[] = {
  NOTE(D2, VOL_MAX, 1), 
  WAIT(1),
  NOTE(D1, VOL_MAX, 0),
  WAIT(5),

  NOTE(D2, VOL_MAX, 1), 
  WAIT(1),
  NOTE(D1, VOL_MAX, 0),
  WAIT(1),
  NOTE(D2, VOL_MAX, 1), 
  WAIT(1),
  NOTE(D1, VOL_MAX, 0),
  WAIT(1),
  NOTE(D2, VOL_MAX, 1), 
  WAIT(1),
  NOTE(D1, VOL_MAX, 0),
  WAIT(1),
  NOTE(C2, VOL_MAX, 1), 
  WAIT(1),
  NOTE(C1, VOL_MAX, 0),
  WAIT(5),

  NOTE(B1, VOL_MAX, 1), 
  WAIT(1),
  NOTE(B0, VOL_MAX, 0),
  WAIT(5),

  NOTE(A1, VOL_MAX, 1), 
  WAIT(1),
  NOTE(A0, VOL_MAX, 0),
  WAIT(5),

  NOTE(A1, VOL_MAX, 1), 
  WAIT(1),
  NOTE(A0, VOL_MAX, 0),
  WAIT(1),
  NOTE(A1, VOL_MAX, 1), 
  WAIT(1),
  NOTE(A0, VOL_MAX, 0),
  WAIT(1),
  NOTE(A1, VOL_MAX, 1), 
  WAIT(1),
  NOTE(A0, VOL_MAX, 0),
  WAIT(1),
  NOTE(A1, VOL_MAX, 1), 
  WAIT(1),
  NOTE(A0, VOL_MAX, 0),
  WAIT(5),

  END
};

static const u8 const MUSIC_INTRO[] = {
  // PATTERN 1 /////////

  NOTE(A0, VOL_MAX, 0), 
  NOTE(A2, VOL_MAX, 1), 
  WAIT(1),
  NOTE(A1, VOL_MAX, 2),
  WAIT(5),

  NOTE(A0, VOL_MAX, 0), 
  NOTE(A2, VOL_MAX, 1), 
  WAIT(1),
  NOTE(A1, VOL_MAX, 2),
  WAIT(1),  
  NOTE(A0, VOL_MAX, 0), 
  NOTE(A2, VOL_MAX, 1), 
  WAIT(1),
  NOTE(A1, VOL_MAX, 2),
  WAIT(1),  
  NOTE(A0, VOL_MAX, 0), 
  NOTE(A2, VOL_MAX, 1), 
  WAIT(1),
  NOTE(A1, VOL_MAX, 2),
  WAIT(1),  
  NOTE(C1, VOL_MAX, 0), 
  NOTE(C3, VOL_MAX, 1), 
  WAIT(1),
  NOTE(C2, VOL_MAX, 2),
  WAIT(5),
  
  NOTE(C1, VOL_MAX, 0), 
  NOTE(C3, VOL_MAX, 1), 
  WAIT(1),
  NOTE(C2, VOL_MAX, 2),
  WAIT(1),  
  NOTE(C1, VOL_MAX, 0), 
  NOTE(C3, VOL_MAX, 1), 
  WAIT(1),
  NOTE(C2, VOL_MAX, 2),
  WAIT(1),  
  NOTE(C1, VOL_MAX, 0), 
  NOTE(C3, VOL_MAX, 1), 
  WAIT(1),
  NOTE(C2, VOL_MAX, 2),
  WAIT(1),  
  NOTE(D1, VOL_MAX, 0), 
  NOTE(D3, VOL_MAX, 1), 
  WAIT(1),
  NOTE(D2, VOL_MAX, 2),
  WAIT(5),

  NOTE(D1, VOL_MAX, 0), 
  NOTE(D3, VOL_MAX, 1), 
  WAIT(1),
  NOTE(D2, VOL_MAX, 2),
  WAIT(1),  
  NOTE(D1, VOL_MAX, 0), 
  NOTE(D3, VOL_MAX, 1), 
  WAIT(1),
  NOTE(D2, VOL_MAX, 2),
  WAIT(1),  
  NOTE(D1, VOL_MAX, 0), 
  NOTE(D3, VOL_MAX, 1), 
  WAIT(1),
  NOTE(D2, VOL_MAX, 2),
  WAIT(1),  
  NOTE(D1, VOL_MAX, 0), 
  NOTE(D3, VOL_MAX, 1), 
  WAIT(1),
  NOTE(D2, VOL_MAX, 2),
  WAIT(5),  

  NOTE(D1, VOL_MAX, 0), 
  NOTE(D3, VOL_MAX, 1), 
  WAIT(1),
  NOTE(D2, VOL_MAX, 2),
  WAIT(5),  
  
  // PATTERN 2 /////////

  NOTE(A0, VOL_MAX, 0), 
  NOTE(A2, VOL_MAX, 1), 
  WAIT(1),
  NOTE(A1, VOL_MAX, 2),
  WAIT(5),

  NOTE(A0, VOL_MAX, 0), 
  NOTE(A2, VOL_MAX, 1), 
  WAIT(1),
  NOTE(A1, VOL_MAX, 2),
  WAIT(1),
  NOTE(A0, VOL_MAX, 0), 
  NOTE(A2, VOL_MAX, 1), 
  WAIT(1),
  NOTE(A1, VOL_MAX, 2),
  WAIT(1),
  NOTE(A0, VOL_MAX, 0), 
  NOTE(A2, VOL_MAX, 1), 
  WAIT(1),
  NOTE(A1, VOL_MAX, 2),
  WAIT(1),
  NOTE(F1, VOL_MAX, 0), 
  NOTE(F3, VOL_MAX, 1), 
  WAIT(1),
  NOTE(F2, VOL_MAX, 2),
  WAIT(5),

  NOTE(E1, VOL_MAX, 0), 
  NOTE(E3, VOL_MAX, 1), 
  WAIT(1),
  NOTE(E2, VOL_MAX, 2),
  WAIT(5),

  NOTE(D1, VOL_MAX, 0), 
  NOTE(D3, VOL_MAX, 1), 
  WAIT(1),
  NOTE(D2, VOL_MAX, 2),
  WAIT(5),

  NOTE(D1, VOL_MAX, 0), 
  NOTE(D3, VOL_MAX, 1), 
  WAIT(1),
  NOTE(D2, VOL_MAX, 2),
  WAIT(1),
  NOTE(D1, VOL_MAX, 0), 
  NOTE(D3, VOL_MAX, 1), 
  WAIT(1),
  NOTE(D2, VOL_MAX, 2),
  WAIT(1),
  NOTE(D1, VOL_MAX, 0), 
  NOTE(D3, VOL_MAX, 1), 
  WAIT(1),
  NOTE(D2, VOL_MAX, 2),
  WAIT(1),
  NOTE(C1, VOL_MAX, 0), 
  NOTE(C3, VOL_MAX, 1), 
  WAIT(1),
  NOTE(C2, VOL_MAX, 2),
  WAIT(1),
  NOTE(B0, VOL_MAX, 0), 
  NOTE(B2, VOL_MAX, 1), 
  WAIT(1),
  NOTE(B1, VOL_MAX, 2),
  WAIT(1),
  NOTE(C1, VOL_MAX, 0), 
  NOTE(C3, VOL_MAX, 1), 
  WAIT(1),
  NOTE(C2, VOL_MAX, 2),
  WAIT(1),
  NOTE(D1, VOL_MAX, 0), 
  NOTE(D3, VOL_MAX, 1), 
  WAIT(1),
  NOTE(D2, VOL_MAX, 2),
  WAIT(5),

  LOOP
};

static const u8 const SOUNDFX_ITEM[] = {
  NOTE(F3, VOL_MAX, 0), 
  NOTE(G3, VOL_MAX, 0), 
  NOTE(A3, VOL_MAX, 0), 
  NOTE(B3, VOL_MAX, 0), 
  END
};

// PROTOTYPES ////////////////////////////////////////////////////
extern void play_tone(uint8_t channel, uint16_t frequency);
extern void set_volume(uint8_t channel, uint8_t volume);
extern void play_noise(uint8_t mode, uint8_t shift);

extern void play_tone_raw(u8 byte1, u8 byte2);
extern void play_music(u8* pointer);
extern void stop_music(void);
extern void update_music(void);

extern void play_soundfx(u8* pointer);
extern void update_soundfx(void);

#endif