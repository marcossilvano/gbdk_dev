;--------------------------------------------------------
; File Created by SDCC : free open source ISO C Compiler 
; Version 4.4.1 #14650 (MINGW64)
;--------------------------------------------------------
	.module samptest
	.optsdcc -mz80
	
;--------------------------------------------------------
; Public variables in this module
;--------------------------------------------------------
	.globl _main
	.globl _cut_sample
	.globl _play_sample
	.globl _puts
	.globl _joypad
	.globl _vsync
	.globl _joy
	.globl _old_joy
;--------------------------------------------------------
; special function registers
;--------------------------------------------------------
_GG_STATE	=	0x0000
_GG_EXT_7BIT	=	0x0001
_GG_EXT_CTL	=	0x0002
_GG_SIO_SEND	=	0x0003
_GG_SIO_RECV	=	0x0004
_GG_SIO_CTL	=	0x0005
_GG_SOUND_PAN	=	0x0006
_MEMORY_CTL	=	0x003e
_JOY_CTL	=	0x003f
_VCOUNTER	=	0x007e
_PSG	=	0x007f
_HCOUNTER	=	0x007f
_VDP_DATA	=	0x00be
_VDP_CMD	=	0x00bf
_VDP_STATUS	=	0x00bf
_JOY_PORT1	=	0x00dc
_JOY_PORT2	=	0x00dd
_FMADDRESS	=	0x00f0
_FMDATA	=	0x00f1
_AUDIOCTRL	=	0x00f2
;--------------------------------------------------------
; ram data
;--------------------------------------------------------
	.area _DATA
_RAM_CONTROL	=	0xfffc
_GLASSES_3D	=	0xfff8
_MAP_FRAME0	=	0xfffd
_MAP_FRAME1	=	0xfffe
_MAP_FRAME2	=	0xffff
_old_joy::
	.ds 1
;--------------------------------------------------------
; ram data
;--------------------------------------------------------
	.area _INITIALIZED
_joy::
	.ds 1
;--------------------------------------------------------
; absolute external ram data
;--------------------------------------------------------
	.area _DABS (ABS)
;--------------------------------------------------------
; global & static initialisations
;--------------------------------------------------------
	.area _HOME
	.area _GSINIT
	.area _GSFINAL
	.area _GSINIT
;--------------------------------------------------------
; Home
;--------------------------------------------------------
	.area _HOME
	.area _HOME
;--------------------------------------------------------
; code
;--------------------------------------------------------
	.area _CODE
;src\samptest.c:20: void main(void) {
;	---------------------------------
; Function main
; ---------------------------------
_main::
;src\samptest.c:21: puts("PRESS A/B TO PLAY\n");
	ld	hl, #___str_0
	call	_puts
;src\samptest.c:23: while (true) {
00107$:
;src\samptest.c:14: old_joy = joy, joy = joypad();
	ld	a, (_joy+0)
	ld	(_old_joy+0), a
	call	_joypad
	ld	a, l
	ld	(_joy+0), a
;src\samptest.c:17: return ((joy & ~old_joy) & key);
	ld	a, (_old_joy+0)
	cpl
	ld	c, a
	ld	a, (_joy+0)
	and	a, c
	bit	5, a
	jr	Z, 00104$
;src\samptest.c:26: if (KEY_PRESSED(J_A)) {
;src\samptest.c:27: bank_save = CURRENT_BANK;
	ld	a, (_MAP_FRAME1+0)
	ld	c, a
;src\samptest.c:28: SWITCH_ROM(BANK(cowbell_8bit_pcm_unsigned));
	ld	hl, #_MAP_FRAME1
	ld	(hl), #<(___bank_cowbell_8bit_pcm_unsigned)
;src\samptest.c:29: play_sample(cowbell_8bit_pcm_unsigned, sizeof(cowbell_8bit_pcm_unsigned));
	push	bc
	ld	de, #0x1f80
	ld	hl, #_cowbell_8bit_pcm_unsigned
	call	_play_sample
	ld	a, #0x07
	call	_cut_sample
	pop	bc
;src\samptest.c:31: SWITCH_ROM(bank_save);
	ld	a, c
	ld	(#_MAP_FRAME1), a
	jr	00105$
00104$:
;src\samptest.c:17: return ((joy & ~old_joy) & key);
	bit	4, a
	jr	Z, 00105$
;src\samptest.c:33: } else if (KEY_PRESSED(J_B)) {
;src\samptest.c:34: bank_save = CURRENT_BANK;
	ld	a, (_MAP_FRAME1+0)
	ld	c, a
;src\samptest.c:35: SWITCH_ROM(BANK(risset_drum_8bit_pcm_unsigned));
	ld	hl, #_MAP_FRAME1
	ld	(hl), #<(___bank_risset_drum_8bit_pcm_unsigned)
;src\samptest.c:36: play_sample(risset_drum_8bit_pcm_unsigned, sizeof(risset_drum_8bit_pcm_unsigned));
	push	bc
	ld	de, #0x0e10
	ld	hl, #_risset_drum_8bit_pcm_unsigned
	call	_play_sample
	ld	a, #0x07
	call	_cut_sample
	pop	bc
;src\samptest.c:38: SWITCH_ROM(bank_save);
	ld	a, c
	ld	(#_MAP_FRAME1), a
00105$:
;src\samptest.c:40: vsync();
	call	_vsync
;src\samptest.c:42: }
	jr	00107$
___str_0:
	.ascii "PRESS A/B TO PLAY"
	.db 0x0a
	.db 0x00
	.area _CODE
	.area _INITIALIZER
__xinit__joy:
	.db #0x00	; 0
	.area _CABS (ABS)
