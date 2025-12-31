
; go through `b` ym2413 tracks and play them back
;
; b: number of channels
; ix : ostinato_channels - needs to be preserved and advanced each loop
; hl : ostinato_orderdata - needs to be preserved and advanced each loop
;
ostinato_ym:

	; get channel number in c
	ld c, 0

    ostinato_ym_loop:

        ; check if we're waiting
        ld e, (hl)
		inc hl
        ld d, (hl)

        ; decrease counter
        dec de
		
		; save it
        ld (hl), d
		dec hl
        ld (hl), e

        ; de == 0?
        ; if not then we're done for this frame
        ld a, e
        or a, d
        jr z, ostinato_ym_loop_continue

			; update channel number
			inc c

			; move hl on to next orderdata
			ld de, _sizeof_orderdata 
			add hl, de
			
			; move ix on to next channel
			ld de, _sizeof_channel
            add ix, de
			
			; loop or finish
			djnz ostinato_ym_loop
			
			ret

		ostinato_ym_loop_continue:

		; preserve ostinato_orderdata pointer
		push hl
		
		; move hl on to bank
		inc hl
		inc hl
		
		; change bank
		ld a, (hl)
		ld (SMS_MAPPER_SLOT_2), a
		
		; move hl on to pattern pointer
		inc hl

        ; load pattern pointer into de
        ld e, (hl)
		inc hl
        ld d, (hl)
		
		; get pattern pointer into hl
		ex de, hl

        ; channel muted?
        bit CHANNEL_FLAGS_BIT_MUTED, (ix + channel.flags)
        jr z, update_ym_not_muted

			; muted

			; get info flag byte
			; keep a copy in d
			ld a, (hl)
			inc hl
			ld d, a

			; isolate flag bits
			and a, 0xe0
			jr z, update_ym_muted_no_writes

				; volume write?
				bit 7, d
				jr z, update_ym_muted_no_volume_write

					ld a, (hl)
					inc hl
					ld (ix + channel.volume), a
				
				update_ym_muted_no_volume_write:
				
				; tone write (low)?
				bit 6, d
				jr z, update_ym_muted_no_tone_low_write

					ld a, (hl)
					inc hl
					ld (ix + channel.tone), a
				
				update_ym_muted_no_tone_low_write:
				
				; tone write (high) ?
				bit 5, d
				jr z, update_ym_muted_no_tone_high_write
					
					ld a, (hl)
					inc hl
					
					; does this need to a note-on too?
					bit 7, a
					jr z, update_ym_muted_tone_high_single_write

						or a, 0x10

					update_ym_muted_tone_high_single_write:

					ld (ix + channel.tone + 1), a
					
				update_ym_muted_no_tone_high_write:

			update_ym_muted_no_writes:

			jr ostinato_ym_handle_waits

        update_ym_not_muted:

			; not muted

			; get info flag byte
			; keep a copy in d
			ld a, (hl)
			inc hl
			ld d, a

			; isolate flag bits
			and a, 0xe0
			jr z, update_ym_no_writes

				; volume write?
				bit 7, d
				jr z, update_ym_no_volume_write

					; write to register
					ld a, 0x30
					add a, c
					out (OPLL_REG_PORT), a

					ld a, (hl)
					out (OPLL_DATA_PORT), a

					push hl
					pop hl
					push hl
					pop hl
					push hl
					pop hl

					inc hl
					ld (ix + channel.volume), a
				
				update_ym_no_volume_write:
				
				; tone write (low)?
				bit 6, d
				jr z, update_ym_no_tone_low_write
				
					; write to register
					ld a, 0x10
					add a, c
					out (OPLL_REG_PORT), a

					ld a, (hl)
					out (OPLL_DATA_PORT), a

					push hl
					pop hl
					push hl
					pop hl
					push hl
					pop hl

					inc hl
					ld (ix + channel.tone), a
				
				update_ym_no_tone_low_write:
				
				; tone write (high) ?
				bit 5, d
				jr z, update_ym_no_tone_high_write
					
					; write to register
					ld a, 0x20
					add a, c
					out (OPLL_REG_PORT), a

					ld a, (hl)
					out (OPLL_DATA_PORT), a
					
					; does this need to a note-on too?
					bit 7, a
					jr z, update_ym_tone_high_single_write

						push hl
						pop hl
						push hl
						pop hl
						push hl
						pop hl

						or a, 0x10
						out (OPLL_DATA_PORT), a

					update_ym_tone_high_single_write:

					inc hl
					ld (ix + channel.tone + 1), a
					
				update_ym_no_tone_high_write:

			update_ym_no_writes:
			
		ostinato_ym_handle_waits:

        ; isolate frame wait value
        ld a, d
        and a, 0x07

        ; long or short wait?
        bit 4, d
        jr z, update_ym_short_wait

			; pattern pointer into de
			ex de, hl

			; restore orderdata pointer into hl
			pop hl
			
			; preserve channel number
			push bc
			
			; preserve upper byte of wait time in c
			ld c, a
			
            ; get and store lower byte of wait time
			; move along pattern pointer
            ld a, (de)
			ld (hl), a
            inc de
			
			; store upper byte of wait time
			inc hl
			ld (hl), c
			inc hl
            
			; move past bank
			inc hl
			
            ; update pattern pointer
			; hl should be on to next orderdata after this
            ld (hl), e
			inc hl
            ld (hl), d
			inc hl
			
			; restore and update channel number
			pop bc
			inc c
			
			; move ix on to next channel
            ld de, _sizeof_channel
            add ix, de

			; loop or finish
            dec b
            jp nz, ostinato_ym_loop
			
			ret

        update_ym_short_wait:

			; pattern pointer into de
			ex de, hl

			; restore orderdata pointer into hl
			pop hl

			; special case where a wait of 0 means "wait for 8 frames"
			or a, a
			jr nz, uymsw_not_zero

				ld a, 8

			uymsw_not_zero:

			; lower byte of wait time
			ld (hl), a
			inc hl
			inc hl
			
			; move past bank
			inc hl
			
            ; update pattern pointer
			; hl should be on to next orderdata
            ld (hl), e
			inc hl
            ld (hl), d
			inc hl
			
			; update channel number
			inc c
						
			; move ix on to next channel
            ld de, _sizeof_channel
            add ix, de

			; loop or finish
            dec b
            jp nz, ostinato_ym_loop
			
			ret