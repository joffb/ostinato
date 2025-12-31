
.RAMSECTION "OSTINATO_RAM" bank 0 slot 3

    ostinato_memory_control_value: db
    ostinato_system_flags: db

	ostinato_flags: db
    ostinato_channel_count: db
    ostinato_order_count: db 

    ostinato_orders: dw

    ostinato_order: db
    ostinato_order_timer: dw

    ostinato_channels: INSTANCEOF channel 15
	ostinato_orderdata: INSTANCEOF orderdata 15
	
	ostinato_ym_patch_data: ds 8

.ENDS

.include "driver_check_hardware.inc"
.include "driver_init.inc"
.include "driver_play.inc"
.include "driver_looping.inc"
.include "driver_stop.inc"
.include "driver_update.inc"
.include "driver_change_order.inc"
.include "driver_mute.inc"
.include "driver_unmute.inc"
.include "driver_get_channel_flags.inc"

.include "driver_sn7.inc"
.include "driver_ym.inc"
.include "driver_ym_patch.inc"
.include "driver_ym_rhythm.inc"
