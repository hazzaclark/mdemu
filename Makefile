SRC_DIR             = src
INC_DIR             = inc
SOUND_DIR           = $(SRC_DIR)/sound
VIDEO_DIR           = $(SRC_DIR)/video

LIB68K_DIR          = lib68k/src
LIB68K_FILES        = $(LIB68K_DIR)/68K.c $(LIB68K_DIR)/68KOPCODE.c
MDFILES             = $(SRC_DIR)/md.c $(SOUND_DIR)/psg.c $(VIDEO_DIR)/vdp.c $(SOUND_DIR)/ym2612.c $(SRC_DIR)/cartridge.c

CFILES              = $(LIB68K_FILES) $(MDFILES) $(SRC_DIR)/main.c
OFILES              = $(CFILES:.c=.o)

CFLAGS              = -std=c99 -Wall -Wextra -Wno-int-conversion -Wno-incompatible-pointer-types \
                      -I$(INC_DIR) -I$(INC_DIR)/cpu -I$(INC_DIR)/sound -I$(INC_DIR)/video
LDFLAGS             = -lSDL2 -l68k

all: mdemu

mdemu: $(OFILES)
	$(CC) $(OFILES) -o mdemu $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OFILES) mdemu
