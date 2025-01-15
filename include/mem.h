/* COPYRIGHT (C) HARRY CLARK 2025 */

/* SEGA MEGA DRIVE EMULATOR */

/* THIS FILE PERTAINS TOWARDS THE FUNCTIONALITY ENCOMPASSING */
/* THE MEMORY UTILITIES BETWEEN COMPONENTS AND THE CONNECTIONS BETWEEN THEM */

/* THIS FILE PROVIDES THE BASIC DEFINITIONS FOR M68K AND Z80 BUS COMMUNICATIONS */
/* AND RELEVANT PARSING AND WHATEVER THE CASE MAY BE AS PER DOCUMENTATION */

#ifndef MEMORY_H
#define MEMORY_H

typedef struct ZBANK_MEM
{
    unsigned(*READ)(unsigned ADDRESS);
    void(*WRITE)(unsigned ADDRESS, unsigned DATA);

} ZBANK_MEM;

#if defined(USE_ZBANK_MEM)
    #define USE_ZBANK_MEM

#else
    #define USE_ZBANK_MEM

    extern unsigned ZBANK_READ_UNUSED(unsigned ADDRESS);
    extern void ZBANK_WRITE_UNUSED(unsigned ADDRESS, unsigned DATA);

    extern unsigned ZBANK_LOOKUP_READ(unsigned ADDRESS);
    extern void ZBANK_LOOKUP_WRITE(unsigned ADDRESS, unsigned DATA);

    extern unsigned ZBANK_READ_CTRL(unsigned ADDRESS);
    extern void ZBANK_WRITE_CTRL(unsigned ADDRESS, unsigned DATA);

    extern unsigned ZBANK_READ_VDP(unsigned ADDRESS);
    extern void ZBANK_WRITE_VDP(unsigned ADDRESS, unsigned DATA);

#endif



extern ZBANK_MEM ZBANK_MEM_MAP[256];

#endif