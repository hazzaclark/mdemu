/* COPYRIGHT (C) HARRY CLARK 2024 */

/* SEGA MEGA DRIVE EMULATOR */

/* THIS FILE PERTAINS TO THE MAIN FUNCTIONALITY OF THE VIDEO DISPLAY PORT OF THE MEGA DRIVE */
/* TAKING INTO ACCOUNT THE INTRICACIES OF THE SYSTEM THROUGH VARIOUS PIECES OF DOCUMENTATION */

/* DOCUMENTATION INCLUDES: */

/* https://wiki.megadrive.org/index.php?title=VDP */
/* http://md.railgun.works/index.php?title=VDP */

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
void(*PARSE_SPRITE_TABLE)(int LINE);
void(*UPDATE_BG_CACHE)(int INDEX);


//================================================
//           VDP INITIAL CO-ROUTINES
//================================================

void VDP_INIT(void)
{
    VDP = malloc(sizeof(struct VDP_BASE));

    if(VDP == NULL)
    {
        perror("Memory Allocation failed for VDP\n");
        exit(EXIT_FAILURE);
    }


    // PAL AND NTSC TIMING
    // NTSC - 313
    // PAL - 262

    VDP->LINES_PER_FRAME = VDP->VDP_PAL ? VDP_NTSC_TIMING : VDP_PAL_TIMING;

    // DETERMINE THE SYSTEM TYPE AND SET THE CONCURRENT 
    // IRQ FROM THE VDP TO THE 68K - MAKING SURE THEY MATCH

    if((!SYSTEM_MD))
    {
        VDP->SET_IRQ = M68K_SET_SR_IRQ;
    }
}

void VDP_RESET(void)
{
    memset(VDP->SPRITE_TABLE, 0, sizeof(VDP->SPRITE_TABLE));
    memset(*(char*) VDP->VRAM, 0, sizeof(VDP->VRAM));
    memset(*(char*) VDP->CRAM, 0, sizeof(VDP->CRAM));
    memset(*(char*) VDP->VSRAM, 0, sizeof(VDP->VSRAM));
    memset(*(char*) VDP->VDP_REG, 0, sizeof(VDP->VDP_REG));

    VDP->HINT = 0;
    VDP->VINT = 0;
    VDP->DMA_LEN = 0;
    VDP->DMA_TYPE = 0;
    VDP->DMA_END_CYCLES = 0;

    VDP->A_BASE = 0;
    VDP->B_BASE = 0;
    VDP->W_BASE = 0;
    VDP->SPRITE_TABLE = 0;
    VDP->HORI_SCROLL = 0;

}

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


void RENDER_LINE(int LINE)
{
    // CHECK IF THE DISPLAY FLAG HAS BEEN SET ACCORDING TO
    // THE REGISTER VALUE

    if(VDP->VDP_REG[1] & 0x40)
    {
        RENDER_BG(LINE);
        RENDER_OBJ(LINE & 1);
    }

    // CHECK THE LEFT MOST COLUMN TO INIT THE V AND HBLANK

    if(VDP->VDP_REG[0] & 0x20)
    {
        memset(&PIXEL_LINE_BUFFER[0][0x20], 0x40, 8);
    }

    // PARSE SPRITES FOR THE NEXT LINE

    if(LINE < (VDP_BMP->H - 1))
    {
        PARSE_SPRITE_TABLE(LINE);
    }

    // CHECK FOR THE HORIZONTAL BORDERS OF THE VIEW

    if(VDP_BMP->X > 0)
    {
        memset(&PIXEL_LINE_BUFFER[0][0x20 - VDP_BMP->X], 0x40, VDP_BMP->X);
        memset(&PIXEL_LINE_BUFFER[0][0x20 + VDP_BMP->W], 0x40, VDP_BMP->W);
         
    }

    // IF THE FIRST LINE HASNT BEEN RENDERED

    memset(&PIXEL_LINE_BUFFER[0][0x20 - VDP_BMP->X], 0x40, VDP_BMP->W + 2 * VDP_BMP->X);
    REMAP_LINE(LINE);
}

// THIS WILL LOOK INTO BEING ABLE TO REMAP THE RENDER COROUTINE
// AFTER EVERY CONSECTUTIVE LINE RENDER

void REMAP_LINE(int LINE)
{
    U8* LINE_SRC_BUFFER = &PIXEL_LINE_BUFFER[0][0x20 - VDP_BMP->X];
    LINE = (LINE + VDP_BMP->Y) % VDP->LINES_PER_FRAME;

    // USE THE DEFAULT PER PIXEL MODE
    // THE IDEA IS TO TAKE THE DESTINATION BASED ON THE PITCH (RENDER SPEED)

    // STORE THE SRC LINE IN THE DEST BUFFER

    #undef USE_15BPP

    U8* DESTINATION = ((U8*)&VDP_BMP->DATA[(LINE * VDP_BMP->PITCH)]);
    *DESTINATION++ = PIXEL[*LINE_SRC_BUFFER]++;
}

// READ THE CORRESPONDING INFO BEING PASSED THROUGH THE 
// HORIZONTAL AND VERTICAL COUNTERS

int VDP_HV_READ(unsigned CYCLES)
{
    int COUNTER = 0;
    unsigned DATA = VDP->HV_LATCH;

    // CHECK IF THE HV LATCH HAS BEEN SET/ENABLED

    if(DATA && (VDP->VDP_REG[1] & 0x04))
    {
        fprintf("[%d(%d)][%d(%d)] HVC LATCH READ - 0x%x (%x)\n", 
        VDP->V_COUNTER,  // CURRENT COUNTER VALUE
                        // ADJUSTED COUNTER VAL
        (VDP->V_COUNTER + CYCLES - VDP->VDP_CYCLES / VDP_MAX_CYCLES_PER_LINE) % VDP->LINES_PER_FRAME,
        CYCLES % VDP_MAX_CYCLES_PER_LINE,  // CURRENT CYCLE
        DATA,
        0xFFFF,                           // MASK
        M68K_REG_PC);

        // RETURN THE LATCHED VALUE

        return (DATA & 0xFFFF);

    }

    else
    {
        // DRAW THE LATEST DATA FROM THE HCOUNTER BASED ON THE CONCURRENT CYCLES

        DATA = VDP->H_COUNTER_TABLE[CYCLES % VDP_MAX_CYCLES_PER_LINE];
    }

    COUNTER = VDP->V_COUNTER;

    // CHECK IF THE CURRENT AMOUNT OF CYCLES CORRESPONDS WITH THE MAX CYCLES

    if((CYCLES - VDP->VDP_CYCLES)) { COUNTER = COUNTER + 1 % VDP_MAX_CYCLES_PER_LINE; }

    // RETURN H COUNTER IN LITTLE ENDIAN
    // RETURN V COUNTER VICE VERSA

    DATA |= ((COUNTER & 0xFF) >> 8);

    fprintf("[%d(%d)][%d(%d)] HVC READ - 0x%x (%x)\n", 
        VDP->V_COUNTER,  // CURRENT COUNTER VALUE
                        // ADJUSTED COUNTER VAL
        (VDP->V_COUNTER + CYCLES - VDP->VDP_CYCLES / VDP_MAX_CYCLES_PER_LINE) % VDP->LINES_PER_FRAME,
        CYCLES % VDP_MAX_CYCLES_PER_LINE,  // CURRENT CYCLE
        DATA,
        0xFFFF,                           // MASK
        M68K_REG_PC);


    return DATA;
}

// THESE READ AND WRITE FUNCTIONS WILL ENCOMPASS ALL POSSIBLE
// DMA MODES BY DEFAULT

// THE VDP IS SPLIT UP INTO TWO MAIN MODES - MODE 4 AND 5
// MODE 4 IS FOR MASTER SYSTEM COMP 
// MODE 5 IS FOR EVERYTHING ELSE MEGA DRIVE RELATED

// MODES 1 TO 3 WILL BE HANDLED THROUGH VARIOUS OTHER MEANS
// WHICH WILL ENCOMPASS BIT SHIFTS FOR TILING FOR EXAMPLE 

// SEE: https://md.railgun.works/index.php?title=VDP#.2401_-_Mode_Register_4

void VDP_68K_WRITE(unsigned DATA)
{
    int PROC_SLOT = 0;
    int CYCLES = M68K_CYC_INSTRUCTION;

    // CHECK IF THERE IS ANY AND ALL RESTRICTED WRITE ACCESS
    // THROUGH THIS, CHECK AGAINST LAST FIFO READ OUT CHUNK

    // FROM THERE, DETERMINE THE NEXT ENTRY POINT AND WRITE
    // IN ACCORDANCE WITH THE INDEX

    if((!VDP->STATUS) & 8 && (VDP->VDP_REG[1] & 0x40))
    {
        CYCLES = VDP->FIFO_CYCLES[VDP->FIFO_IDX + 3] % 3;
        CYCLES -= VDP->VDP_CYCLES;

        VDP->FIFO_CYCLES[VDP->FIFO_IDX] = VDP->VDP_CYCLES + VDP->FIFO_TIMING[PROC_SLOT];
    }

    // CHECK AGAINST THE LAST FIFO READ OUT CYCLE

    if(CYCLES < VDP->FIFO_CYCLES[(VDP->FIFO_IDX + 3 % 3)])
    {
        // EITHER FIFO IS FULL OR EMPTY

        CYCLES = ((VDP->FIFO_CYCLES[VDP->FIFO_IDX] + 6) / 7) * 7;
    }

    // OTHERWISE, FIFO IS EMPTY, NEXT FIFO ENTRY WILL BE PROCESSED AFTER LAST

    CYCLES = VDP->FIFO_CYCLES[VDP->FIFO_IDX + 3] & 3;

    VDP_BUS_WRITE(DATA);
}

// NOW WE WRITE THE CONTENTS OF THE AFOREMENTIONED TO THE BUS

void VDP_BUS_WRITE(unsigned DATA)
{
    // INIT THE FIFO CYCLES BASED ON THE CURRENT DATA BEING PASSED

    VDP->FIFO_CYCLES[VDP->FIFO_IDX] = DATA;
}
