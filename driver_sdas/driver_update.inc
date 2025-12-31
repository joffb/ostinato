
_ostinato_update: 

    ; song playing?
    ld a, (ostinato_flags)
    bit SONG_FLAGS_BIT_PLAYING, a
    ret z

    push ix

    ; update order timer
    ld hl, (ostinato_order_timer)
    dec hl

    ; check if it's zero
    ; and we're at the end of the order
    ld a, l
    or a, h
    jr z, ostinato_update_next_order

    ; if we aren't, just update the timer
    ld (ostinato_order_timer), hl

    ostinato_update_channels: 
	
	ld ix, #ostinato_channels
	ld hl, #ostinato_orderdata

	; sn7 present?
    ld a, (ostinato_flags)
    bit 7, a
	jr z, su_no_sn7
    
        call ostinato_sn7
		
    su_no_sn7: 
	
	; ym2413 rhythm mode?
	ld a, (ostinato_flags)
	bit 5, a
	jr z, su_no_ym_rhythm
		
		ld b, #9
        call ostinato_ym
		call ostinato_ym_rhythm
		call ostinato_ym_patch
		
		jr ostinato_update_done

    su_no_ym_rhythm: 
	
	; ym2413 melody mode?
	ld a, (ostinato_flags)
	bit 6, a
	jr z, ostinato_update_done

		ld b, #9
        call ostinato_ym
		
        ; skip rhythm channel
		ld de, #_sizeof_channel
		add ix, de
		ld de, #_sizeof_orderdata
		add hl, de
		
        call ostinato_ym_patch

    ostinato_update_done: 

        pop ix

        ret 

    ostinato_update_next_order: 

        ; move to next order
        ld a, (ostinato_order_count)
        ld b, a
        ld a, (ostinato_order)
        inc a

        ; passed the last order
        cp a, b
        jr nz, ostinato_update_order_no_loop

            ld a, (ostinato_flags)
            bit SONG_FLAGS_BIT_LOOPING, a
            jr z, ostinato_update_no_looping
            
            xor a, a

        ostinato_update_order_no_loop: 

            call ostinato_change_order

            jr ostinato_update_channels


ostinato_update_no_looping: 

    call _ostinato_stop

    pop ix

    ret 
