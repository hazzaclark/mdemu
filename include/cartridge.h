/* COPYRIGHT (C) HARRY CLARK 2024 */

/* SEGA MEGA DRIVE EMULATOR */

/* THIS FILE PERTAINS TOWARDS THE FUNCTIONALITY SURROUDNING THE LOADING AND READING OF THE CARTRIDGE */

#ifndef MEGA_DRIVE_CARTRIDGE
#define MEGA_DRIVE_CARTRIDGE

/* NESTED INCLUDES */

#include "common.h"
#include "md.h"

/* SYSTEM INCLUDES */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#if defined(LOAD_MD_ROM)
#define LOAD_MD_ROM
#else
#define LOAD_MD_ROM

typedef struct ROM_INFO
{
    char TYPE[18];
    char COPYRIGHT[18];
    char DOMESTIC[50];
    char INTERNATIONAL[50];
    char ROM_TYPE[4];
    char SERIAL[14];
    
    unsigned short CHECKSUM;
    unsigned int START;
    unsigned int END;
    unsigned char REGION[18];

    S16* PERIPHERALS;

} ROM_INFO;

/* HARD CODED PRE-PROCESSOR DIRECTIVES FOR THE BITWISE VALUE OF HEADER INFORMATION */
/* OF A TRADITIONAL MEGA DRIVE HEADER */

#define         ROM_TYPE            384
#define         ROM_COPYRIGHT       272
#define         ROM_DOMESTIC        288
#define         ROM_INTERNATIONAL   336
#define         ROM_SERIAL          386
#define         ROM_CHECKSUM        398
#define         ROM_START           416
#define         ROM_END             440
#define         ROM_REGION          496
#define         MD_ROM_NAME_LEN      256

U16 GET_CHECKSUM(U8* ROM, unsigned LENGTH, char* FILENAME);
void MD_ROM_CHECKER(U8* SRC);
void MD_GET_ROM_INFO(char* HEADER);
int MD_LOAD_ROM(char* FILENAME);

#endif
#endif
