#!/usr/bin/env python3

import struct
import math
import sys
import hashlib
import pprint

from pathlib import Path
from optparse import OptionParser

TYPE_SN7 = 0
TYPE_YM = 1
TYPE_YM_EX = 2

INFO_BIT_VOLUME = 0x80
INFO_BIT_TONE_LOW = 0x40
INFO_BIT_TONE_HIGH = 0x20
INFO_BIT_LONG_WAIT = 0x10

INFO_BIT_RHTYHM = 0x80
INFO_BIT_RHTYHM2 = 0x40

INFO_BIT_PATCH_DATA = 0x80
INFO_BIT_PART_PATCH_DATA = 0x40

YM2413_NOTE_ON_BIT = 0x10

parser = OptionParser("Usage: vgmparse.py [options] INPUT_FILE_NAME.VGM")

parser.add_option("-o", '--out',        dest='outfilename',     help='output file name')
parser.add_option("-i", '--identifier', dest='identifier',      help='identifier name in source code')
parser.add_option("-f", "--framerate",  dest='framerate',       default="60",  help='song framerate in hz')

parser.add_option("-b", '--bank',       dest='bank',            default="2",   help='starting BANK number in SDAS mode')
parser.add_option("-a", '--area',       dest='area',            default="BANK", help='AREA name in SDAS mode')

parser.add_option("-s", '--sdas',       dest='sdas',              action="store_true",    help='SDAS assembler compatible output')

(options, args) = parser.parse_args()

# check for an input filename
if (len(args) == 0):
    parser.print_help()
    parser.error("Input file name required\n")

infilename = Path(args[0])

# try to read file
try:
    f = open(infilename, "rb")
    data = f.read()
    f.close()
except OSError:
    print("Error reading input file: " + str(infilename), file=sys.stderr)
    sys.exit(1)

# break up header into chunks
header = struct.unpack("<" + ("I" * 16), data[0:64])

# check vgm id string
if header[0] != 0x206d6756:
    print("Error: file is not a valid VGM file: " + str(infilename), file=sys.stderr)
    sys.exit(1)

# requires VGM version >= 1.50
if header[2] < 0x150:
    print("Error: file must have VGM version >= 1.50", file=sys.stderr)
    sys.exit(1)

# pointer to start of data
ptr = header[13] + 0x34
print("data_start: " + hex(ptr))

# framerate
if str(options.framerate) == 60:
    samples_per_frame = 735
else:
    samples_per_frame = 882

# song identifier
if options.identifier:
    song_prefix = options.identifier
else:
    song_prefix = infilename.stem

# output filename
if options.outfilename:
    outfilename = options.outfilename
else:
    outfilename = infilename.stem + ".asm"

# output type
if options.sdas:
    asm_type = "sdas"
else:
    asm_type = "wladx"

print(options)

sdas_area = options.area

sdas_bank = int(options.bank)
sdas_start_bank = sdas_bank
sdas_bank_size = 16384

sn7_found = False
ym2413_found = False
ym2413_rhythm_found = False

furnace_channel_count = 0
channel_count = 0
ym2413_ch0 = 0

# sn7 used
if header[3] != 0:
    furnace_channel_count += 4
    channel_count += 4
    ym2413_ch0 = 4
    
    sn7_found = True
    
# ym2413 used
if header[4] != 0:
    furnace_channel_count += 9
    channel_count += 11
    
    ym2413_found = True

channels = []

for i in range (0, channel_count):

    if channel_count == 11 or (channel_count == 15 and i >= ym2413_ch0):

        if i == channel_count - 1:
            channel_type = TYPE_YM_EX
        else:
            channel_type = TYPE_YM
    else:
        channel_type = TYPE_SN7

    channels.append({"number": i, "type": channel_type, "patterns": [], "orders": []})

new_frame = False
pattern_count = 0

# sn76489 latches
sn_channel = 0
sn_mode = 0

def set_wait(wait):
    
    #print("frame wait * " + str(wait))
    
    # set wait amount in all channels
    for i in range (0, channel_count):
        pattern = channels[i]["patterns"][pattern_count - 1]
        pattern["data"][len(pattern["data"])-1]["wait"] = wait
    

while ptr < len(data):
    
    if new_frame:
        
        for i in range(0, channel_count):
            pattern = channels[i]["patterns"][pattern_count - 1]
            pattern["data"].append({"wait": 0})
            
        new_frame = False
    
    # data block
    if data[ptr] == 0x67 and data[ptr + 1] == 0x66:

        print("block start: " + hex(ptr))
        
        ptr += 2
        
        block_type = data[ptr]
        ptr += 1
        
        block_length = struct.unpack("<I", data[ptr:ptr+4])[0]
        ptr += 4
        
        # furnace pattern change hint
        # 0x67 0x66 0xfe LLLLLLLL 01 OO RR (PP PP ...)
        if block_type == 0xfe:
            
            # get order info
            pad, order, row = struct.unpack("<BBB", data[ptr:ptr+3])
            ptr += 3
            
            # get channel pattern numbers
            pattern_numbers = struct.unpack("B" * channel_count, data[ptr:ptr+channel_count])
            ptr += block_length - 3
            
            # create a new pattern and order record
            for i in range(0, channel_count):
                channels[i]["orders"].append({"number": len(channels[i]["patterns"]), "order": order, "row": row, "match": False, "pattern_index": pattern_numbers[i]})
                channels[i]["patterns"].append({"number": pattern_numbers[i], "converted_size": 0, "converted": [], "data": []})

            new_frame = True
            pattern_count += 1

            print("patterns: " + str(pattern_numbers))
     
        # other block types ignored
        else:
            
            ptr += block_length 

        print("block end: " + hex(ptr))
     
    # word length wait
    elif data[ptr] == 0x61:
        
        # divide up into 60hz frames
        wait = struct.unpack("<H", data[ptr+1:ptr+3])
        wait = math.floor(wait[0] / samples_per_frame)
        
        set_wait(wait)
        new_frame = True
        
        ptr += 3
        
    # one frame wait
    elif data[ptr] == 0x62 or data[ptr] == 0x63:
        
        set_wait(1)
        new_frame = True
        
        ptr += 1
            
    # sn76489 write
    elif data[ptr] == 0x50:
            
        val = data[ptr + 1]
        
        # latch
        if val & 0x80:
            sn_channel = (val >> 5) & 0x3
            sn_type = (val >> 4) & 0x1
            
            pattern = channels[sn_channel]["patterns"][pattern_count - 1]
            event = pattern["data"][len(pattern["data"])-1]
            
            # volume write
            if sn_type == 1:
                #print("sn volume write ch" + str(sn_channel) + " " + hex(val))
                event["volume"] = val
                
            # latched tone write
            else:
                #print("sn tone low write ch" + str(sn_channel) + " " + hex(val))
                event["tone_low"] = val

        # data write
        else:
            
            pattern = channels[sn_channel]["patterns"][pattern_count - 1]
            event = pattern["data"][len(pattern["data"])-1]
            
            # volume write
            if sn_type == 1:
                #print("sn volume write ch" + str(sn_channel) + " " + hex(val))
                event["volume"] = val
            
            # tone write
            else:
                #print("sn tone high write ch" + str(sn_channel) + " " + hex(val))
                event["tone_high"] = val

        ptr += 2
    
    # gg stereo
    elif data[ptr] == 0x4f:

        ptr += 2

    # ym2413 write
    elif data[ptr] == 0x51:
        
        reg = data[ptr + 1]
        val = data[ptr + 2]
        
        # volume change
        if reg >= 0x30:
            
            channel = ym2413_ch0 + reg - 0x30
            
            pattern = channels[channel]["patterns"][pattern_count - 1]
            event = pattern["data"][len(pattern["data"])-1]
            
            event["volume"] = val
            
        # tone high/note on 
        elif reg >= 0x20:
            
            channel = ym2413_ch0 + reg - 0x20
            
            pattern = channels[channel]["patterns"][pattern_count - 1]
            event = pattern["data"][len(pattern["data"])-1]
            
            # we want to use the msb of the tone_high byte to flag
            # that we're going from note-off to note-on
            if "tone_high" in event and ((event["tone_high"] & YM2413_NOTE_ON_BIT) == 0) and (val & YM2413_NOTE_ON_BIT):
                event["tone_high"] = (val & ~YM2413_NOTE_ON_BIT) | 0x80
            else:
                # msb should be clear on the first write
                event["tone_high"] = val & 0x7f

            #print("TONE_HIGH " + str(channel) + " : " + hex(val) + " -> " + hex(event["tone_high"]))
        
        # tone low
        elif reg >= 0x10:
            
            channel = ym2413_ch0 + reg - 0x10
            
            pattern = channels[channel]["patterns"][pattern_count - 1]
            event = pattern["data"][len(pattern["data"])-1]
            
            event["tone_low"] = val
            
        # rhythm mode
        elif reg == 0x0e:
            
            channel = ym2413_ch0 + 9
            
            ym2413_rhythm_found = True

            pattern = channels[channel]["patterns"][pattern_count - 1]
            event = pattern["data"][len(pattern["data"])-1]

            if "rhythm" not in event:
                event["rhythm"] = [val & 0x3f]
            else:
                event["rhythm"].append(val & 0x3f)
        
        # user patch data
        elif reg < 0x8:
            
            channel = ym2413_ch0 + 10

            pattern = channels[channel]["patterns"][pattern_count - 1]
            event = pattern["data"][len(pattern["data"])-1]
            
            if "patch" not in event:
                event["patch"] = {}
                
            event["patch"][reg] = val
            
        ptr += 3
    
    # end of data
    elif data[ptr] == 0x66:
        
        break
        
    else:
        
        print("skipped: " + hex(data[ptr]))
        
        ptr += 1
        

for channel in channels:
    for pattern in channel["patterns"]:
        
        vol = False
        tone_low = False
        tone_high = False
        
        # remove volume events where the volume hasn't changed
        for i in range(0, len(pattern["data"])):
            
            event = pattern["data"][i]
            
            if (channel["type"] == TYPE_SN7) or (channel["type"] == TYPE_YM):

                if "volume" in event:
                    
                    # first volume write this pattern
                    if vol is False:
                        vol = event["volume"]
                        
                    # volume is the same
                    elif vol == event["volume"]:
                        del event["volume"]
                        
                    # volume is new
                    else:
                        vol = event["volume"]

        # simplify rhythm events
        for i in range(0, len(pattern["data"])):
            
            event = pattern["data"][i]
            
            if "rhythm" in event:

                # only one rhythm change this frame
                if len(event["rhythm"]) == 1:
                    
                    event["rhythm"] = event["rhythm"][0]

                # multiple rhythm changes
                else:

                    last_rhythm = event["rhythm"][len(event["rhythm"]) - 1]
                    
                    # when going from note-on to note-off, furnace seems to
                    # create a lot of note-off and note-on events 
                    # (possibly due to rhythm being split over many tracks)
                    # notes need to note-off befor note-on to trigger them properly so
                    # try to find the rhythm write with the most note-on/off bits which are 0
                    # and output that byte first, then the latest rhythm write
                    min_rhythm = last_rhythm
                    min_offs = 0

                    for j in range(0, len(event["rhythm"]) - 1):

                        rhythm = event["rhythm"][j]

                        offs = 0

                        if (rhythm & 0x01) == 0: offs += 1
                        if (rhythm & 0x02) == 0: offs += 1
                        if (rhythm & 0x04) == 0: offs += 1
                        if (rhythm & 0x08) == 0: offs += 1
                        if (rhythm & 0x10) == 0: offs += 1

                        if offs > min_offs:
                            min_rhythm = rhythm
                            min_offs = offs

                    event["rhythm"] = min_rhythm | 0x80
                    event["rhythm2"] = last_rhythm


        # remove tone events where the tone hasn't changed
        for i in range(0, len(pattern["data"])):
            
            event = pattern["data"][i]
            
            if channel["type"] == TYPE_SN7:

                if "tone_low" in event and "tone_high" in event: 
                    
                    new_tone_low = event["tone_low"]
                    new_tone_high = event["tone_high"]
                    
                    # first tone write this pattern
                    if tone_low is False or tone_high is False:
                        tone_low = new_tone_low
                        tone_high = new_tone_high
                        
                    # tone is the same
                    elif tone_low == new_tone_low and tone_high == new_tone_high:
                        del event["tone_low"]
                        del event["tone_high"]
                        
                    # only low tone is different
                    elif tone_low != new_tone_low and tone_high == new_tone_high:
                        tone_low = new_tone_low
                        del event["tone_high"]
                        
                    # tone is new
                    else:
                        tone_low = new_tone_low
                        tone_high = new_tone_high

            elif channel["type"] == TYPE_YM:

                if "tone_low" in event:

                    new_tone_low = event["tone_low"]

                    # first tone low write this pattern
                    if tone_low is False:
                        tone_low = new_tone_low
                        
                    # tone is the same
                    elif tone_low == new_tone_low:
                        del event["tone_low"]
                        
                    # tone is new
                    else:
                        tone_low = new_tone_low
        
        # condense frame waits down to a single value
        for i in reversed(range(1, len(pattern["data"]))):
            
            # if there's only a wait on this frame
            # condense it with the previous frame's wait
            # remove frame event object
            if (len(pattern["data"][i].keys()) == 1):
                pattern["data"][i - 1]["wait"] += pattern["data"][i]["wait"]
                pattern["data"].pop(i)

# convert events to bytes
for channel in channels:
    for pattern in channel["patterns"]:
        for i in range(0, len(pattern["data"])):
            
            event = pattern["data"][i]
            
            # info byte says what work is to be done this frame
            info_byte = 0
            out = []
            
            if "volume" in event:
                info_byte |= INFO_BIT_VOLUME
                out.append(event["volume"])
            
            if "tone_low" in event:
                info_byte |= INFO_BIT_TONE_LOW
                out.append(event["tone_low"])
                
            if "tone_high" in event:
                info_byte |= INFO_BIT_TONE_HIGH
                out.append(event["tone_high"])
            
            # ym2413 rhythm data on special channel
            if "rhythm" in event:
                info_byte |= INFO_BIT_RHTYHM
                out.append(event["rhythm"])

                if "rhythm2" in event:
                    info_byte |= INFO_BIT_RHTYHM2
                    out.append(event["rhythm2"])
            
            # ym2413 patch data on special channel
            if "patch" in event:
                
                # full patch change
                if len(event["patch"]) == 8:
                    
                    info_byte |= INFO_BIT_PATCH_DATA
                
                    # output patch data
                    for b in event["patch"]:
                        out.append(event["patch"][b])
                 
                # partial patch change
                else:
                    
                    info_byte |= INFO_BIT_PART_PATCH_DATA
                    
                    # get byte flagging which registers will be updated
                    reg_byte = 0
                    
                    for b in event["patch"]:
                        reg_byte = reg_byte | (1 << b)
                        
                    out.append(reg_byte)
                    
                    # output patch data
                    for b in event["patch"]:
                        out.append(event["patch"][b])
            
            # long wait
            if event["wait"] > 8:
                info_byte |= INFO_BIT_LONG_WAIT
                info_byte |= (event["wait"] >> 8)
                out.append(event["wait"] & 0xff)

                if event["wait"] > 0x3ff:
                    print("WARNING: TOO LONG WAIT FOUND!")
                
            # short wait
            # special case where 0 means "wait 8 frames"
            else:
                info_byte |= (event["wait"] & 0x7)
            
            # put info byte at start
            out.insert(0, info_byte)
            
            pattern["converted"].append(out)
            pattern["converted_size"] += len(out)

# match patterns for duplicates
for channel in channels:

    for i in range(1, len(channel["orders"])):
        
        order = channel["orders"][i]
        conv_i = channel["patterns"][order["number"]]["converted"]
        
        for j in range(0, i):
            
            neworder = channel["orders"][j]
            conv_j = channel["patterns"][neworder["number"]]["converted"]
            
            # different lengths, we're done
            if len(conv_i) != len(conv_j):
                continue
                
            matches = 0
            
            # compare contents
            for x in range (0, len(conv_i)):
                if conv_i[x] == conv_j[x]:
                    matches += 1
            
            # match with previous pattern found
            if matches == len(conv_i):
                order["match"] = neworder["number"]
                #print("match found")
                break

# calculate size of each channel
for channel in channels:

    channel["converted_size"] = 0

    # go through orders and add up sizes 
    # of orders which haven't been deduplicated
    for order in channel["orders"]:
        if order["match"] is False:
            channel["converted_size"] += channel["patterns"][order["number"]]["converted_size"]

output_data_size = 0
deduplications = 0

# put together song flags byte
# set playing and looping flags by default
song_flags = 1 | 2
if sn7_found: song_flags |= 0x80
if ym2413_found: song_flags |= 0x40
if ym2413_rhythm_found: song_flags |= 0x20

outfile = open(song_prefix + ".asm", "w")

if (asm_type == "wladx"):
    
    outfile.write(".SECTION \"" + song_prefix + "_song_section\" SUPERFREE SLOT 2" + "\n")
    outfile.write(song_prefix + "_data:" + "\n")

elif (asm_type == "sdas"):

    headerfile = open(song_prefix + ".h", "w")
    headerfile.write("#define __bank_" + song_prefix + "_data " + str(sdas_start_bank) + "\n")
    headerfile.write("extern unsigned char " + song_prefix + "_data[];" + "\n")
    headerfile.close()
    
    outfile.write(".module _" + song_prefix + "_data" + "\n")
    outfile.write(".globl _" + song_prefix + "_data" + "\n")

    outfile.write("\n")

    outfile.write("___bank_" + song_prefix + "_data" + " .equ " + str(sdas_start_bank) + "\n")
    outfile.write(".globl ___bank_" + song_prefix + "_data" + "\n")

    outfile.write("\n")

    outfile.write(".AREA _" + sdas_area + str(sdas_start_bank) + " (REL, CON)\n")
    outfile.write("_" + song_prefix + "_data:" + "\n")

    outfile.write("\n")

# song data
order_count = len(channels[0]["orders"])
outfile.write("\t" + song_prefix + "_flags: .db " + str(hex(song_flags)) + "\n")
outfile.write("\t" + song_prefix + "_channel_count: .db " + str(len(channels)) + "\n")
outfile.write("\t" + song_prefix + "_order_count: .db " + str(order_count) + "\n")
outfile.write("\t" + song_prefix + "_orders: .dw " + song_prefix + "_order_table" + "\n")

output_data_size += 5

# calculate size of order table
output_data_size += (order_count * ((len(channels) * 2) + 2))

# add size of channel_banks list
output_data_size += len(channels)

bank_allocations = []

# calculate sdas banks for channel pattern data and write them out
if asm_type == "sdas":

    bank_usage = [output_data_size]

    for channel in channels:

        for i, usage in enumerate(bank_usage):

            # channel data will overflow bank i, try next one
            if bank_usage[i] + channel["converted_size"] > sdas_bank_size:

                continue

            # it fits in this bank
            else:

                # doesn't overflow bank, just add this pattern's size to the current_size
                bank_usage[i] += channel["converted_size"]
                channel["sdas_bank"] = i + sdas_start_bank

                break
            
        # not enough space in the created banks so far, so add a new one to the list
        if "sdas_bank" not in channel:
            bank_usage.append(channel["converted_size"])
            channel["sdas_bank"] = len(bank_usage) - 1 + sdas_start_bank

        bank_allocations.append(str(channel["sdas_bank"]))

        print(bank_usage)

# just use the `:` bank number operator in wla-dx mode
elif asm_type == "wladx":

    for channel in channels:
        bank_allocations.append(":" + song_prefix + "_ch_" + str(channel["number"]) + "_patterns")

# write out bank numbers
outfile.write("\t" + song_prefix + "_channel_banks: .db " + ",".join(bank_allocations) + "\n")

# order data
outfile.write("\n")
outfile.write(song_prefix + "_order_table:" + "\n")

for i in range(0, order_count):

    length = 0

    # get order length
    for event in channels[0]["patterns"][i]["data"]:
        length += event["wait"]

    outfile.write("\t.dw " + str(length) + "\n")

    # get order pattern pointers
    for channel in channels:

        order = channel["orders"][i]

        # use original pattern data
        if order["match"] is False:
            pattern = channel["patterns"][order["number"]]
            pattern_label = song_prefix + "_ch_" + str(channel["number"]) + "_pattern_" + str(order["number"])
            
        # use matched pattern data
        else:
            pattern = channel["patterns"][order["match"]]
            pattern_label = song_prefix + "_ch_" + str(channel["number"]) + "_pattern_" + str(order["match"])

            deduplications += 1

        outfile.write("\t.dw " + pattern_label + "\n")
        
    outfile.write("\n")

if (asm_type == "wladx"):

    outfile.write(".ENDS\n")

# pattern data
outfile.write("\n")
outfile.write(song_prefix + "patterns:" + "\n")

for channel in channels:
    
    # in wla-dx mode, create a new free section for each channel's data
    if (asm_type == "wladx"):
        
        outfile.write(".SECTION \"" + song_prefix + "_ch_" + str(channel["number"]) + "_section\" SUPERFREE SLOT 2" + "\n")
        outfile.write(song_prefix + "_ch_" + str(channel["number"]) + "_patterns:" + "\n")
    
    # in sdas mode, set the area/bank for this channel's data
    elif (asm_type == "sdas"):

        outfile.write(".AREA _" + sdas_area + str(channel["sdas_bank"]) + " (REL,CON)\n")

    for order in channel["orders"]:
        
        # this order uses a unique pattern which hasn't been matched to a previous one
        if order["match"] is False:

            pattern = channel["patterns"][order["number"]]                    

            outfile.write(song_prefix + "_ch_" + str(channel["number"]) + "_pattern_" + str(order["number"]) + ":" + "\n")
            
            # write out the pattern data
            for conv in pattern["converted"]:
                outfile.write("\t" + ".db " + ",".join(map(hex, conv)) + "\n")
                
                output_data_size += len(conv)
                
    if (asm_type == "wladx"):
        
        outfile.write(".ENDS\n")

outfile.write("\n")

outfile.close()

print("data size: " + str(output_data_size) + " bytes")
print("patterns deduplicated: " + str(deduplications))
