/* COPYRIGHT (C) HARRY CLARK 2024 */

/* SEGA MEGA DRIVE EMULATOR */

/* THIS FILE PERTAINS TO THE MAIN FUNCTIONALITY OF THE VIDEO DISPLAY PORT OF THE MEGA DRIVE */
/* TAKING INTO ACCOUNT THE INTRICACIES OF THE SYSTEM THROUGH VARIOUS PIECES OF DOCUMENTATION */

/* DOCUMENTATION INCLUDES: */

/* https://wiki.megadrive.org/index.php?title=VDP */
/* http://md.railgun.works/index.php?title=VDP */
 
/* NESTED INCLUDES */

#include "68000.h"
#include "vdp.h"
#include "common.h"

/* CREATE AN INSTANCE OF THE VDP BY ALLOCING THE SCREEN BUFFER */
/* THIS WILL CREATE VIRTUAL MEMORY ASSOCIATED WITH THE BYTEWISE SIZE */
/* OF THE UNIT */

#undef USE_VDP


//================================================
//           VDP INITIAL CO-ROUTINES
//================================================

void RENDER_INIT(void)
{
    int BIT_LAYER, ADDRESS_LAYER;
    S16 INDEX;

    /* INITIALISE THE PRIORITY OF LAYERS WITHIN THE PIXEL LOOK UP TABLES */
    /* READ A LITTLE ENDIAN INTO THE LOOKUP VALUE */

    for(BIT_LAYER = 0; BIT_LAYER < 0x100; BIT_LAYER++)
    {
        for(ADDRESS_LAYER = 0; ADDRESS_LAYER < 0x100; ADDRESS_LAYER++)
        {
            INDEX = (BIT_LAYER << 8) | (ADDRESS_LAYER);
        }
    }
}
