# COPYRIGHT (C) HARRY CLARK 2024
# SEGA MEGA DRIVE EMULATOR

LIB68K_DIR          =       lib68k/src
LIB68K_FILES        =       $(LIB68K_DIR)/68K.c $(LIB68K_DIR)/68KOPCODE.c
MDFILES             =       md.c psg.c vdp.c ym2612.c cartridge.c

CFILES              =       $(LIB68K_FILES) $(MDFILES) main.c
OFILES              =       $(CFILES:.c=.o)

TARGET              =       mdemu
EXEPATH             =       ./

CC                  =       gcc
CFLAGS              =       -std=c99 -Wall -Wextra -Wno-int-conversion -Wno-incompatible-pointer-types
LDFLAGS             =       -lSDL2 -l68k

all: $(TARGET)

$(EXEPATH)$(TARGET): $(OFILES)
	$(CC) $(OFILES) -o $(EXEPATH)$(TARGET) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OFILES) $(EXEPATH)$(TARGET)

.PHONY: all clean
