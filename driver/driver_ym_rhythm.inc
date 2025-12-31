

; play back ym rhythm note-on/offs
;
; ix : ostinato_channels - needs to be preserved and advanced each loop
; hl : ostinato_orderdata - needs to be preserved and advanced each loop
;
ostinato_ym_rhythm:

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
	jr z, ostinato_ym_rhythm_continue
	
		; move hl on to next orderdata
		ld de, _sizeof_orderdata 
		add hl, de
		
		; move ix on to next channel
		ld de, _sizeof_channel
		add ix, de
		
		ret
	
	ostinato_ym_rhythm_continue:

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

    ; is this channel muted?
    bit CHANNEL_FLAGS_BIT_MUTED, (ix + channel.flags)
    jr z, update_ym_rhythm_not_muted

		; muted

        ; get info flag byte
        ; keep it in d
        ld a, (hl)
        inc hl
        ld d, a

        ; isolate flag bits
        and a, 0xe0
        jr z, update_ym_rhythm_muted_no_writes

            ; rhythm write?
            bit 7, d
            jr z, update_ym_rhythm_muted_no_write
            
                ld a, (hl)

                ; second rhythm write required? (msb of first rhythm byte set)
                bit 7, a
                jr z, update_ym_rhythm_muted_single

                    ; move pointer to second rhythm value
                    inc hl

                    ld a, (hl)

                update_ym_rhythm_muted_single:

				; keep latest rhythm value in volume for this channel
                ld (ix + channel.volume), a

                inc hl
            
            update_ym_rhythm_muted_no_write:
            
        update_ym_rhythm_muted_no_writes:

		jr ostinato_ym_rhythm_handle_waits

    update_ym_rhythm_not_muted:

		; not muted

        ; get info flag byte
        ; keep it in d
        ld a, (hl)
        inc hl
        ld d, a

        ; isolate flag bits
        and a, 0xe0
        jr z, update_ym_rhythm_no_writes

            ; rhythm write?
            bit 7, d
            jr z, update_ym_rhythm_no_write
            
                ; write to register
                ld a, 0x0e
                out (OPLL_REG_PORT), a

                ld a, (hl)
                out (OPLL_DATA_PORT), a

                push hl
                pop hl
                push hl
                pop hl
                push hl
                pop hl

                ; second rhythm write required? (msb of first rhythm byte set)
                bit 7, a
                jr z, update_ym_rhythm_single

                    ; move pointer to second rhythm value
                    inc hl

                    ld a, (hl)
                    out (OPLL_DATA_PORT), a

                update_ym_rhythm_single:

				; keep latest rhythm value in volume for this channel
                ld (ix + channel.volume), a

                inc hl
            
            update_ym_rhythm_no_write:
            
        update_ym_rhythm_no_writes:
		
	ostinato_ym_rhythm_handle_waits:

    ; isolate frame wait value
    ld a, d
    and a, 0x07

    ; long or short wait?
    bit 4, d
    jr z, update_ym_rhythm_short_wait

		; pattern pointer into de
		ex de, hl

		; restore orderdata pointer into hl
		pop hl
		
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
		
		; move ix on to next channel
		ld de, _sizeof_channel
		add ix, de

		ret

	update_ym_rhythm_short_wait:

		; pattern pointer into de
		ex de, hl

		; restore orderdata pointer into hl
		pop hl

		; special case where a wait of 0 means "wait for 8 frames"
		or a, a
		jr nz, uymrsw_not_zero

			ld a, 8

		uymrsw_not_zero:

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
					
		; move ix on to next channel
		ld de, _sizeof_channel
		add ix, de
		
		ret
