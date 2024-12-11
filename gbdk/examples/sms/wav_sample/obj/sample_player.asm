;--------------------------------------------------------
; File Created by SDCC : free open source ISO C Compiler 
; Version 4.4.1 #14650 (MINGW64)
;--------------------------------------------------------
	.module sample_player
	.optsdcc -mz80
	
;--------------------------------------------------------
; Public variables in this module
;--------------------------------------------------------
	.globl _play_sample
	.globl _cut_sample
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
;--------------------------------------------------------
; ram data
;--------------------------------------------------------
	.area _INITIALIZED
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
;src\sample_player.c:40: void play_sample(uint8_t * sample, uint16_t size) NAKED {
;	---------------------------------
; Function play_sample
; ---------------------------------
_play_sample::
;src\sample_player.c:97: __endasm;
	di
	push	hl
	pop	iy ; iy = hl
;	set tone to 0
	ld	c, #<_PSG
	xor	a
	ld	b, #(0b10000000 | 0b00000000) ; ch 0 tone 0
	out	(c), b
	out	(c), a
	ld	b, #(0b10000000 | 0b00100000) ; ch 1
	out	(c), b
	out	(c), a
	ld	b, #(0b10000000 | 0b01000000) ; ch 2
	out	(c), b
	out	(c), a
1$:
	ld	bc, #_voltable
	ld	h, #0
	ld	l, 0(iy)
	inc	iy
	add	hl, hl
	add	hl, bc ; hl = &_voltable[(sample[iy+0] * 2)]
	ld	a, (hl)
	.rept	4
	rlca
	.endm
	and	#0x0f
	or	#(0b10000000 | 0b00000000 | 0b00010000)
	ld	b, a
	ld	a, (hl)
	inc	hl
	and	#0x0f
	or	#(0b10000000 | 0b00100000 | 0b00010000)
	ld	c, #<_PSG
	out	(c), b
	out	(c), a
	outi
	ld	b, #19 ; delay, need to be calculated properly
2$:
	djnz	2$
	dec	de
	ld	a, e
	or	d
	jp	nz, 1$
	ei
	in	a, (_VDP_STATUS) ; cancel pending VDP interrupts
	ret
;src\sample_player.c:98: }
;src\sample_player.c:100: void cut_sample(uint8_t mask) NAKED {
;	---------------------------------
; Function cut_sample
; ---------------------------------
_cut_sample::
;src\sample_player.c:120: __endasm;
	ld	c, #<_PSG
	ld	b, a
	ld	e, #-1
1$:
	inc	e
	srl	b
	jr	c, 2$
	ret	z
	jr	1$
2$:
	ld	a, e
	.rept	3
	rrca
	.endm
	or	#(0b10000000 | 0b00010000 | 0x0f)
	out	(c), a
	jr	1$
;src\sample_player.c:121: }
	.area _CODE
_voltable:
	.db #0xff	; 255
	.db #0xdf	; 223
	.db #0xff	; 255
	.db #0xdf	; 223
	.db #0xef	; 239
	.db #0xdf	; 223
	.db #0xef	; 239
	.db #0xdf	; 223
	.db #0xdf	; 223
	.db #0xdf	; 223
	.db #0xcf	; 207
	.db #0xdf	; 223
	.db #0xcf	; 207
	.db #0xdf	; 223
	.db #0xbf	; 191
	.db #0xdf	; 223
	.db #0xaf	; 175
	.db #0xdf	; 223
	.db #0xaf	; 175
	.db #0xdf	; 223
	.db #0x9f	; 159
	.db #0xdf	; 223
	.db #0x9f	; 159
	.db #0xdf	; 223
	.db #0x9f	; 159
	.db #0xdf	; 223
	.db #0x8f	; 143
	.db #0xdf	; 223
	.db #0x8f	; 143
	.db #0xdf	; 223
	.db #0x8f	; 143
	.db #0xdf	; 223
	.db #0x8e	; 142
	.db #0xdf	; 223
	.db #0x8e	; 142
	.db #0xdf	; 223
	.db #0x8d	; 141
	.db #0xdf	; 223
	.db #0x8c	; 140
	.db #0xdf	; 223
	.db #0x8b	; 139
	.db #0xdf	; 223
	.db #0x8b	; 139
	.db #0xdf	; 223
	.db #0x8a	; 138
	.db #0xdf	; 223
	.db #0x8a	; 138
	.db #0xdf	; 223
	.db #0x89	; 137
	.db #0xdf	; 223
	.db #0x89	; 137
	.db #0xdf	; 223
	.db #0x88	; 136
	.db #0xdf	; 223
	.db #0x88	; 136
	.db #0xdf	; 223
	.db #0x88	; 136
	.db #0xdf	; 223
	.db #0x88	; 136
	.db #0xde	; 222
	.db #0x88	; 136
	.db #0xde	; 222
	.db #0x88	; 136
	.db #0xdd	; 221
	.db #0x88	; 136
	.db #0xdc	; 220
	.db #0x88	; 136
	.db #0xdc	; 220
	.db #0x88	; 136
	.db #0xdb	; 219
	.db #0x88	; 136
	.db #0xda	; 218
	.db #0x88	; 136
	.db #0xda	; 218
	.db #0x88	; 136
	.db #0xd9	; 217
	.db #0x88	; 136
	.db #0xd9	; 217
	.db #0x88	; 136
	.db #0xd9	; 217
	.db #0x88	; 136
	.db #0xd8	; 216
	.db #0x88	; 136
	.db #0xd8	; 216
	.db #0x88	; 136
	.db #0xd8	; 216
	.db #0x78	; 120	'x'
	.db #0xd8	; 216
	.db #0x78	; 120	'x'
	.db #0xd8	; 216
	.db #0x78	; 120	'x'
	.db #0xd8	; 216
	.db #0x77	; 119	'w'
	.db #0xd8	; 216
	.db #0x77	; 119	'w'
	.db #0xd8	; 216
	.db #0x77	; 119	'w'
	.db #0xd8	; 216
	.db #0x77	; 119	'w'
	.db #0xd8	; 216
	.db #0x77	; 119	'w'
	.db #0xd7	; 215
	.db #0x77	; 119	'w'
	.db #0xd7	; 215
	.db #0x77	; 119	'w'
	.db #0xd7	; 215
	.db #0x77	; 119	'w'
	.db #0xd7	; 215
	.db #0x67	; 103	'g'
	.db #0xd7	; 215
	.db #0x67	; 103	'g'
	.db #0xd7	; 215
	.db #0x67	; 103	'g'
	.db #0xd7	; 215
	.db #0x67	; 103	'g'
	.db #0xd7	; 215
	.db #0x66	; 102	'f'
	.db #0xd7	; 215
	.db #0x66	; 102	'f'
	.db #0xd7	; 215
	.db #0x66	; 102	'f'
	.db #0xd7	; 215
	.db #0x66	; 102	'f'
	.db #0xd7	; 215
	.db #0x66	; 102	'f'
	.db #0xd7	; 215
	.db #0x66	; 102	'f'
	.db #0xd6	; 214
	.db #0x66	; 102	'f'
	.db #0xd6	; 214
	.db #0x66	; 102	'f'
	.db #0xd6	; 214
	.db #0x66	; 102	'f'
	.db #0xd6	; 214
	.db #0x66	; 102	'f'
	.db #0xd6	; 214
	.db #0x56	; 86	'V'
	.db #0xd6	; 214
	.db #0x56	; 86	'V'
	.db #0xd6	; 214
	.db #0x56	; 86	'V'
	.db #0xd6	; 214
	.db #0x56	; 86	'V'
	.db #0xd6	; 214
	.db #0x56	; 86	'V'
	.db #0xd6	; 214
	.db #0x55	; 85	'U'
	.db #0xd6	; 214
	.db #0x55	; 85	'U'
	.db #0xd6	; 214
	.db #0x55	; 85	'U'
	.db #0xd6	; 214
	.db #0x55	; 85	'U'
	.db #0xd6	; 214
	.db #0x55	; 85	'U'
	.db #0xd6	; 214
	.db #0x55	; 85	'U'
	.db #0xd6	; 214
	.db #0x55	; 85	'U'
	.db #0xd5	; 213
	.db #0x55	; 85	'U'
	.db #0xd5	; 213
	.db #0x55	; 85	'U'
	.db #0xd5	; 213
	.db #0x55	; 85	'U'
	.db #0xd5	; 213
	.db #0x55	; 85	'U'
	.db #0xd5	; 213
	.db #0x55	; 85	'U'
	.db #0xd5	; 213
	.db #0x45	; 69	'E'
	.db #0xd5	; 213
	.db #0x45	; 69	'E'
	.db #0xd5	; 213
	.db #0x45	; 69	'E'
	.db #0xd5	; 213
	.db #0x45	; 69	'E'
	.db #0xd5	; 213
	.db #0x45	; 69	'E'
	.db #0xd5	; 213
	.db #0x45	; 69	'E'
	.db #0xd5	; 213
	.db #0x45	; 69	'E'
	.db #0xd5	; 213
	.db #0x44	; 68	'D'
	.db #0xd5	; 213
	.db #0x44	; 68	'D'
	.db #0xd5	; 213
	.db #0x44	; 68	'D'
	.db #0xd5	; 213
	.db #0x44	; 68	'D'
	.db #0xd5	; 213
	.db #0x44	; 68	'D'
	.db #0xd5	; 213
	.db #0x44	; 68	'D'
	.db #0xd5	; 213
	.db #0x44	; 68	'D'
	.db #0xd5	; 213
	.db #0x44	; 68	'D'
	.db #0xd4	; 212
	.db #0x44	; 68	'D'
	.db #0xd4	; 212
	.db #0x44	; 68	'D'
	.db #0xd4	; 212
	.db #0x44	; 68	'D'
	.db #0xd4	; 212
	.db #0x44	; 68	'D'
	.db #0xd4	; 212
	.db #0x44	; 68	'D'
	.db #0xd4	; 212
	.db #0x44	; 68	'D'
	.db #0xd4	; 212
	.db #0x44	; 68	'D'
	.db #0xd4	; 212
	.db #0x34	; 52	'4'
	.db #0xd4	; 212
	.db #0x34	; 52	'4'
	.db #0xd4	; 212
	.db #0x34	; 52	'4'
	.db #0xd4	; 212
	.db #0x34	; 52	'4'
	.db #0xd4	; 212
	.db #0x34	; 52	'4'
	.db #0xd4	; 212
	.db #0x34	; 52	'4'
	.db #0xd4	; 212
	.db #0x34	; 52	'4'
	.db #0xd4	; 212
	.db #0x34	; 52	'4'
	.db #0xd4	; 212
	.db #0x34	; 52	'4'
	.db #0xd4	; 212
	.db #0x33	; 51	'3'
	.db #0xd4	; 212
	.db #0x33	; 51	'3'
	.db #0xd4	; 212
	.db #0x33	; 51	'3'
	.db #0xd4	; 212
	.db #0x33	; 51	'3'
	.db #0xd4	; 212
	.db #0x33	; 51	'3'
	.db #0xd4	; 212
	.db #0x33	; 51	'3'
	.db #0xd4	; 212
	.db #0x33	; 51	'3'
	.db #0xd4	; 212
	.db #0x33	; 51	'3'
	.db #0xd4	; 212
	.db #0x33	; 51	'3'
	.db #0xd3	; 211
	.db #0x33	; 51	'3'
	.db #0xd3	; 211
	.db #0x33	; 51	'3'
	.db #0xd3	; 211
	.db #0x33	; 51	'3'
	.db #0xd3	; 211
	.db #0x33	; 51	'3'
	.db #0xd3	; 211
	.db #0x33	; 51	'3'
	.db #0xd3	; 211
	.db #0x33	; 51	'3'
	.db #0xd3	; 211
	.db #0x33	; 51	'3'
	.db #0xd3	; 211
	.db #0x33	; 51	'3'
	.db #0xd3	; 211
	.db #0x33	; 51	'3'
	.db #0xd3	; 211
	.db #0x23	; 35
	.db #0xd3	; 211
	.db #0x23	; 35
	.db #0xd3	; 211
	.db #0x23	; 35
	.db #0xd3	; 211
	.db #0x23	; 35
	.db #0xd3	; 211
	.db #0x23	; 35
	.db #0xd3	; 211
	.db #0x23	; 35
	.db #0xd3	; 211
	.db #0x23	; 35
	.db #0xd3	; 211
	.db #0x23	; 35
	.db #0xd3	; 211
	.db #0x23	; 35
	.db #0xd3	; 211
	.db #0x23	; 35
	.db #0xd3	; 211
	.db #0x23	; 35
	.db #0xd3	; 211
	.db #0x22	; 34
	.db #0xd3	; 211
	.db #0x22	; 34
	.db #0xd3	; 211
	.db #0x22	; 34
	.db #0xd3	; 211
	.db #0x22	; 34
	.db #0xd3	; 211
	.db #0x22	; 34
	.db #0xd3	; 211
	.db #0x22	; 34
	.db #0xd3	; 211
	.db #0x22	; 34
	.db #0xd3	; 211
	.db #0x22	; 34
	.db #0xd3	; 211
	.db #0x22	; 34
	.db #0xd3	; 211
	.db #0x22	; 34
	.db #0xd3	; 211
	.db #0x22	; 34
	.db #0xd3	; 211
	.db #0x22	; 34
	.db #0xd2	; 210
	.db #0x22	; 34
	.db #0xd2	; 210
	.db #0x22	; 34
	.db #0xd2	; 210
	.db #0x22	; 34
	.db #0xd2	; 210
	.db #0x22	; 34
	.db #0xd2	; 210
	.db #0x22	; 34
	.db #0xd2	; 210
	.db #0x22	; 34
	.db #0xd2	; 210
	.db #0x22	; 34
	.db #0xd2	; 210
	.db #0x22	; 34
	.db #0xd2	; 210
	.db #0x22	; 34
	.db #0xd2	; 210
	.db #0x22	; 34
	.db #0xd2	; 210
	.db #0x22	; 34
	.db #0xd2	; 210
	.db #0x22	; 34
	.db #0xd2	; 210
	.db #0x12	; 18
	.db #0xd2	; 210
	.db #0x12	; 18
	.db #0xd2	; 210
	.db #0x12	; 18
	.db #0xd2	; 210
	.db #0x12	; 18
	.db #0xd2	; 210
	.db #0x12	; 18
	.db #0xd2	; 210
	.db #0x12	; 18
	.db #0xd2	; 210
	.db #0x12	; 18
	.db #0xd2	; 210
	.db #0x12	; 18
	.db #0xd2	; 210
	.db #0x12	; 18
	.db #0xd2	; 210
	.db #0x12	; 18
	.db #0xd2	; 210
	.db #0x12	; 18
	.db #0xd2	; 210
	.db #0x12	; 18
	.db #0xd2	; 210
	.db #0x12	; 18
	.db #0xd2	; 210
	.db #0x12	; 18
	.db #0xd2	; 210
	.db #0x11	; 17
	.db #0xd2	; 210
	.db #0x11	; 17
	.db #0xd2	; 210
	.db #0x11	; 17
	.db #0xd2	; 210
	.db #0x11	; 17
	.db #0xd2	; 210
	.db #0x11	; 17
	.db #0xd2	; 210
	.db #0x11	; 17
	.db #0xd2	; 210
	.db #0x11	; 17
	.db #0xd2	; 210
	.db #0x11	; 17
	.db #0xd2	; 210
	.db #0x11	; 17
	.db #0xd2	; 210
	.db #0x11	; 17
	.db #0xd2	; 210
	.db #0x11	; 17
	.db #0xd2	; 210
	.db #0x11	; 17
	.db #0xd2	; 210
	.db #0x11	; 17
	.db #0xd2	; 210
	.db #0x11	; 17
	.db #0xd2	; 210
	.db #0x11	; 17
	.db #0xd1	; 209
	.db #0x11	; 17
	.db #0xd1	; 209
	.db #0x11	; 17
	.db #0xd1	; 209
	.db #0x11	; 17
	.db #0xd1	; 209
	.db #0x11	; 17
	.db #0xd1	; 209
	.db #0x11	; 17
	.db #0xd1	; 209
	.db #0x11	; 17
	.db #0xd1	; 209
	.db #0x11	; 17
	.db #0xd1	; 209
	.db #0x11	; 17
	.db #0xd1	; 209
	.db #0x11	; 17
	.db #0xd1	; 209
	.db #0x11	; 17
	.db #0xd1	; 209
	.db #0x11	; 17
	.db #0xd1	; 209
	.db #0x11	; 17
	.db #0xd1	; 209
	.db #0x11	; 17
	.db #0xd1	; 209
	.db #0x11	; 17
	.db #0xd1	; 209
	.db #0x11	; 17
	.db #0xd1	; 209
	.db #0x01	; 1
	.db #0xd1	; 209
	.db #0x01	; 1
	.db #0xd1	; 209
	.db #0x01	; 1
	.db #0xd1	; 209
	.db #0x01	; 1
	.db #0xd1	; 209
	.db #0x01	; 1
	.db #0xd1	; 209
	.db #0x01	; 1
	.db #0xd1	; 209
	.db #0x01	; 1
	.db #0xd1	; 209
	.db #0x01	; 1
	.db #0xd1	; 209
	.db #0x01	; 1
	.db #0xd1	; 209
	.db #0x01	; 1
	.db #0xd1	; 209
	.db #0x01	; 1
	.db #0xd1	; 209
	.db #0x01	; 1
	.db #0xd1	; 209
	.db #0x01	; 1
	.db #0xd1	; 209
	.db #0x01	; 1
	.db #0xd1	; 209
	.db #0x01	; 1
	.db #0xd1	; 209
	.db #0x01	; 1
	.db #0xd1	; 209
	.db #0x01	; 1
	.db #0xd1	; 209
	.db #0x00	; 0
	.db #0xd1	; 209
	.db #0x00	; 0
	.db #0xd1	; 209
	.db #0x00	; 0
	.db #0xd1	; 209
	.db #0x00	; 0
	.db #0xd1	; 209
	.db #0x00	; 0
	.db #0xd1	; 209
	.db #0x00	; 0
	.db #0xd1	; 209
	.db #0x00	; 0
	.db #0xd1	; 209
	.db #0x00	; 0
	.db #0xd1	; 209
	.db #0x00	; 0
	.db #0xd1	; 209
	.db #0x00	; 0
	.db #0xd1	; 209
	.db #0x00	; 0
	.db #0xd1	; 209
	.db #0x00	; 0
	.db #0xd1	; 209
	.db #0x00	; 0
	.db #0xd1	; 209
	.db #0x00	; 0
	.db #0xd1	; 209
	.db #0x00	; 0
	.db #0xd1	; 209
	.db #0x00	; 0
	.db #0xd1	; 209
	.db #0x00	; 0
	.db #0xd1	; 209
	.db #0x00	; 0
	.db #0xd1	; 209
	.db #0x00	; 0
	.db #0xd0	; 208
	.db #0x00	; 0
	.db #0xd0	; 208
	.db #0x00	; 0
	.db #0xd0	; 208
	.db #0x00	; 0
	.db #0xd0	; 208
	.db #0x00	; 0
	.db #0xd0	; 208
	.db #0x00	; 0
	.db #0xd0	; 208
	.db #0x00	; 0
	.db #0xd0	; 208
	.db #0x00	; 0
	.db #0xd0	; 208
	.area _INITIALIZER
	.area _CABS (ABS)
