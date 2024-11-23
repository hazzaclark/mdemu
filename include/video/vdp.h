/* COPYRIGHT (C) HARRY CLARK 2024 */

/* SEGA MEGA DRIVE EMULATOR */

/* THIS FILE PERTAINS TO THE MAIN FUNCTIONALITY OF THE VIDEO DISPLAY PORT OF THE MEGA DRIVE */
/* TAKING INTO ACCOUNT THE INTRICACIES OF THE SYSTEM THROUGH VARIOUS PIECES OF DOCUMENTATION */

/* DOCUMENTATION INCLUDES: */

/* https://wiki.megadrive.org/index.php?title=VDP */
/* http://md.railgun.works/index.php?title=VDP */

#ifndef VISUAL_DISPLAY_PORT
#define VISUAL_DISPLAY_PORT

/* NESTED INCLUDES */

#include "common.h"

/* SYSTEM INCLUDES */

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#if defined(USE_VDP)
#define USE_VDP
	#else
#define USE_VDP

	#if defined(USE_VDP_UTIL)
		#define USE_VDP_UTIL
			#else
		#define USE_VDP_UTIL

		#define		VDP_MAX_SPRITE_LINE		20
		#define		VDP_TMSS_MAX_LINE		4

		// DEFINE AN ENDIANESS PARSER FOR READING 
		// AND WRITING CONTENTS TO THE VDP

		// THIS IS BY BIT SHFITING THE LSB AND MSB
		// OF EACH ENDIAN TYPE


		#define		VDP_READ_LONG(ADDRESS)			\
					((U32)ADDRESS & 3) ?			\
					(								\
						*((U8*)ADDRESS)	+			\
						(*((U8*)ADDDRESS + 1) << 8) +	\
						(*((U8*)ADDDRESS + 2) << 16) +	\
						(*((U8*)ADDDRESS + 3) << 24)	\
					) :									\
					(*(U32*)(ADDRESS)) 
		

		#endif

#endif
#endif
