
; a: which chips to enable, combination of SYS_FLAG_HAS_x values
ostinato_init:

    ; using chips to use parameter
    ; enable/disable the requested chips with the audio control port
    ; sn = 0x1, opll = 0x2
    and a, SYS_FLAG_HAS_SN7 | SYS_FLAG_HAS_YM

    ; enable both opll and sn
    cp a, SYS_FLAG_HAS_SN7 | SYS_FLAG_HAS_YM
    jr z, bisc_sn_opll

    cp a, SYS_FLAG_HAS_SN7
    jr z, bisc_sn_only

    cp a, SYS_FLAG_HAS_YM
    jr z, bisc_opll_only

    ; neither opll or sn selected, try to enable both
    bisc_sn_opll:
    ld a, 0x3
    jr bisc_write

    bisc_opll_only:
    ld a, 0x1
    jr bisc_write

    bisc_sn_only:
    ld a, 0x00

    bisc_write:
    out (SMS_AUDIO_CONTROL_PORT), a

    ret