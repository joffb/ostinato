python ../../vgmparse.py --sdas -b 2 ../test_ym.vgm
sdasz80 -g -o test_ym.rel test_ym.asm

python ../../vgmparse.py --sdas -b 3 ../test.vgm
sdasz80 -g -o test.rel test.asm

sdcc -c -I../../music_driver_sdas -mz80 main.c
sdcc -c -I../../music_driver_sdas -mz80 --codeseg BANK1 bank1.c

sdcc -o dktest.ihx -mz80 --no-std-crt0 --data-loc 0xC000 \
    -Wl-b_BANK1=0x18000 -Wl-b_BANK2=0x28000 -Wl-b_BANK3=0x38000 -Wl-b_BANK4=0x48000 \
    crt0_sms.rel SMSlib.lib \
    ../../lib/ostinato.rel \
    main.rel bank1.rel test.rel test_ym.rel
makesms dktest.ihx dktest.sms