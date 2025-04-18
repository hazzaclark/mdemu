/* COPYRIGHT (C) HARRY CLARK 2024 */

/* SEGA MEGA DRIVE EMULATOR */

/* THIS FILE PERTAINS TO THE MAIN FUNCTIONALITY OF THE VIDEO DISPLAY PORT OF THE MEGA DRIVE */
/* TAKING INTO ACCOUNT THE INTRICACIES OF THE SYSTEM THROUGH VARIOUS PIECES OF DOCUMENTATION */

/* DOCUMENTATION INCLUDES: */

/* https://wiki.megadrive.org/index.php?title=VDP */
/* http://md.railgun.works/index.php?title=VDP */

/* NESTED INCLUDES */

#include <68K.h>
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

VDP_BASE* VDP = NULL;
static VDP_BITMAP* VDP_BMP;

void(*RENDER_BG)(int LINE);
void(*RENDER_OBJ)(int LINE);
void(*PARSE_SPRITE_TABLE)(int LINE);
void(*UPDATE_BG_CACHE)(int INDEX);

unsigned(*VDP_DATA_R)(void);
void(*VDP_DATA_W)(void);

unsigned(*VDP_CTRL_R)(unsigned CYCLES);
void(*VDP_CTRL_W)(unsigned DATA);

//================================================
//           VDP INITIAL CO-ROUTINES
//================================================

void VDP_INIT(void) 
{
    VDP = malloc(sizeof(VDP_BASE));
    if (VDP == NULL) 
    {
        perror("Memory Allocation failed for VDP\n");
        exit(EXIT_FAILURE);
    }

    memset(VDP->VDP_REG, 0, sizeof(VDP->VDP_REG));
    memset(VDP->VRAM, 0, sizeof(VDP->VRAM));
    memset(VDP->CRAM, 0, sizeof(VDP->CRAM));
    memset(VDP->VSRAM, 0, sizeof(VDP->VSRAM));

    VDP->VDP_REG[0] = 0x04; // Mode Register 1
    VDP->VDP_REG[1] = 0x74; // Mode Register 2 (Enable display, enable VBLANK interrupt)
    VDP->VDP_REG[2] = 0x30; // Plane A Name Table Address
    VDP->VDP_REG[3] = 0x3C; // Window Name Table Address
    VDP->VDP_REG[4] = 0x07; // Plane B Name Table Address
    VDP->VDP_REG[5] = 0x6C; // Sprite Table Address
    VDP->VDP_REG[6] = 0x00; // Sprite Pattern Generator Base Address
    VDP->VDP_REG[7] = 0x00; // Background Color
    VDP->VDP_REG[8] = 0x00; // Unused
    VDP->VDP_REG[9] = 0x00; // Unused
    VDP->VDP_REG[10] = 0xFF; // HBLANK Counter
    VDP->VDP_REG[11] = 0x00; // External Interrupt Enable
    VDP->VDP_REG[12] = 0x81; // Mode Register 3
    VDP->VDP_REG[13] = 0x37; // Mode Register 4
    VDP->VDP_REG[14] = 0x00; // HScroll Data Address
    VDP->VDP_REG[15] = 0x02; // Auto-Increment Value
    VDP->VDP_REG[16] = 0x01; // Plane Size
    VDP->VDP_REG[17] = 0x00; // Window Plane Horizontal Position
    VDP->VDP_REG[18] = 0x00; // Window Plane Vertical Position
    VDP->VDP_REG[19] = 0x00; // DMA Length Low
    VDP->VDP_REG[20] = 0x00; // DMA Length High
    VDP->VDP_REG[21] = 0x00; // DMA Source Address Low
    VDP->VDP_REG[22] = 0x00; // DMA Source Address Mid
    VDP->VDP_REG[23] = 0x80; // DMA Source Address High

    for (int i = 0; i < 0x40; i += 2) 
    {
        VDP->CRAM[i] = 0x0E; 
        VDP->CRAM[i + 1] = 0x0E;
    }

    VDP->HINT = 0;
    VDP->VINT = 0;
    VDP->STATUS = 0;
    VDP->DMA_LEN = 0;
    VDP->DMA_END_CYCLES = 0;
    VDP->DMA_TYPE = 0;
    VDP->A_BASE = 0;
    VDP->B_BASE = 0;
    VDP->W_BASE = 0;
    VDP->SPRITE_TABLE = 0;
    VDP->HORI_SCROLL = 0;
    VDP->VDP_PAL = 0;
    VDP->H_COUNTER = 0;
    VDP->V_COUNTER = 0;
    VDP->VC_MAX = 0;
    VDP->PAL = 0;
    VDP->LINES_PER_FRAME = VDP->VDP_PAL ? VDP_NTSC_TIMING : VDP_PAL_TIMING;
    VDP->VINT_CYCLES = 0;
    VDP->HV_LATCH = 0;
    VDP->FIFO_IDX = 0;
    VDP->FIFO_TIMING = NULL;

    for (int i = 0; i < 4; i++) 
    {
        VDP->FIFO_CYCLES[i] = 0;
    }

    VDP->VDP_CYCLES = 0;
    VDP->H_COUNTER_TABLE = NULL;
    VDP->SET_IRQ = NULL;
    VDP->SET_IRQ_DELAY = NULL;

    printf("VDP initialized: %p\n", (void*)VDP);
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
        CPU.PC);

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
        CPU.PC);


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
    unsigned CYCLES = M68K_CYCLES_REMAINING;

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

    if(CYCLES < (VDP->FIFO_CYCLES[(VDP->FIFO_IDX + 3 % 3)]))
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

unsigned VDP_READ_BYTE(unsigned ADDRESS)
{
    switch(ADDRESS & 0xFD)
    {
        case 0x00:
        {
            return (VDP_DATA_R() >> 8);
        }

        case 0x01:
        {
            return (VDP_DATA_R() & 0xFF);
        }

        case 0x04:
        {
            unsigned DATA = (VDP_CTRL_R(M68K_CYCLE) >> 8);
            ADDRESS = M68K_REG_PC;
            return DATA;
        }

        case 0x05:
        {
            return (VDP_CTRL_R(M68K_CYCLE) & 0xFF);
        }

        case 0x08:
        case 0x0C:
        {
            return (VDP_HV_READ(M68K_CYCLE) >> 8);
        }

        case 0x09:
        case 0x0D:
        {
            return (VDP_HV_READ(M68K_CYCLE) & 0xFF);
        }

        default:
            return M68K_READ_8(ADDRESS);
    }
}

unsigned VDP_READ_WORD(unsigned ADDRESS)
{
    switch (ADDRESS & 0xFC)
    {
        case 0x00:
        {
            return VDP_DATA_R();
        }

        case 0x04:
        {
            unsigned DATA = VDP_HV_READ(M68K_CYCLE) & 0x3FF;
            ADDRESS = M68K_PC;
            DATA |= (*(U16*)(CPU.MEMORY_MAP[((ADDRESS) >> 16) & 0xFF].MEMORY_BASE + ((ADDRESS) & 0xFFFF)) & 0xFC00);

            return DATA;
        }

        case 0x08:
        case 0x0C:
        {
            return VDP_HV_READ(M68K_CYCLE);
        }
    
        default:
        {
            return M68K_READ_16(ADDRESS);
        }
    }
}

void VDP_WRITE_BYTE(unsigned ADDRESS, unsigned DATA)
{
    switch (ADDRESS)
    {
        case 0x00:
        {
            VDP_68K_WRITE(DATA << 8);
            return;
        }

        case 0x04:
        {
            VDP_CTRL_W(DATA << 8);
            return;
        }

        case 0x10:
        case 0x14:
        {
            if(ADDRESS & 1)
            {
                return;
            }

            return;
        }

        default:
            M68K_WRITE_8(ADDRESS, DATA);
            break;
    }
}

void VDP_DEBUG_OUTPUT(void) 
{
    if (VDP == NULL) {
        printf("VDP is NULL. Cannot output debug information.\n");
        return;
    }

    printf("VDP Debug Information:\n");
    printf("======================\n");

    printf("VDP Registers:\n");
    for (int i = 0; i < 0x20; i++) 
    {
        printf("Register 0x%02X: 0x%02X\n", i, VDP->VDP_REG[i]);
    }

    printf("\nVRAM Contents (first 256 bytes):\n");
    for (int i = 0; i < 256; i++) 
    {
        if (i % 16 == 0) printf("\n0x%04X: ", i);
        printf("%02X ", VDP->VRAM[i]);
    }

    printf("\n\nCRAM Contents:\n");
    for (int i = 0; i < 0x80; i++) 
    {
        if (i % 16 == 0) printf("\n0x%04X: ", i);
        printf("%02X ", VDP->CRAM[i]);
    }

    printf("\n\nVSRAM Contents:\n");
    for (int i = 0; i < 0x80; i++) 
    {
        if (i % 16 == 0) printf("\n0x%04X: ", i);
        printf("%02X ", VDP->VSRAM[i]);
    }

    printf("\n\nStatus Flags:\n");
    printf("HINT: 0x%02X\n", VDP->HINT);
    printf("VINT: 0x%02X\n", VDP->VINT);
    printf("STATUS: 0x%04X\n", VDP->STATUS);

    printf("\nDMA Information:\n");
    printf("DMA_LEN: 0x%08X\n", VDP->DMA_LEN);
    printf("DMA_END_CYCLES: 0x%08X\n", VDP->DMA_END_CYCLES);
    printf("DMA_TYPE: 0x%02X\n", VDP->DMA_TYPE);

    printf("\nBase Addresses:\n");
    printf("A_BASE: 0x%04X\n", VDP->A_BASE);
    printf("B_BASE: 0x%04X\n", VDP->B_BASE);
    printf("W_BASE: 0x%04X\n", VDP->W_BASE);
    printf("SPRITE_TABLE: 0x%04X\n", VDP->SPRITE_TABLE);
    printf("HORI_SCROLL: 0x%04X\n", VDP->HORI_SCROLL);

    printf("\nCounters:\n");
    printf("H_COUNTER: 0x%02X\n", VDP->H_COUNTER);
    printf("V_COUNTER: 0x%04X\n", VDP->V_COUNTER);
    printf("VC_MAX: 0x%04X\n", VDP->VC_MAX);
    printf("PAL: 0x%04X\n", VDP->PAL);
    printf("LINES_PER_FRAME: 0x%04X\n", VDP->LINES_PER_FRAME);
    printf("VINT_CYCLES: 0x%08X\n", VDP->VINT_CYCLES);

    printf("\nFIFO Information:\n");
    printf("FIFO_IDX: %d\n", VDP->FIFO_IDX);

    for (int i = 0; i < 4; i++) 
    {
        printf("FIFO_CYCLES[%d]: 0x%08X\n", i, VDP->FIFO_CYCLES[i]);
    }
    printf("VDP_CYCLES: 0x%08X\n", VDP->VDP_CYCLES);

    printf("\nEnd of VDP Debug Information\n");
    printf("============================\n");
}
