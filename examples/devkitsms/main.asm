;--------------------------------------------------------
; File Created by SDCC : free open source ISO C Compiler 
; Version 4.4.0 #14620 (Linux)
;--------------------------------------------------------
	.module main
	.optsdcc -mz80
	
;--------------------------------------------------------
; Public variables in this module
;--------------------------------------------------------
	.globl ___SMS__SDSC_signature
	.globl ___SMS__SDSC_descr
	.globl ___SMS__SDSC_name
	.globl ___SMS__SDSC_author
	.globl ___SMS__SEGA_signature
	.globl _main
	.globl _ostinato_looping_off
	.globl _ostinato_stop
	.globl _ostinato_update
	.globl _ostinato_play
	.globl _ostinato_init
	.globl _ostinato_check_hardware
	.globl _SMS_VRAMmemsetW
	.globl _SMS_getKeysPressed
	.globl _SMS_setBGPaletteColor
	.globl _SMS_waitForVBlank
	.globl _SMS_VDPturnOnFeature
	.globl _song_bank
	.globl _tic
	.globl _SMS_SRAM
	.globl _SRAM_bank_to_be_mapped_on_slot2
	.globl _ROM_bank_to_be_mapped_on_slot0
	.globl _ROM_bank_to_be_mapped_on_slot1
	.globl _ROM_bank_to_be_mapped_on_slot2
;--------------------------------------------------------
; special function registers
;--------------------------------------------------------
_SMS_VDPControlPort	=	0x00bf
;--------------------------------------------------------
; ram data
;--------------------------------------------------------
	.area _DATA
_ROM_bank_to_be_mapped_on_slot2	=	0xffff
_ROM_bank_to_be_mapped_on_slot1	=	0xfffe
_ROM_bank_to_be_mapped_on_slot0	=	0xfffd
_SRAM_bank_to_be_mapped_on_slot2	=	0xfffc
_SMS_SRAM	=	0x8000
_tic::
	.ds 1
_song_bank::
	.ds 1
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
;main.c:11: void main(void)
;	---------------------------------
; Function main
; ---------------------------------
_main::
;main.c:15: SMS_VRAMmemsetW(0, 0x0000, 16384);
	ld	hl, #0x4000
	push	hl
	ld	de, #0x0000
	ld	h, l
	call	_SMS_VRAMmemsetW
;main.c:16: SMS_setBGPaletteColor(0, RGBHTML(0x0000FF));
	ld	l, #0x30
;	spillPairReg hl
;	spillPairReg hl
	xor	a, a
	call	_SMS_setBGPaletteColor
;main.c:19: SMS_displayOn();
	ld	hl, #0x0140
	call	_SMS_VDPturnOnFeature
;main.c:21: ostinato_check_hardware();
	call	_ostinato_check_hardware
;main.c:23: if (ostinato_system_flags & SYS_FLAG_HAS_YM)
	ld	a, (_ostinato_system_flags+0)
	bit	1, a
	jr	Z, 00102$
;main.c:25: ostinato_init(SYS_FLAG_HAS_YM);
	ld	a, #0x02
	call	_ostinato_init
;main.c:27: song_bank = __bank_test_ym_data;
	ld	hl, #_song_bank
	ld	(hl), #0x02
;main.c:28: SMS_mapROMBank(song_bank);
	ld	hl, #_ROM_bank_to_be_mapped_on_slot2
	ld	(hl), #0x02
;main.c:29: ostinato_play(test_ym_data);
	ld	hl, #_test_ym_data
	call	_ostinato_play
	jr	00103$
00102$:
;main.c:33: ostinato_init(SYS_FLAG_HAS_SN7);
	ld	a, #0x01
	call	_ostinato_init
;main.c:35: song_bank = __bank_test_data;
	ld	hl, #_song_bank
	ld	(hl), #0x03
;main.c:36: SMS_mapROMBank(song_bank);
	ld	hl, #_ROM_bank_to_be_mapped_on_slot2
	ld	(hl), #0x03
;main.c:37: ostinato_play(test_data);   
	ld	hl, #_test_data
	call	_ostinato_play
00103$:
;main.c:40: ostinato_looping_off();
	call	_ostinato_looping_off
;main.c:42: tic = 0;
	ld	hl, #_tic
	ld	(hl), #0x00
00107$:
;main.c:47: SMS_waitForVBlank();
	call	_SMS_waitForVBlank
;main.c:49: keys = SMS_getKeysPressed();
	call	_SMS_getKeysPressed
;main.c:51: if (keys & 0x20)
	bit	5, e
	jr	Z, 00105$
;main.c:53: ostinato_stop();
	call	_ostinato_stop
00105$:
;main.c:56: SMS_mapROMBank(song_bank);
	ld	a, (_song_bank+0)
	ld	(_ROM_bank_to_be_mapped_on_slot2+0), a
;main.c:57: ostinato_update();
	call	_ostinato_update
;main.c:59: tic++;
	ld	hl, #_tic
	inc	(hl)
;main.c:61: }
	jr	00107$
	.area _CODE
__str_0:
	.ascii "joe k  "
	.db 0x00
__str_1:
	.ascii "ostinx test"
	.db 0x00
__str_2:
	.db 0x00
	.area _INITIALIZER
	.area _CABS (ABS)
	.org 0x3FF0
___SMS__SEGA_signature:
	.db #0x54	; 84	'T'
	.db #0x4d	; 77	'M'
	.db #0x52	; 82	'R'
	.db #0x20	; 32
	.db #0x53	; 83	'S'
	.db #0x45	; 69	'E'
	.db #0x47	; 71	'G'
	.db #0x41	; 65	'A'
	.db #0xff	; 255
	.db #0xff	; 255
	.db #0xff	; 255
	.db #0xff	; 255
	.db #0x99	; 153
	.db #0x99	; 153
	.db #0x00	; 0
	.db #0x4b	; 75	'K'
	.org 0x3FD8
___SMS__SDSC_author:
	.ascii "joe k  "
	.db 0x00
	.org 0x3FCC
___SMS__SDSC_name:
	.ascii "ostinx test"
	.db 0x00
	.org 0x3FCB
___SMS__SDSC_descr:
	.db 0x00
	.org 0x3FE0
___SMS__SDSC_signature:
	.db #0x53	; 83	'S'
	.db #0x44	; 68	'D'
	.db #0x53	; 83	'S'
	.db #0x43	; 67	'C'
	.db #0x01	; 1
	.db #0x00	; 0
	.db #0x00	; 0
	.db #0x00	; 0
	.db #0x00	; 0
	.db #0x00	; 0
	.db #0xd8	; 216
	.db #0x3f	; 63
	.db #0xcc	; 204
	.db #0x3f	; 63
	.db #0xcb	; 203
	.db #0x3f	; 63
