
; a : channel number
_ostinato_mute_channel: 
	
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
	
	; mute channel
	set CHANNEL_FLAGS_BIT_MUTED, (hl)

    ; song flags in c
    ld a, (ostinato_flags)
    ld c, a

    ; channel number back into a
    pop af

    ; sn7 present?
    bit SONG_FLAGS_BIT_SN7, c
    jr z, smc_no_sn7

        ; sn7 channels are 0-3
        cp a, #4
        jr nc, smc_has_sn7_ym

            ; sn7 channel
            ; shift channel number into position
            rrca 
            rrca 
            rrca 

            ; mask out other bits
            and a, #0x60

            ; volume change, max volume attenuation
            or a, #0x9f

            ; write
            out (#SN76489_PORT), a

            ret 

        ; ym channels are 4-15
        smc_has_sn7_ym: 

            ; zero-index the channel number
            sub a, #4

            ; fall through to ym muting code

    smc_no_sn7: 

        ; rhythm note-on/off channel
        cp a, #9
        jr z, smc_rhythm_mute

        ; patch change channel
        cp a, #10
        jr z, smc_patch_mute

        ; regular ym channel

        ; select tone high register
        add a, #0x20
        out (#OPLL_REG_PORT), a

        ; move hl to upper tone byte
        inc hl
        inc hl
        inc hl

        ; get high byte of tone and turn it into a note-off
        ld a, (hl)
        and a, #0xef

        ; send note-off
        out (#OPLL_DATA_PORT), a

    smc_patch_mute: 

	    ret 


smc_rhythm_mute: 

    ; do nothing if this song isn't rhythm mode
    ld a, (ostinato_flags)
    bit SONG_FLAGS_BIT_YM_RHYTHM, a
    ret z

    ; set all note-offs
    ld a, #0xe
    out (#OPLL_REG_PORT), a
    ld a, #0x20
    out (#OPLL_DATA_PORT), a

    ret 
