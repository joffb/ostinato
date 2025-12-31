

### Exporting VGMs from Furnace

VGMs must be exported from Furnace with the "add pattern change hints" option selected. VGM files without pattern change hints are not supported.

### Compression

Currently the major compression is that
* patterns with duplicate data will only use one copy of the data
* multiple VGM writes in a frame are condensed down to use only the latest write (with special cases for YM2413 note-on/offs)

### Banking

Each channel's song data needs to fit into a single bank. This lets the song save space by only storing the bank number for a channel's pattern data once, rather than for every pattern order which can add up.

In WLA-DX mode the channel's song data is grouped like `.SECTION "song_ch_2_section" SUPERFREE SLOT 2`. This lets the linker place the channel data wherever it can.

In SDAS mode the script starts by assuming it has an empty bank. It begins with a start bank number (default = 2) and the script will use a type of first-fit bin packing to put each channel's song data into a bank. If the data can't fit into an existing bank, it moves on to a new empty bank.

## Song Data
Patterns have a list of "events" which start with an info byte. The info byte has a list of flags to indicate which parameters will be changed that frame, along with a wait duration to say how many frames to wait until the next event.

### SN7 and YM2413 frame event byte
```
vlhwxddd

v: volume change
l: tone low change
h: tone high change
w: long wait
x: reserved for looping
d: wait duration
```
### YM2413 rhythm event byte
```
rauwxddd

r: rhythm change
a: extra rhythm change
u: unused
w: long wait
x: reserved for looping
d: wait duration
```
### YM2413 patch change event byte
```
fpuwxddd

f: full patch change
p: partial patch change
u: unused
w: long wait
x: reserved for looping
d: wait duration
```
### Long/Short Waits:

When the long wait bit *is not* set, the frame wait duration will just be the 3-bit `ddd` duration i.e. 0-7 frames.
When the long wait bit *is* set, an additional byte will be read from the pattern and combined with the 3-bit duration for an 11-bit duration (like `dddDDDDDDDD`) for a wait time of up to 0x3ff or 1024 frames.

## Channel numbering

SN7 only
* SN7 channels 0-3

YM only
* YM channels 0-8
* YM rhythm channel 9
* YM patch change channel 10

SN7 + YM
* SN7 channels 0-3
* YM channels 4-12
* YM rhythm channel 13
* YM patch change channel 14