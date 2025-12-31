
.define VDP_CONTROL_PORT 0xbf
.define VDP_DATA_PORT 0xbe

.define VDP_WRITE_ADDRESS 0x4000
.define VDP_WRITE_CRAM 0xc000
.define VDP_WRITE_REGISTER 0x8000

.MEMORYMAP
	SLOTSIZE $4000
	DEFAULTSLOT 0
	SLOT 0 $0000			; ROM slot 0.
	SLOT 1 $4000			; ROM slot 1.
	SLOT 2 $8000			; ROM slot 2
	SLOT 3 $C000			; RAM
.ENDME

.ROMBANKMAP
	BANKSTOTAL 4
	BANKSIZE $4000
	BANKS 4
.ENDRO

.RAMSECTION "Main Vars" bank 0 slot 3

	bios_byte: db
	last_input: db

	song_bank: db

.ENDS

.include "../../driver/ostinato_defines.asm"


.org 0x0000
    jp init

.org 0x0038

	push af
	exx
	
	ld c, VDP_CONTROL_PORT
	
	; acknowledge interrupt
	in a, (c)
	
	exx
	pop af
	
	ei
	reti

.org 0x0066
	retn

init:

	di
	im 1
    ld sp, 0xdfff

	; clear last_input byte
	xor a, a
	ld (last_input), a

	call ostinato_check_hardware

	; ym chip available?
	ld a, (ostinato_system_flags)
	and a, SYS_FLAG_HAS_YM
	jr z, +

		ld a, SYS_FLAG_HAS_YM
		call ostinato_init

		ld a, :test_ym_data
		ld (SMS_MAPPER_SLOT_2), a

		ld (song_bank), a

		ld hl, test_ym_data
		call ostinato_play

		jr ostinato_init_done

	+:

		ld a, SYS_FLAG_HAS_SN7
		call ostinato_init

		ld a, :test_data
		ld (SMS_MAPPER_SLOT_2), a

		ld (song_bank), a

		ld hl, test_data
		call ostinato_play

		jr ostinato_init_done


	ostinato_init_done:

	ei

	wait_vblank:
	
		halt

		; get input into c
		in a, (0xdc)
		cpl
		ld c, a

		; xor with last frame's input
		ld a, (last_input)
		xor a, c

		; and with this frame's input
		and a, c
		ld c, a

		main_mute_check:
		bit 0, c
		jr z, +

			ld a, 0
			call toggle_channel

			jr main_mute_check_done

		+:
		bit 1, c
		jr z, +

			ld a, 1
			call toggle_channel

			jr main_mute_check_done

		+:
		bit 2, c
		jr z, +

			ld a, 2
			call toggle_channel

			jr main_mute_check_done

		+:
		bit 3, c
		jr z, main_mute_check_done

			ld a, 3
			call toggle_channel

		main_mute_check_done:


		ld a, (song_bank)
		ld (SMS_MAPPER_SLOT_2), a
		
        call ostinato_update

		; update last_input variable
		in a, (0xdc)
		cpl
		ld (last_input), a

        jr wait_vblank

; a: channel
toggle_channel:

	push af
	call ostinato_get_channel_flags

	bit CHANNEL_FLAGS_BIT_MUTED, a
	jr z, +

		pop af
		call ostinato_unmute_channel

		ret

	+:

		pop af
		call ostinato_mute_channel

		ret

.incdir "../../driver"
.include "../../driver/ostinato.asm"


.include "test.asm"
.include "test_ym.asm"