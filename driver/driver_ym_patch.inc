
; extra channel with patch
ostinato_ym_patch:

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
	jr z, ostinato_ym_patch_continue
		
		; this is the last channel so we don't need to do anything!
		ret

	ostinato_ym_patch_continue:

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
    jr z, update_ym_patch_not_muted

		; muted
        ; get info flag byte
        ld a, (hl)
        inc hl

        ; keep it in d
        ld d, a

        ; isolate flag bits
        and a, 0xe0
        jr z, update_ym_patch_muted_no_writes
            
            ; full patch write?
            bit 7, d
            jr z, update_ym_patch_muted_no_full_patch_write

				; preserve value of d
				ld c, d

				; number of registers
                ld b, 8
				
				; patch data array
				ld de, ostinato_ym_patch_data
				
                update_ym_patch_muted_patch_loop:
            
                    ld a, (hl)
                    inc hl

					; save patch to ostinato_ym_patch_data array
                    ld (de), a
					inc de

                    djnz update_ym_patch_muted_patch_loop
				
				; restore value of d
				ld d, c
				
				jr ostinato_ym_patch_handle_waits

            update_ym_patch_muted_no_full_patch_write:
			
			; partial patch write?
            bit 6, d
            jr z, update_ym_patch_muted_no_partial_patch_write
			
				; preserve d
				push de
				
				; number of registers
                ld b, 8
				
				; patch data array
				ld de, ostinato_ym_patch_data
				
				; get register flag byte into c
				ld c, (hl)
				inc hl

                update_ym_patch_muted_partial_patch_loop:
				
					; is this channel's bit set?
					bit 0, c
					jr z, update_ym_patch_muted_partial_patch_continue
				
						ld a, (hl)						
						inc hl
						
						; save patch to ostinato_ym_patch_data array
						ld (de), a
					
					update_ym_patch_muted_partial_patch_continue:
					
					; move along ostinato_ym_patch_data pointer
					inc de
					
					; shift c along so next channel's bit is the lsb
					srl c

                    djnz update_ym_patch_muted_partial_patch_loop
				
				; restore d
				pop de
					
			update_ym_patch_muted_no_partial_patch_write:
            
        update_ym_patch_muted_no_writes:

    update_ym_patch_not_muted:

		; not muted

        ; get info flag byte
        ld a, (hl)
        inc hl

        ; keep it in d
        ld d, a

        ; isolate flag bits
        and a, 0xe0
        jr z, update_ym_patch_no_writes
            
            ; full patch write?
            bit 7, d
            jr z, update_ym_patch_no_full_patch_write

				; preserve value of d
				ld c, d

				; number of registers
                ld b, 8
				
				; patch data array
				ld de, ostinato_ym_patch_data
				
                update_ym_patch_patch_loop:
            
                    ; write to register
                    ld a, 8
					sub a, b
                    out (OPLL_REG_PORT), a

                    ld a, (hl)
                    out (OPLL_DATA_PORT), a
                    
                    inc hl

					; save patch to ostinato_ym_patch_data array
                    ld (de), a
					inc de
					
                    push hl
                    pop hl
                    push hl
                    pop hl

                    djnz update_ym_patch_patch_loop
				
				; restore value of d
				ld d, c
				
				jr ostinato_ym_patch_handle_waits

            update_ym_patch_no_full_patch_write:
			
			; partial patch write?
            bit 6, d
            jr z, update_ym_patch_no_partial_patch_write
			
				; preserve d
				push de
				
				; number of registers
                ld b, 8
				
				; patch data array
				ld de, ostinato_ym_patch_data
				
				; get register flag byte into c
				ld c, (hl)
				inc hl

                update_ym_patch_partial_patch_loop:
				
					; is this channel's bit set?
					bit 0, c
					jr z, update_ym_patch_partial_patch_continue
				
						; write to register
						ld a, 8
						sub a, b
						out (OPLL_REG_PORT), a

						ld a, (hl)
						out (OPLL_DATA_PORT), a
						
						inc hl
						
						; save patch to ostinato_ym_patch_data array
						ld (de), a
						
						push hl
						pop hl
						push hl
						pop hl
					
					update_ym_patch_partial_patch_continue:
					
					; move along ostinato_ym_patch_data pointer
					inc de
					
					; shift c along so next channel's bit is the lsb
					srl c

                    djnz update_ym_patch_partial_patch_loop
				
				; restore d
				pop de
					
			update_ym_patch_no_partial_patch_write:
            
        update_ym_patch_no_writes:
		
	ostinato_ym_patch_handle_waits:

    ; isolate frame wait value
    ld a, d
    and a, 0x07

    ; long or short wait?
    bit 4, d
    jr z, update_ym_patch_short_wait

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
		
		ret

    update_ym_patch_short_wait:
	
		; pattern pointer into de
		ex de, hl

		; restore orderdata pointer into hl
		pop hl
		
		; special case where a wait of 0 means "wait for 8 frames"
		or a, a
		jr nz, uympsw_not_zero

			ld a, 8

		uympsw_not_zero:

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

		ret
