
#ifndef OSTINATO_H
#define OSTINATO_H

#define SYS_FLAG_HAS_SN7    0x01
#define SYS_FLAG_HAS_YM     0x02
#define SYS_FLAG_MARK_III   0x40
#define SYS_FLAG_GG         0x80

extern unsigned char ostinato_system_flags;

void ostinato_check_hardware(void);
void ostinato_init(unsigned char);

void ostinato_play(unsigned char *song_data);
void ostinato_update(void);
void ostinato_stop(void);

void ostinato_looping_on(void);
void ostinato_looping_off(void);

void ostinato_order_jump(unsigned char order_number);

unsigned char ostinato_get_channel_flags(unsigned char channel);
void ostinato_mute_channel(unsigned char channel);
void ostinato_unmute_channel(unsigned char channel);

#endif