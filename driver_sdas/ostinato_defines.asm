GAME_GEAR_PORT_0    .equ	0x0
SN76489_PORT    .equ		0x7f
OPLL_REG_PORT    .equ		0xf0
OPLL_DATA_PORT    .equ		0xf1

SMS_MEMORY_CONTROL_PORT    .equ		0x3e
SMS_AUDIO_CONTROL_PORT    .equ		0xf2

SMS_MAPPER_SLOT_2    .equ 0xffff

SONG_FLAGS_BIT_SN7    .equ 7
SONG_FLAGS_BIT_YM    .equ 6
SONG_FLAGS_BIT_YM_RHYTHM    .equ 5
SONG_FLAGS_BIT_LOOPING    .equ 1
SONG_FLAGS_BIT_PLAYING    .equ 0

CHANNEL_FLAGS_BIT_MUTED    .equ 0

SYS_FLAG_HAS_SN7    .equ 0x01
SYS_FLAG_HAS_YM     .equ 0x02
SYS_FLAG_MARK_III   .equ 0x40
SYS_FLAG_GG         .equ 0x80

channel.flags    .equ   0
channel.volume  .equ    1
channel.tone    .equ    2
_sizeof_channel .equ    4

orderdata.wait  .equ    0
orderdata.bank  .equ    2
orderdata.ptr   .equ    3
_sizeof_orderdata .equ  5
