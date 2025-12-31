
_ostinato_looping_on: 

    ld a, (ostinato_flags)
    set SONG_FLAGS_BIT_LOOPING, a
    ld (ostinato_flags), a

    ret 

_ostinato_looping_off: 

    ld a, (ostinato_flags)
    res SONG_FLAGS_BIT_LOOPING, a
    ld (ostinato_flags), a

    ret 
