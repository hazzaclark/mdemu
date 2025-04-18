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

		#define		VDP_LINE_BUFFER			0x200 * 2

		#define		VDP_NTSC_TIMING			313
		#define 	VDP_PAL_TIMING			252

		#define		VDP_MAX_CYCLES_PER_LINE			3420
		#define		VDP_CLOCK_NTSC					53693175
		#define		VDP_CLOCK_PAL					53203424

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
		#define USE_8BPP
		#else
    		#define BIT_8_PIXEL(R, G, B) (((R) << 5) | ((G) << 2) | (B))
    		#define GET_8_R(PIXEL) (((PIXEL) & 0xe0) >> 5)
    		#define GET_8_G(PIXEL) (((PIXEL) & 0x1c) >> 2)
    		#define GET_8_B(PIXEL) (((PIXEL) & 0x03) >> 0)

		#endif

		#if defined(USE_15BPP)
		#define USE_15BPP
		#else
        	#define BIT_15_PIXEL(R, G, B) ((1 << 15) | ((B) << 10) | ((G) << 5) | (R))
        	#define GET_15_B(PIXEL) (((PIXEL) & 0x7c00) >> 10)
        #define GET_15_G(PIXEL) (((PIXEL) & 0x03e0) >> 5)
        #define GET_15_R(PIXEL) (((PIXEL) & 0x001f) >> 0)
    #endif

	#if defined(USE_16BPP)
		#define USE_16BPP
	#else
    	#define BIT_16_PIXEL(R, G, B) (((R) << 11) | ((G) << 5) | (B))
    	#define GET_16_R(PIXEL) (((PIXEL) & 0xf800) >> 11)
    	#define GET_16_G(PIXEL) (((PIXEL) & 0x07e0) >> 5)
    	#define GET_16_B(PIXEL) (((PIXEL) & 0x001f) >> 0)

	#endif

	#if defined(USE_32BPP)
	#define USE_32BPP
		#else
    	#define BIT_32_PIXEL(R, G, B) ((0xff << 24) | ((R) << 16) | ((G) << 8) | (B))
    	#define GET_32_R(PIXEL) (((PIXEL) & 0xff0000) >> 16)
    	#define GET_32_G(PIXEL) (((PIXEL) & 0x00ff00) >> 8)
    	#define GET_32_B(PIXEL) (((PIXEL) & 0x0000ff) >> 0)

	#endif
		
		typedef struct VDP_BASE
		{
			U8 VRAM[0x10000];
			U8 VSRAM[0x80];
			U8 CRAM[0x80];
			U8 VDP_REG[0x20];
			U8 HINT;
			U8 VINT;
			U16 STATUS;
			U32 DMA_LEN;
			U32 DMA_END_CYCLES;
			U8 DMA_TYPE;

			U16 A_BASE;
			U16 B_BASE;
			U16 W_BASE;
			U16 SPRITE_TABLE;
			U16 HORI_SCROLL;
			U8 VDP_PAL;
			U8 H_COUNTER;
			U16 V_COUNTER;
			U16 VC_MAX;
			U16 PAL;
			U16 LINES_PER_FRAME;
			U32 VINT_CYCLES;

			U32 HV_LATCH;
			S32 FIFO_IDX;
			S32* FIFO_TIMING;
			U32 FIFO_CYCLES[4];
			U32 VDP_CYCLES;

			U8* H_COUNTER_TABLE; 
			
			void(*SET_IRQ)(unsigned LEVEL);
			void(*SET_IRQ_DELAY)(unsigned LEVEL);

		} VDP_BASE;

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

		typedef struct VDP_PLANE
		{
			U8 LEFT;
			U8 RIGHT;
			U8 ENABLED;

		} VDP_PLANE;

		//===============================================================
		//						GLOBAL DEFINITIONS
		//===============================================================

		void RENDER_INIT(void);
		void RENDER_RESET(void);
		void PALETTE_INIT(void);
		void VDP_INIT(void);
		void VDP_RESET(void);
		void REMAP_LINE(int LINE);

		// ASSUME THAT THESE READ FUNCTIONS WILL BE MODE 5 BY DEFAULT

		void VDP_68K_WRITE(unsigned DATA);
		void VDP_68K_READ(void);
		void VDP_Z80_WRITE(unsigned DATA);
		void VDP_Z80_READ(void);

		unsigned VDP_READ_BYTE(unsigned ADDRESS);
		void VDP_WRITE_BYTE(unsigned ADDRESS, unsigned DATA);
		unsigned VDP_READ_WORD(unsigned ADDRESS);
		void VDP_WRITE_WORD(unsigned ADDRESS, unsigned DATA);


		int VDP_HV_READ(unsigned CYCLES);

		void VDP_BUS_WRITE(unsigned DATA);
		void VDP_REG_WRITE(unsigned REG, unsigned DEST, unsigned CYCLES);

		void VDP_DMA_68K_EXT(unsigned LEN);
		void VDP_DMA_68K_RAM(unsigned LEN);
		void VDP_DMA_68K_IO(unsigned LEN);
		void VDP_DMA_COPY(unsigned LEN);
		void VDP_DMA_FILL(unsigned LEN);

#endif
#endif
#endif
