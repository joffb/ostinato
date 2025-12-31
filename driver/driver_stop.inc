
ostinato_stop:

    ; song playing?
    ld a, (ostinato_flags)
    bit SONG_FLAGS_BIT_PLAYING, a
    ret z

        ld hl, ostinato_channels

        ; preserve ostinato_flags
        ld d, a

        ; sn7 active?
        bit SONG_FLAGS_BIT_SN7, d
        jr z, ostinato_stop_no_sn7

            ; mute sn7 channels
            ld a, 0x9f
            out (SN76489_PORT), a
            ld a, 0xbf
            out (SN76489_PORT), a
            ld a, 0xdf
            out (SN76489_PORT), a
            ld a, 0xff
            out (SN76489_PORT), a

            ld hl, ostinato_channels + (_sizeof_channel * 4)

        ostinato_stop_no_sn7:
        bit SONG_FLAGS_BIT_YM, d
        jr z, ostinato_stop_no_ym

            ld b, 9
            ld c, 0x30

            ostinato_stop_ym_mute_loop:

                ; volume register
                ld a, c
                out (OPLL_REG_PORT), a

                ; get volume value for channel and set volume to quietest
                inc hl
                ld a, (hl)
                or a, 0x0f
                out (OPLL_DATA_PORT), a

                ; move on to upper tone value for channel
                inc hl
                inc hl
                
                ; tone upper/note on-off reg
                ld a, c
                sub a, 0x10
                out (OPLL_REG_PORT), a

                ; get tone/note on-off and clear note-on bit
                ld a, (hl)
                and a, 0xef
                out (OPLL_REG_PORT), a

                ; move to next channel
                inc hl

                ; next channel's registers
                inc c

                push hl
                pop hl
                push hl
                pop hl
                push hl
                pop hl

                djnz ostinato_stop_ym_mute_loop

        ostinato_stop_no_ym:

        ; clear song playing flag
        ld a, d
        res SONG_FLAGS_BIT_PLAYING, a
        ld (ostinato_flags), a

        ret