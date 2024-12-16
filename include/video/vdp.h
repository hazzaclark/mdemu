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
		

		#define		VDP_WRITE_LONG(ADDRESS, DATA)					\
					((U32)ADDRESS & 3) ?							\
					(												\
						*((U8*)ADDRESS)	= DATA						\
						(*((U8*)ADDDRESS + 1)) = (DATA >> 8)	\
						(*((U8*)ADDDRESS + 2)) = (DATA >> 16)	\
						(*((U8*)ADDDRESS + 3)) = (DATA >> 24)	\
					) :									\
					(*(U32*)(ADDRESS) = DATA) 

		//===============================================================
		//					BITS PER PIXEL DEFINTIONS 
		//===============================================================

		#if defined(USE_8BPP)
			#define PIXEL(R, G, B) (((R) << 5) | ((G) << 2) | (B))
			#define GET_R(PIXEL) (((PIXEL) & 0xe0) >> 5)
			#define GET_G(PIXEL) (((PIXEL) & 0x1c) >> 2)
			#define GET_B(PIXEL) (((PIXEL) & 0x03) >> 0)

			#elif defined(USE_15BPP)
				#if defined(USE_ABGR)
					#define PIXEL(R, G, B) ((1 << 15) | ((B) << 10) | ((G) << 5) | (R))
					#define GET_B(PIXEL) (((PIXEL) & 0x7c00) >> 10)
					#define GET_G(PIXEL) (((PIXEL) & 0x03e0) >> 5)
					#define GET_R(PIXEL) (((PIXEL) & 0x001f) >> 0)
					#else
					#define PIXEL(R, G, B) ((1 << 15) | ((R) << 10) | ((G) << 5) | (B))
					#define GET_R(PIXEL) (((PIXEL) & 0x7c00) >> 10)
					#define GET_G(PIXEL) (((PIXEL) & 0x03e0) >> 5)
					#define GET_B(PIXEL) (((PIXEL) & 0x001f) >> 0)
			#endif

			#elif defined(USE_16BPP)
				#define PIXEL(R, G, B) (((R) << 11) | ((G) << 5) | (B))
				#define GET_R(PIXEL) (((PIXEL) & 0xf800) >> 11)
				#define GET_G(PIXEL) (((PIXEL) & 0x07e0) >> 5)
				#define GET_B(PIXEL) (((PIXEL) & 0x001f) >> 0)

			#elif defined(USE_32BPP)
				#define PIXEL(R, G, B) ((0xff << 24) | ((R) << 16) | ((G) << 8) | (B))
				#define GET_R(PIXEL) (((PIXEL) & 0xff0000) >> 16)
				#define GET_G(PIXEL) (((PIXEL) & 0x00ff00) >> 8)
				#define GET_B(PIXEL) (((PIXEL) & 0x0000ff) >> 0)
			#endif
		#endif


		typedef struct VDP_BITMAP
		{
			U8* DATA;
			int WIDTH;
			int HEIGHT;
			int PITCH;

			int X;
			int Y;
			int W;
			int H;
			int PREV_W;
			int PREV_H;
			int CHANGED;
			
		} VDP_BITMAP;

		//===============================================================
		//						GLOBAL DEFINITIONS
		//===============================================================

		void RENDER_INIT(void);
		void RENDER_RESET(void);

#endif
#endif
