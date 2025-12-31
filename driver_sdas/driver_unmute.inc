
; a : channel number
_ostinato_unmute_channel: 
	
    ; don't do anything if a song isn't playing
    ld hl, #ostinato_flags
    bit SONG_FLAGS_BIT_PLAYING, (hl)
    ret z

    ; preserve channel number
    push af

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
	
	; unmute channel
	res CHANNEL_FLAGS_BIT_MUTED, (hl)

    ; song flags in c
    ld a, (ostinato_flags)
    ld c, a

    ; channel number back into a
    pop af

    ; sn7 present?
    bit SONG_FLAGS_BIT_SN7, c
    jr z, sumc_no_sn7

        ; sn7 channels are 0-3
        cp a, #4
        jr nc, sumc_has_sn7_ym

            ; sn7 channel
            
            ; preserve channel number in b
            ld b, a
            
            ; move to volume byte in channel
            inc hl

            ; restore volume
            ld a, (hl)
            out (#SN76489_PORT), a

            ; restore tone low
            inc hl
            ld a, (hl)
            out (#SN76489_PORT), a

            ; is this ch3 ? if so don't restore tone high
            ; as it will be undefined
            ld a, b
            cp a, #3
            jr z, sumc_sn7_ch3

                ; restore tone high
                inc hl
                ld a, (hl)
                out (#SN76489_PORT), a

            sumc_sn7_ch3: 

            ret 

        ; ym channels are 4-15
        sumc_has_sn7_ym: 

            ; zero-index the channel number
            sub a, #4

            ; fall through to ym muting code

    sumc_no_sn7: 

        ; rhythm note-on/off channel
        cp a, #9
        jr z, sumc_rhythm_unmute

        ; patch change channel
        cp a, #10
        jr z, sumc_patch_unmute

        ; regular ym channel

        ; move to volume byte in channel
        inc hl

        ; select volume register
        ; preserve value in c
        add a, #0x30
        ld c, a
        out (#OPLL_REG_PORT), a

        ; get volume and output it
        ld a, (hl)
        out (#OPLL_DATA_PORT), a

        ; wait
        push hl
        pop hl
        push hl
        pop hl
        push hl
        pop hl

        ; move hl and reg to tone low byte
        inc hl
        ld a, c
        sub a, #0x20
        out (#OPLL_REG_PORT), a

        ; get tone low and output it
        ld a, (hl)
        out (#OPLL_DATA_PORT), a

        ; wait
        push hl
        pop hl
        push hl
        pop hl
        push hl
        pop hl

        ; move hl and reg to tone high byte
        inc hl
        ld a, c
        sub a, #0x10
        out (#OPLL_REG_PORT), a

        ; get tone low and output it
        ld a, (hl)
        out (#OPLL_DATA_PORT), a

    sumc_rhythm_unmute: 

        ret 

    sumc_patch_unmute: 

        ld b, #8
        ld hl, #ostinato_ym_patch_data

        sumv_patch_unumte_loop: 

            ld a, #8
            sub a, b
            out (#OPLL_REG_PORT), a

            ld a, (hl)
            out (#OPLL_DATA_PORT), a

            inc hl

            push hl
            pop hl
            push hl
            pop hl
            push hl
            pop hl

            djnz sumv_patch_unumte_loop

	    ret 
