# COPYRIGHT (C) HARRY CLARK 2024
# SEGA MEGA DRIVE EMULATOR

SRC_DIR             =       src
SOUND_DIR           =       src/sound
VIDEO_DIR           =       src/video
INC_DIR             =       inc
CPU_INC_DIR         =       $(INC_DIR)/cpu
SOUND_INC_DIR       =       $(INC_DIR)/sound
VIDEO_INC_DIR       =       $(INC_DIR)/video

LIB68K_DIR          =       lib68k/src
LIB68K_FILES        =       $(LIB68K_DIR)/68K.c $(LIB68K_DIR)/68KOPCODE.c
MDFILES             =       $(SRC_DIR)/md.c $(SOUND_DIR)/psg.c $(VIDEO_DIR)/vdp.c $(SOUND_DIR)/ym2612.c $(SRC_DIR)/cartridge.c

CFILES              =       $(LIB68K_FILES) $(MDFILES) $(SRC_DIR)/main.c
OFILES              =       $(CFILES:.c=.o)

TARGET              =       mdemu
EXEPATH             =       ./

CC                  =       gcc
CFLAGS              =       -std=c99 -Wall -Wextra -Wno-int-conversion -Wno-incompatible-pointer-types \
                           -I$(INC_DIR) -I$(CPU_INC_DIR) -I$(SOUND_INC_DIR) -I$(VIDEO_INC_DIR)
LDFLAGS             =       -lSDL2 -l68k

all: $(TARGET)

$(EXEPATH)$(TARGET): $(OFILES)
	$(CC) $(OFILES) -o $(EXEPATH)$(TARGET) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OFILES) $(EXEPATH)$(TARGET)

.PHONY: all clean
