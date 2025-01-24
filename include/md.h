/* COPYRIGHT (C) HARRY CLARK 2024 */

/* SEGA MEGA DRIVE EMULATOR */

/* THIS FILE PERTAINS TOWARDS THE MAIN FUNCTIONALITY OF THE CONSOLE */
/* IN CONJUNCTION WITH THE METHODS ESTABLISHED IN THE CPU'S HEADER FILE */

#ifndef MEGA_DRIVE
#define MEGA_DRIVE

/* NESTED INCLUDES */

#include <68K.h>
#include "vdp.h"
#include "common.h"

/* SYSTEM INCLUDES */

#include <stdio.h>
#include <stdbool.h>

#if defined(USE_MD)
#define USE_MD
#else
#define USE_MD

#define     SYSTEM_MD           0x80        /* THE TYPICAL ENTRY POINT OF A MD CART */
#define     SYSTEM_MISC         0x81
#define     ZBUFFER_MAX         256
#define     ZBANK_MAX_RAM       4096
#define     CART_MAX_SIZE       32 * 1024 * 1024

#define     MD_CART_BANK_DEFAULT        0
#define     MD_CART_BANK_UNUSED         0xFF
#define     MD_CART_BANK_RO             1
#define     MD_CART_ROM_SIZE            (2 * 1024 * 1024)

typedef struct MD_CART
{
    U8* ROM_BASE;
    U8* ROM_ADDON;
    U8 CARTRIDGE_REGS[4];
    U16 CARTRIDGE_RESET;
    U32 CARTRIDGE_BANKS[4];
    U32 CARTRIDGE_MASK[4];
    U32 CARTRIDGE_ADDRESS[4];
    U32 ROM_SIZE;
    U32* ROM_DATA;

    unsigned(*TICK_TIMER)(unsigned ADRRESS);
    unsigned(*REGISTER_READ)(unsigned ADDRESS);
    void(*REGISTER_WRITE)(unsigned ADDRESS, unsigned DATA);

    const char* ROM_SERIAL;
    const char* ROM_DOMESTIC;
    const char* ROM_INTER;

    U32(*ROM_LOAD_CRC)(void);
    U32(*ROM_SRAM_INIT)(U32* INFO, U32* START, U32* END);

    int* ROM_MAP;

} MD_CART;

typedef struct MD
{
    MD_CART* MD_CART;
    U8* BOOT_ROM[0x800];
    U8* BOOT_RAM[0x10000];
    U8* SYS_ROM;
    U8* SYS_RAM;
    U8* ZRAM[0x2000];
    U8 ZSTATE;
    U8 SYSTEM_BIOS;
    U32* ZBANK[ZBANK_MAX_RAM];
    U8 MEMORY_CUR_PAGE;
    U8* TMSS[4];
    U8* SYSTEM_TYPE;

    bool IS_TMSS;

} MD; 

typedef enum MD_RESET_MODE
{
    MODE_SOFT,
    MODE_HARD,
    NONE,

} MD_RESET_MODE;

typedef enum MD_CART_MAP_MODE
{
    MAPPER_NORMAL,
    MAPPER_READONLY,
    MAPPER_UNUSED,

} MD_CART_MAP_MODE;

void MD_MAKE(void);
void MD_INIT(void);
void MD_RESET(void);
void MD_ADDRESS_BANK_WRITE(unsigned DATA);
void MD_ADDRESS_BANK_READ(void);
void MD_BUS_REQ(unsigned STATE, unsigned CYCLES);
int MD_CART_INIT(struct MD_CART* CART, unsigned char* DATA, unsigned long* SIZE);
void MD_CART_RESET(int const RESET_TYPE);
S32(*MD_CART_CONTEXT(U8* STATE))(void);
U32(*MD_BANKSWITCH());
void MD_CART_MEMORY_MAP(void);

#endif

#endif
