
.ifndef OSTINATO_DEFS
.define OSTINATO_DEFS 1 
 
.define GAME_GEAR_PORT_0    0x0

.define SN76489_PORT		0x7f
.define OPLL_REG_PORT		0xf0
.define OPLL_DATA_PORT		0xf1

.define SMS_MEMORY_CONTROL_PORT		0x3e
.define SMS_AUDIO_CONTROL_PORT		0xf2

.define SMS_MAPPER_SLOT_2 0xffff

.define SONG_FLAGS_BIT_SN7 7
.define SONG_FLAGS_BIT_YM 6
.define SONG_FLAGS_BIT_YM_RHYTHM 5
.define SONG_FLAGS_BIT_LOOPING 1
.define SONG_FLAGS_BIT_PLAYING 0

.define CHANNEL_FLAGS_BIT_MUTED 0

.define SYS_FLAG_HAS_SN7    0x01
.define SYS_FLAG_HAS_YM     0x02
.define SYS_FLAG_MARK_III   0x40
.define SYS_FLAG_GG         0x80

.STRUCT channel
    flags: db
    volume: db
    tone: dw
.ENDST

.STRUCT orderdata
	wait: dw
	bank: db
	ptr: dw
.ENDST

.endif