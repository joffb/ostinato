

; go through the 4 sn7 tracks and play them back
;
; ix : ostinato_channels - needs to be preserved and advanced each loop
; hl : ostinato_orderdata - needs to be preserved and advanced each loop
;
ostinato_sn7: 

    ld b, #4

    ostinato_sn7_loop: 

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
        jr z, ostinato_sn7_loop_continue
					
			; move hl on to next orderdata
			ld de, #_sizeof_orderdata
			add hl, de
			
			; move ix on to next channel
			ld de, #_sizeof_channel
            add ix, de
			
			; loop or finish
			djnz ostinato_sn7_loop
			
			ret 

		ostinato_sn7_loop_continue: 

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
        bit CHANNEL_FLAGS_BIT_MUTED, channel.flags(ix)
        jr z, ostinato_sn7_not_muted

            ; muted

            ; get info flag byte
			; keep it in c
            ld a, (hl)
            inc hl
            ld c, a

            ; isolate flag bits
            and a, #0xe0
            jp z, ostinato_sn7_handle_waits

                ; volume write?
                bit 7, c
                jr z, ostinato_sn7_muted_no_volume_write
                
                    ld a, (hl)
                    inc hl
                    ld channel.volume(ix), a
                
                ostinato_sn7_muted_no_volume_write: 
                
                ; tone write (low)?
                bit 6, c
                jr z, ostinato_sn7_muted_no_tone_write

                    ld a, (hl)
                    inc hl
                    ld channel.tone(ix), a
                    
                    ; tone write (high) ?
                    bit 5, c
                    jr z, ostinato_sn7_muted_no_tone_high_write
                    
                        ld a, (hl)
                        inc hl
                        ld channel.tone+1(ix), a

                    ostinato_sn7_muted_no_tone_high_write: 
                    
                ostinato_sn7_muted_no_tone_write: 

            jr ostinato_sn7_handle_waits

        ostinato_sn7_not_muted: 

            ; not muted

            ; get info flag byte
            ld a, (hl)
            inc hl

            ; keep copy in c
            ld c, a

            ; isolate flag bits
            and a, #0xe0
            jr z, ostinato_sn7_no_writes

                ; volume write?
                bit 7, c
                jr z, ostinato_sn7_no_volume_write
                
                    ld a, (hl)
                    inc hl
                    ld channel.volume(ix), a
                    out (#SN76489_PORT), a
                
                ostinato_sn7_no_volume_write: 
                
                ; tone write (low)?
                bit 6, c
                jr z, ostinato_sn7_no_tone_write

                    ld a, (hl)
                    inc hl
                    ld channel.tone(ix), a
                    out (#SN76489_PORT), a
                    
                    ; tone write (high) ?
                    bit 5, c
                    jr z, ostinato_sn7_no_tone_high_write
                    
                        ld a, (hl)
                        inc hl
                        ld channel.tone+1(ix), a
                        out (#SN76489_PORT), a

                    ostinato_sn7_no_tone_high_write: 
                    
                ostinato_sn7_no_tone_write: 

            ostinato_sn7_no_writes: 

        ostinato_sn7_handle_waits: 

        ; isolate frame wait value
        ld a, c
        and a, #0x07

        ; long or short wait?
        bit 4, c
        jr z, update_sn_short_wait

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
            ld de, #_sizeof_channel
            add ix, de

			; loop or finish
            dec b
            jp nz, ostinato_sn7_loop
			
			ret 

        update_sn_short_wait: 

			; pattern pointer into de
			ex de, hl

			; restore orderdata pointer into hl
			pop hl
            
            ; special case where a wait of 0 means "wait for 8 frames"
            or a, a
            jr nz, usnsw_not_zero

                ld a, #8

            usnsw_not_zero: 

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
            ld de, #_sizeof_channel
            add ix, de

			; loop or finish
            dec b
            jp nz, ostinato_sn7_loop
			
			ret 
