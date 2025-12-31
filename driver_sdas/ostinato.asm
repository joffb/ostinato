.include "ostinato_defines.asm"

.module OSTINATO

.globl _ostinato_check_hardware
.globl _ostinato_init

.globl _ostinato_play
.globl _ostinato_update
.globl _ostinato_stop

.globl _ostinato_looping_on
.globl _ostinato_looping_off

.globl _ostinato_order_jump

.globl _ostinato_get_channel_flags
.globl _ostinato_mute_channel
.globl _ostinato_unmute_channel

.globl _ostinato_system_flags

.area _DATA (REL,CON)

    ostinato_memory_control_value: .ds 1
    _ostinato_system_flags: .ds 1
	
    ostinato_flags: .ds 1
    ostinato_channel_count: .ds 1
    ostinato_order_count: .ds 1 

    ostinato_orders: .ds 2

    ostinato_order: .ds 1
    ostinato_order_timer: .ds 2

    ostinato_channels: .ds _sizeof_channel * 15
	ostinato_orderdata: .ds _sizeof_orderdata * 15
	
	ostinato_ym_patch_data: .ds 8

;.ifdef BANJO_GBDK
	;.area _CODE_1 (REL,CON)
;.else
	.area _CODE (REL,CON)
;endif 

.include "driver_init.inc"
.include "driver_check_hardware.inc"
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
