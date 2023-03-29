/* Copyright(C) 2023 Harry Clark * /

/* SEGA Mega Drive Emulator */

/* THIS FILE PERTAINS TO THE MAIN FUNCTIONALITY OF THE VIDEO DISPLAY PORT OF THE MEGA DRIVE */
/* TAKING INTO ACCOUNT THE INTRICACIES OF THE SYSTEM THROUGH VARIOUS PIECES OF DOCUMENTATION */

/* DOCUMENTATION INCLUDES: */

/* https://wiki.megadrive.org/index.php?title=VDP */

#ifndef VDP
#define VDP

/* NESTED INCLUDES */

#include "common.h"

/* SYSTEM INCLUDES */

#include <stdbool.h>
#include <stdio.h>

#ifdef DEBUG
#define DEBUG_LOG(...)(__VA_ARGS__)
#else
#define DEBUG_LOG()
#endif

/* THIS INFORMATION IS WITHIN ACCORDANCE TO THE BITWISE VALUE */
/* OF WHAT THE PAL MEGA DRIVE CAN OUTPUT */

#ifndef VDP_TYPES
#define VDP_TYPES

#define BUFFER_WIDTH 320
#define BUFFER_HEIGHT 240
#define BUFFER_SIZE (BUFFER_WIDTH * BUFFER_HEIGHT * 3)

#endif

/* USING THE RULE OF THE NTH VALUE + 4 */
/* THIS HELPS TO CREATE THE CORRESPONDING COLOURS CHANNELS */
/* TO CREATE AN 11 BIT COLOUR VALUE */

/* DOCUMENTATION PERTAINING TO THIS */
/* https://wiki.megadrive.org/index.php?title=VDP_Palette */

#ifndef COLOUR_CHANNELS
#define COLOUR_CHANNELS

#define RED_CHANNEL(COLOUR) FRAGMENT((COLOUR), 3, 1)
#define GREEN_CHANNEL(COLOUR) FRAGMENT((COLOUR), 7, 5)
#define BLUE_CHANNEL(COLOUR) FRAGMENT((COLOUR), 11, 9)

#define COLOUR_COMP(COLOUR) 
static COLOUR_COMP(COLOUR * 32);

#endif

#ifndef REG
#define REG

typedef U8* REG_1;
typedef U8* REG_2;
typedef U8* REG_3;
typedef U8* REG_4;

typedef struct VDP_REGISTERS
{
	typedef REG_1* DISPLAY_ENABLED(bool);
	typedef REG_1* INTERRUPT_ENABLED(bool);
	typedef REG_1* DMA(bool);
	typedef REG_1* DISPLAY_HEIGHT;
};

#endif

#endif