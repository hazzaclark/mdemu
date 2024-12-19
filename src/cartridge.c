/* COPYRIGHT (C) HARRY CLARK 2024 */

/* SEGA MEGA DRIVE EMULATOR */

/* THIS FILE PERTAINS TOWARDS THE FUNCTIONALITY SURROUDNING THE LOADING AND READING OF THE CARTRIDGE */

/* NESTED INCLUDES */

#include "cartridge.h"

#ifdef LOAD_MD_ROM

/* RETURN THE VALUE OF THE DESIGNATED CHECKSUM FROM THE PROVIDED ROM FILE */
/* A PRINT UTILITY WILL BE PROVIDED TO DEBUG THIS UTILIY */

U16 GET_CHECKSUM(U8* ROM, unsigned LENGTH, char* FILENAME)
{
    unsigned INDEX = 0;
    unsigned CHECKSUM = 0;

    for (INDEX = 0; INDEX < LENGTH; INDEX += 2)
    {
        CHECKSUM += ((ROM[INDEX] << 8) + ROM[INDEX + 1]); 
    }

    printf("Checksum for ROM: %s, 0x%x\n", FILENAME, CHECKSUM);
    return CHECKSUM;
}

/* PASS A RAW POINTER THROUGH THE HEADER OF THE ROM */
/* FROM THERE, PARSE ANY AND ALL SUBSEQUENT INFORMATION THAT PERTAINS */
/* TOWARDS THE STRUCTURE */

void MD_GET_ROM_INFO(char* HEADER)
{
    struct ROM_INFO* ROM = malloc(sizeof(struct ROM_INFO));

    int INDEX = 0;
    int ITERATOR = 0;
    unsigned char SYSTEM_TYPE = 0;
    unsigned short OFFSET = 0;

    /* THROUGH EVERY SUBSEQUENT READ OF THE ROM HEADER */
    /* CLEAR THE READER BUFFER, THAT WAY MISMATCHED DATA DOESN'T GET TRANSCODED */
    /* WHEN OTHER ROMS ARE LOADED */

    memset(ROM, 0, sizeof(struct ROM_INFO));

    if(ROM == NULL)
    {
        fprintf(stderr, "Could not evaluate space for the ROM Header");
        exit(1);
    }

    if(SYSTEM_TYPE == SYSTEM_MD)
    {
        /* EVALUATE THE PRE-REQUISITES FROM THE ENTRY POINT */
        /* TO AVOID AN OVERFLOW IN DESIGNATED MEMORY BEING APPLIED, IT WILL BE DYNAMICALLY */
        /* EVALUATED BASED ON THE ROM LOADED IN - RATHER THAN A HARD CODED VALUE */

        memcpy(ROM->TYPE, HEADER, sizeof(ROM->TYPE));

        for(INDEX = 1; INDEX < 48; INDEX++)
        {
            ROM->DOMESTIC[ITERATOR] = HEADER[288 + INDEX];
            ITERATOR++;
        }
    }

    ROM->DOMESTIC[ITERATOR] = 0;

    /* NOW ASSERT THE PRE-REQUISITE INFORMATION BASED ON ROM HEADER INFO */

    memcpy(ROM->TYPE, HEADER + ROM_TYPE, 2);
    memcpy(ROM->SERIAL, HEADER + ROM_SERIAL, 12);
    memcpy(ROM->CHECKSUM, HEADER + ROM_CHECKSUM, 2);
    memcpy(ROM->START, HEADER + ROM_START, 4);
    memcpy(ROM->END, HEADER + ROM_END, 4);
    memcpy(ROM->INTERNATIONAL, HEADER + ROM_INTERNATIONAL, 16);

    /* FROM THERE, WE WILL BEGIN TO EVALUATE THE MEMORY ADDRESSES */\
    /* OF EACH RESPECTIVE ENTITY */

    /* THIS WILL BE DONE BASED ON A BIT MASK BETWEEN 0 AND 15  (16 BIT CHECKSUM) */

    ROM->START = 0;
    switch(HEADER[OFFSET - 0x0F])
    {
        case 0x00:
          ROM->END = 0x3FFFF;
          break;
        case 0x01:
          ROM->END = 0x7FFFF;
          break;
        case 0x02:
          ROM->END = 0xFFFFF;
          break;
        case 0x0A:
          ROM->END = 0x1FFF;
          break;
        case 0x0B:
          ROM->END = 0x3FFF;
          break;
        case 0x0C:
          ROM->END = 0x7FFF;
          break;
        case 0x0D:
          ROM->END = 0xBFFF;
          break;
        case 0x0E:
          ROM->END = 0xFFFF;
          break;
        case 0x0F:
          ROM->END = 0x1FFFF;
          break;
    }

    free(ROM);
}

/* LOAD ROM INTO THE CURRENT BUFFER BASED ON THE SIZE OF THE CURRENT STRUCT */
/* OF THE ROM ITSELF */

int MD_LOAD_ROM(char* FILENAME)
{
    char* ROM_MAX_FILENAME = malloc(MD_ROM_NAME_LEN);
    U8 SYS_TYPE = 0;
    struct MD_CART* CART = malloc(sizeof(struct MD_CART));

    /* EVALUATE THE SIZE AND STORE IT IN THE ROM INFO */

    CART->ROM_SIZE = 0x8000000;
    CART->ROM_BASE = sizeof(char*);
    MD_GET_ROM_INFO((char *)CART->ROM_BASE);

    if((SYS_TYPE == SYSTEM_MD) && (CART->ROM_SIZE <= 0x8000000))
    {
        strncpy(ROM_MAX_FILENAME, FILENAME, MD_ROM_NAME_LEN - 1);
        ROM_MAX_FILENAME[MD_ROM_NAME_LEN - 1] = '\0';
    }

    return 0;
}

#endif
