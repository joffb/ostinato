
; a : channel number
;
; returns channel's flags in a
_ostinato_get_channel_flags: 

    push hl
	
    ld hl, #ostinato_channels

	; a = a * channel size
	add a, a
	add a, a
	
	; move to channel
	add a, l
	ld l, a
	add a, h
	sub a, l
	ld h, a
	
    ; get flags into a
    ld a, (hl)

    pop hl

    ret 
