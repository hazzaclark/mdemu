# COPYRIGHT (C) HARRY CLARK 2024
# SEGA MEGA DRIVE EMULATOR

68000FILES          =       68000.c opcode.c
MDFILES             =       md.c psg.c vdp.c ym2612.c cartridge.c
CFILES              =       $(68000FILES) $(MDFILES) main.c

OFILES              =       $(CFILES:.c=.o)

TARGET              =       mdemu
EXEPATH             =       ./

CC                  =       gcc
CFLAGS              =       -std=c99 -Wall -Wextra -Wno-int-conversion -Wno-incompatible-pointer-types
LDFLAGS             =       -lSDL2

all: $(TARGET)

$(EXEPATH)$(TARGET): $(OFILES)
	$(CC) $(OFILES) -o $(EXEPATH)$(TARGET) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OFILES) $(EXEPATH)$(TARGET)

.PHONY: all clean
