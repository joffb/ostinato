
; hl: song pointer
_ostinato_play: 

    ; copy song header data
    ld de, #ostinato_flags

	; ostinato_flags, ostinato_channel_count
	ldi 
    ldi 

	; ostinato_order_count, ostinato_orders
	ldi 
    ldi 
    ldi 

    ; initialise channel bank numbers in ostinato_orderdata
    ld a, (ostinato_channel_count)
    ld b, a

    ld c, #0xff
    ld de, #ostinato_orderdata+2

    ostinato_play_orderdata_bank_loop: 

        ; copy channel's bank
        ldi 

        ; move to next channel
        ld a, #_sizeof_channel
        add a, e
        ld e, a
        adc a, d
        sub a, e
        ld d, a

        djnz ostinato_play_orderdata_bank_loop

	; initialise channels
    ld hl, #ostinato_channels
    ld de, #_sizeof_channel

    ld a, (ostinato_channel_count)
    ld b, a
	
    xor a, a

    ostinato_play_init_loop: 

        ; clear flags byte
        ld (hl), a
        add hl, de

        djnz ostinato_play_init_loop

    ; change to first order
    xor a, a
    call ostinato_change_order

    ret 
