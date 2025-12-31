#include "SMSlib.h"

#include "../../lib/ostinato.h"

#include "test.h"
#include "test_ym.h"

unsigned char tic;
unsigned char song_bank;

void main(void)
{
	unsigned int keys;

	SMS_VRAMmemsetW(0, 0x0000, 16384);
	SMS_setBGPaletteColor(0, RGBHTML(0x0000FF));

	/* Turn on the display */
	SMS_displayOn();

    ostinato_check_hardware();

    if (ostinato_system_flags & SYS_FLAG_HAS_YM)
    {
        ostinato_init(SYS_FLAG_HAS_YM);

        song_bank = __bank_test_ym_data;
        SMS_mapROMBank(song_bank);
	    ostinato_play(test_ym_data);
    }
    else
    {
        ostinato_init(SYS_FLAG_HAS_SN7);

        song_bank = __bank_test_data;
        SMS_mapROMBank(song_bank);
	    ostinato_play(test_data);   
    }

	ostinato_looping_off();

	tic = 0;

	for(;;)
	{

		SMS_waitForVBlank();

		keys = SMS_getKeysPressed();

		if (keys & 0x20)
		{
			ostinato_stop();
		}

        SMS_mapROMBank(song_bank);
		ostinato_update();

		tic++;
	}
}

SMS_EMBED_SEGA_ROM_HEADER_16KB(9999,0);
SMS_EMBED_SDSC_HEADER_AUTO_DATE_16KB(1,0,"joe k  ","ostinx test","");