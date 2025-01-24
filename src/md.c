/* COPYRIGHT (C) HARRY CLARK 2024 */

/* SEGA MEGA DRIVE EMULATOR */

/* THIS FILE PERTAINS TOWARDS THE MAIN FUNCTIONALITY OF THE CONSOLE */

/* NESTED INCLUDES */

#include "md.h"
#include "common.h"

#ifdef USE_MD

static MD* MD_CONSOLE;
static MD_CART* MD_CARTRIDGE;

static U8 WORK_RAM[0x10000];

/* INITIALISE THE CONSOLE THROUGH THE PRE-REQUISTIES */
/* ESTABLISHED IN THE CORRESPONDING HEADER FILES */

/* THIS FUNCTION ENCOMPASSES THE FUNCTIONALITY OF ENABLING */
/* THE MEMORY MANAGEMENT UNIT FOR ALLOWING THE CONSOLE TO BEGIN */
/* IT'S INITIAL COMMUNICATIONS BETWEEN M68K AND Z80 ON STARTUP */

void MD_INIT(void)
{    
    M68K_INIT();

    /* ASSUMING THAT THE ABOVE COROUTINE HAVE BEEN ESTABLISHED */
    /* THE MEMORY MAP WILL NOW BE INITIALISED */

    /* THIS FUNCTIONALITY IS SIMILAR TO THE WAY IN WHICH IT WORKS */
    /* ON REAL HARDWARE WITH THE WAY IN WHICH THE VECTOR TABLE INITIALISES */ 

    for (int i = 0; i < 0xFF; i++)
    {
        CPU->MEMORY_MAP[i].MEMORY_BASE = WORK_RAM;
        CPU->MEMORY_MAP[i].MEMORY_READ_8 = M68K_READ_8;
        CPU->MEMORY_MAP[i].MEMORY_WRITE_8 = M68K_WRITE_8;
        CPU->MEMORY_MAP[i].MEMORY_READ_16 = M68K_READ_16;
        CPU->MEMORY_MAP[i].MEMORY_WRITE_16 = M68K_WRITE_16;
    }

    /* VDP ENTRY POINTS */

    for (int i = 0xC; i < 0xFF; i++)
    {
        CPU->MEMORY_MAP[i].MEMORY_READ_8 =  CTRL_READ_BYTE;
        CPU->MEMORY_MAP[i].MEMORY_WRITE_8 = M68K_WRITE_8;
        CPU->MEMORY_MAP[i].MEMORY_READ_16 = CTRL_READ_WORD;
        CPU->MEMORY_MAP[i].MEMORY_WRITE_16 = M68K_WRITE_16;
    }

    /* IO CONTROL REGISTERS */

    for (int i = 0; i < 0xA100; i++)
    {
        CPU->MEMORY_MAP[0xA1].MEMORY_READ_8 = CTRL_READ_BYTE;
        CPU->MEMORY_MAP[0xA1].MEMORY_WRITE_8 = CTRL_WRITE_BYTE;
        CPU->MEMORY_MAP[0xA1].MEMORY_READ_8 = CTRL_READ_WORD;
        CPU->MEMORY_MAP[0xA1].MEMORY_WRITE_8 = CTRL_WRITE_WORD;
    } 
}


/* NOW COMES THE COROUTINE FOR RESETTING THE CONSOLE */
/* THIS WILL DETERMINE BY AN NUMERICAL VALUE TO DISCERN THE RESET TYPE */

/* WHEN IT COMES TO RESET METHODS ON THE MEGA DRIVE, ESPECIALLY IN THE HEADER */
/* OF THE MAIN ASSEMBLY FILE, IT INVOLVES THE MOVE INSTRUCTION OF THE VALUE FROM D7 */
/* INTO ONE OF THREE ADDRESSING MODES */

/* A1, A2 & A3 ENCOMPASS THE INITIAL STEPS FOR HARDWARE COROUTINE CHECKS */
/* AS THESE ARE THE MAIN 3 REGISTERS THAT COMMUNICATE WITH THE BUS */

/* D7 ACTS AS THE STACK POINTER TO DETERMINE WHERE THE DATA SHOULD GO TOWARDS */

/* SEE 68K INSTRUCTION REF. https://md.railgun.works/index.php?title=68k_Instruction_Reference */

void MD_RESET(void)
{
    MD_RESET_MODE MODE = 0;

    switch (MODE)
    {
        /* SOFT RESET EVOKES THE METHODS USED TO */
        /* RESET THE CONSOLE FROM A SOFTWARE */
        /* THIS WILL BE GOVERNED BY THE STACK POINTER */
        /* STORING THE LOCATION OF THE BOOT RAM CACHE */

        /* FROM THERE, BOOT BACK TO THE START OF THE PROGRAM EXECUTION */
        /* IN RELATION TO THE BOOT RAM GOVERNED BY IT'S DESIGNATED DATA REGISTER */

        case MODE_SOFT:
            CPU->PC = MD_CONSOLE->BOOT_RAM;
            CPU->STACK_POINTER = 0x2700;
            CPU->REGISTER_BASE[7] = MD_CONSOLE->BOOT_RAM;
            break;

        /* HARD RESET ENVOKES THAT ALL ASPECTS OF THE CONSOLE NEED TO BE */
        /* RESET INDICATIVE OF THE PRESSING OF THE RESET BUTTON */

        /* IN ASSEMBLY, THIS WOULD TYPICALLY ENTAIL USING THE RESET FLAG */
        /* TO RESET CPU EXECUTION BASED ON THEIR REGISTER COUNT */

        /* THE DIFFERENCE BEING IS THAT WE EVOKE MEMSET TO ASSERT ALL VALUES */
        /* BACK TO DEFAULT */

        case MODE_HARD:
            CPU->PC = MD_CONSOLE->BOOT_RAM;
            CPU->STACK_POINTER = 0x2700;
            CPU->REGISTER_BASE[7] = MD_CONSOLE->BOOT_RAM;
            memset(MD_CONSOLE->BOOT_RAM, 0x00, sizeof(MD_CONSOLE->BOOT_RAM));
            memset(MD_CONSOLE->ZRAM, 0x00, sizeof(MD_CONSOLE->ZRAM));
            break;

        default:
            break;
    } 
}

/* THE BANK SWITCH FUNCTIONS LOOKS INTO THE CORRESPODENCE STORED IN */
/* THE ZBUFFER TO DETERMINE THE OFFSET OF MEMORY ALLOCATIONS */

/* THIS WILL CHECK TO SEE IF THE BOOT ROM HAS BEEN LOADED AND IF SO */
/* MIMMICK THE FUNCTIONALITY OF THE JUMP COROUTINE TO INITIALISE THE START OF THE CART */

// TO-DO: 15/01/25
// BANKSWITCH READ AND WRITE WILL GO HERE



/* INITIALISE THE FUNCTIONALITY PERTAINING TOWARDS THE Z80 AS IT COMMUNICATES */
/* WITH THE CONSOLE */

/* USING RUNTIME OF THE CONSOLE, STORE RELEVANT REGISTERS AND THEIR STATES AND CONDITIONS */
/* SUCH THAT THE APPROPRIATE CONTEXT CAN BE MET DEPENDING OF WHICH OPERANDS ARE USING */
/* SAID REGISTERS */

/* SEE lib68k OPCODE FOR FURTHER READING */

void MD_SAVE_REGISTER_STATE(struct CPU_68K* CPU_68K)
{
    int INDEX;

    /* STORE THE MAIN 15 REGISTERS; DATA AND ADDRESS (MAIN M68000 CPU) */

    for (INDEX = 0; INDEX < 8; INDEX++)
        CPU_68K->REGISTER_BASE[INDEX] = CPU_68K->DATA_REGISTER[INDEX];

    for (INDEX = 0; INDEX < 7; INDEX++)
        CPU_68K->REGISTER_BASE[INDEX] = CPU_68K->ADDRESS_REGISTER[INDEX];

    /* THIS CHECKS TO SEE IF AND WHEN THE STACK POINTER COMES INTO INITIALISATION */
    /* THE STACK POINTER GOVERNS A 32 BIT UNSIGNED VALUE WHICH INCLUDES A 16 BIT SIGNED OPERAND */
    /* AND AN ARBITRARY MANTISSA */

    /* ONCE THE STACK HAS BEEN INITIALISED ON STARTUP (0x2000), A7 WILL */
    /* BEGIN TO LOAD CONTENTS FROM THE HEADER */

    if(CPU_68K->STACK_POINTER && 0x2000)
    {
        CPU_68K->STACK_POINTER = CPU_68K->ADDRESS_REGISTER[7];
        CPU_68K->USER_STACK = CPU_68K->ADDRESS_STACK_POINTER;
    }
    
    else
    {
        CPU_68K->STACK_POINTER = CPU_68K->ADDRESS_STACK_POINTER;
        CPU_68K->USER_STACK = CPU_68K->ADDRESS_REGISTER[7];
    }

    CPU_68K->PC += *(int*)malloc(sizeof(CPU_68K->PC));
    CPU_68K->STATUS_REGISTER += *(U16*)malloc(sizeof(CPU_68K->STATUS_REGISTER));

    memset(CPU_68K, 0x00, sizeof(*CPU_68K));
}

/* SUCH IS THE CASE WITH THE FUNCTION ABOVE */
/* WE WILL NOW DO THIS FOR THE CARTRIDGE */

/* SINVE THE CARTRIDGE NEEDS TO BE ABLE TO COMMUNICATE BETWEEN */
/* THE PC TO DETERMINE THE STATE */

int MD_CART_UPDATE_BANKING(struct CPU_68K* CPU_68K, int BANKS)
{
    int BANKS_UPDATED = 0;
    int INDEX = 0;

    /* DOES THE CORRESPONDING AMOUNT OF BANKS RELATE */
    /* TO THE CURRENTLY AVAILABLE ON THE STACK */

    if((UNK)BANKS >= sizeof(MD_CARTRIDGE->CARTRIDGE_BANKS))
        BANKS += sizeof(MD_CARTRIDGE->CARTRIDGE_BANKS);

    /* EVALUATE THE HIGH AND LOW ADDRESSING MODES */
    /* THIS IS TO DETERMINE DATA TRANSFER BETWEEN THE BUS */

    for (INDEX = 0; INDEX < BANKS; INDEX++)
    {
        U32 ROM_ADDR_START = (0x80000 * (MD_CARTRIDGE->CARTRIDGE_BANKS[INDEX]));
        MD_CARTRIDGE->CARTRIDGE_BANKS[INDEX] = (U32)sizeof(ROM_ADDR_START);

        if(ROM_ADDR_START < MD_CARTRIDGE->ROM_SIZE)
        {
            CPU_68K->LOW_ADDR = ROM_ADDR_START;
            CPU_68K->HIGH_ADDR = ROM_ADDR_START + 0x7FFFF; /* ADD MASK */
            BANKS_UPDATED++;
        }
    }

    return BANKS_UPDATED;
}

/* INITIALISE THE CARTRIDGE COUROUTINE */
/* BY ESTABLISHING THE VARIABLE ROM SIZE, ALLOCATING MEMORY */
/* FOR THE ROM IMAGE AND DETERMINING VARIOUS INSTANCES OF THE BUFFER */

/* WHILE THE FUNCTION IS AN INT, IT WILL BE VOID AS THERE WILL BE NO LOCAL */
/* RETURN TYPE, JUST FOR BASIC CONDITIONS */

int MD_CART_INIT(struct MD_CART* CART, unsigned char* DATA, unsigned long *SIZE)
{
    /* ASSUME TO BEGIN WITH THAT THERE IS NO CURRENT ROM */
    /* BEING LOADED ONTO THE STACK */

    CART->ROM_BASE = DATA;
    CART->ROM_SIZE = SIZE;

    /* CHECK TO DETERMINE IF THE FILE IS TOO BIG */
    /* IN THE LUCKLIHOOD OF AN INCOMPLETE FILE TYPE */

    if(CART->ROM_SIZE == MD_CART_ROM_SIZE)
    {
        return -2;
    }

    /* ALLOCATE INITAL SPACE FOR THE CARTRIDGE */

    CART->ROM_BASE += *(unsigned char*)malloc(sizeof(CART->ROM_SIZE));

    /* STORE THE INITIAL 512KB FOR SSE2 */
    /* THIS IS BY ALIGNING 16 BYTES OF ADDRESSABLE VECTOR TYPES TO THE HEADER */
    
    memcpy(CART->ROM_DATA, CART->ROM_BASE, CART->ROM_SIZE);

    /* AFTER ALLOCATING THE PROVIDED MEMORY TO THE STACK */
    /* BEGIN BY INITIALISAING THE MEMORY MAP OF THE CARTRIDGE */

    MD_CART_MEMORY_MAP();

    free(CART);
    return 0;
}

/* DISCERN THE MEMORY MAP FOR THE CARTRIDGE'S ROM SIZE */
/* THIS IS BY TAKING INTO ACCOUNT SEVERAL FACTORS SUCH AS */
/* SETTING THE ROM MAP, SETTING MAPPER REGISTER BASED ON BANKING TYPE */

/* EACH MAPPER TYPE REPRESENTS AN ACTION TAKEN AT EACH SPECIFIC MEMORY ADDRESS */
/* ON THE HEADER */

/* FROM THERE, COPY THE REGISTER DATA TO THE DESIGNATED MAPPER */

void MD_CART_MEMORY_MAP(void)
{
    UNK INDEX;
    int MAP_MODE = 0;

    switch (MAP_MODE)
    {
        case MAPPER_NORMAL:
            for (INDEX = 0; INDEX < sizeof(MD_CARTRIDGE->CARTRIDGE_BANKS); INDEX++)
                MD_CARTRIDGE->CARTRIDGE_BANKS[INDEX] = MD_CART_BANK_DEFAULT;
            break;

        case MAPPER_READONLY:
            for (INDEX = 0; INDEX < 8; INDEX++)
                MD_CARTRIDGE->CARTRIDGE_BANKS[INDEX] = MD_CART_BANK_RO;

            memcpy(&MD_CARTRIDGE->REGISTER_READ, &CPU->REGISTER_BASE, sizeof(MD_CARTRIDGE->REGISTER_READ));

            break;
    }
}

unsigned int Z80_READ(unsigned int ADDRESS)
{
    unsigned int DATA = 0;

    CPU->INSTRUCTION_CYCLES = malloc(M68K_LOW_BITMASK);

    switch((ADDRESS >> 13) & 3)
    {
        default:
            break;
    }

    return (DATA | (DATA << 8));
}

void Z80_WRITE(unsigned int ADDRESS, unsigned int DATA)
{
    CPU->INSTRUCTION_CYCLES = malloc(M68K_LOW_BITMASK);

    switch((ADDRESS > 13) & 3)
    {
        case 0x7F:
            M68K_WRITE_8(ADDRESS, DATA);
            return;

        default:
            return;
    }
}

unsigned int CTRL_READ_BYTE(unsigned int ADDRESS)
{
    switch ((ADDRESS >> 8) & 0xFF)
    {
        case 0x00 & 0xE0:
            return M68K_READ_8(ADDRESS);
    }

    return M68K_READ_8(ADDRESS);

}

void CTRL_WRITE_BYTE(unsigned int ADDRESS, unsigned int DATA)
{
    DATA &= 8;
    switch ((ADDRESS >> DATA) & 0xFF)
    {
        case 0x00 & 0xE1:
            M68K_READ_8(ADDRESS);
    }
}

unsigned int CTRL_READ_WORD(unsigned int ADDRESS)
{
    switch ((ADDRESS >> 8) & 0xFF)      
    {
        case 0x00 & 0xE0:
            return M68K_READ_8(ADDRESS);
    }

    return M68K_READ_16(ADDRESS);
}

void CTRL_WRITE_WORD(unsigned int ADDRESS, unsigned int DATA)
{
    DATA &= 8;
    switch ((ADDRESS >> DATA) & 0xFF)
    {
        case 0x00 & 0xE1:
            M68K_READ_8(ADDRESS);
    }
}


#endif 
