/* COPYRIGHT (C) HARRY CLARK 2024 */

/* SEGA MEGA DRIVE EMULATOR */

/* THIS FILE PERTAINS TO THE MAIN FUNCTIONALITY OF THE VIDEO DISPLAY PORT OF THE MEGA DRIVE */
/* TAKING INTO ACCOUNT THE INTRICACIES OF THE SYSTEM THROUGH VARIOUS PIECES OF DOCUMENTATION */

/* DOCUMENTATION INCLUDES: */

/* https://wiki.megadrive.org/index.php?title=VDP */
/* http://md.railgun.works/index.php?title=VDP */
 			U16 PAL;

/* NESTED INCLUDES */

#include "68000.h"
#include "md.h"
#include "vdp.h"
#include "common.h"

/* CREATE AN INSTANCE OF THE VDP BY ALLOCING THE SCREEN BUFFER */
/* THIS WILL CREATE VIRTUAL MEMORY ASSOCIATED WITH THE BYTEWISE SIZE */
/* OF THE UNIT */

#undef USE_VDP

static U8 PIXEL[0x100];
static U8 PIXEL_LUT[3][0x200];
static U8 PIXEL_LINE_BUFFER[2][0x200];

static VDP_BASE* VDP;
static VDP_BITMAP* VDP_BMP;

void(*RENDER_BG)(int LINE);
void(*RENDER_OBJ)(int LINE);
void(*UPDATE_BG_CACHE)(int INDEX);

//================================================
//           VDP INITIAL CO-ROUTINES
//================================================

void RENDER_INIT(void)
{
    int BIT_LAYER, ADDRESS_LAYER;
    U16 INDEX;

    /* INITIALISE THE PRIORITY OF LAYERS WITHIN THE PIXEL LOOK UP TABLES */
    /* READ A LITTLE ENDIAN INTO THE LOOKUP VALUE */

    for(BIT_LAYER = 0; BIT_LAYER < 0x100; BIT_LAYER++)
    {
        for(ADDRESS_LAYER = 0; ADDRESS_LAYER < 0x100; ADDRESS_LAYER++)
        {
            INDEX += (BIT_LAYER << 8) | (ADDRESS_LAYER);
        }
    }

    PALETTE_INIT();
}

void RENDER_RESET()
{
    // CLEAR DISPLAY SETTINGS

    memset(VDP_BMP->DATA, 0, VDP_BMP->PITCH * VDP_BMP->HEIGHT);

    // CLEAR LINE BUFFER

    memset(PIXEL_LINE_BUFFER, 0, sizeof(PIXEL_LINE_BUFFER));

    // CLEAR COLOUR PALETTE

    memset(PIXEL, 0, sizeof(PIXEL));
}

/* INITIALISES MODE 5 SUPPORT FOR PALETTE DEFINITION */
/* READS LITTLE ENDIAN INTO THE CRAM LOOKUP TABLE TO INITILAISE BASE COLOUR */

void PALETTE_INIT(void)
{
    int R, G, B, I;

    for(I = 0; I < 0x200; I++)
    {
        R = (I >> 0) & 7;
        G = (I >> 3) & 7;
        B = (I >> 6) & 7;

        // ALL OF THE BITS PER PIXEL - WHY NOT?

        #undef USE_32BPP

        PIXEL_LUT[0][I] = BIT_32_PIXEL(R, G, B);
        PIXEL_LUT[1][I] = BIT_32_PIXEL(R, G, B);
        PIXEL_LUT[2][I] = BIT_32_PIXEL(R, G, B);
    }
}

void VDP_INIT(void)
{
    // PAL AND NTSC TIMING
    // NTSC - 313
    // PAL - 262

    VDP->LINES_PER_FRAME = VDP->VDP_PAL ? 313 : 262;

    // DETERMINE THE SYSTEM TYPE AND SET THE CONCURRENT 
    // IRQ FROM THE VDP TO THE 68K - MAKING SURE THEY MATCH

    if((!SYSTEM_MD))
    {
        VDP->SET_IRQ = M68K_SET_SR_IRQ;
    }
}
