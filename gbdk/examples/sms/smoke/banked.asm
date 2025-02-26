;--------------------------------------------------------
; File Created by SDCC : free open source ISO C Compiler 
; Version 4.4.1 #14650 (MINGW64)
;--------------------------------------------------------
	.module banked
	.optsdcc -mz80
	
;--------------------------------------------------------
; Public variables in this module
;--------------------------------------------------------
	.globl b_banked_func
	.globl _banked_func
;--------------------------------------------------------
; special function registers
;--------------------------------------------------------
;--------------------------------------------------------
; ram data
;--------------------------------------------------------
	.area _DATA
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
	.area _CODE_3
;banked.c:5: uint16_t banked_func(uint8_t be, uint8_t ef) __banked {
;	---------------------------------
; Function banked_func
; ---------------------------------
	b_banked_func	= 3
_banked_func::
	push	ix
;banked.c:6: return ((be << 8) | ef);
	ld	hl, #0x7
	add	hl, sp
	ld	d, (hl)
	inc	hl
	ld	e, (hl)
;banked.c:7: }
	pop	ix
	ret
	.area _CODE_3
	.area _INITIALIZER
	.area _CABS (ABS)
