/* Copyright(C) 2023 Harry Clark * /

/* SEGA Mega Drive Emulator */

/* THIS FILE PERTAINS TO THE FUNCTIONALITY OF THE INSTRUCTION SET IN ACCORDANCE */
/* WITH THE INSTRUCTIONS PERTAINING TO THE ARCHITECTURE OF THE CPU */

#ifndef INSTRUCTION_SET
#define INSTRUCTION_SET

/* NESTED INCLUDES */

#include "68000.h"
#include "common.h"

/* SYSTEM INCLUDES */

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#if defined(USE_M68K_MEM)
#define USE_M68K_MEM
#else
#define USE_M68K_MEM

#define BIT (VALUE, INDEX)            ((VALUE) >> (INDEX))
#define BITS (VALUE, INDEX, RESULT)   ((VALUE) >> (INDEX)) & ((1 << (RESULT)) - 1)) 
#define BYTE_LO 0
#define BYTE_HI 0

extern void MEM_INIT();
extern U32 LOAD_BIN();

#endif

#if defined(USE_ISA)
#define USE_ISA
#else
#define USE_ISA

/* CREATES A STATIC OBJECT FROM WHICH A DESIGNATED OPCODE */
/* CAN BE CONSTRUCTED FROM */

/* USING A STRUCTURE METHOD TO CONCATENATE THE STRUCTURE */
/* WITH THE OPCODE VALUE PROVIDED */

/* THE FOLLOWING ALLOWS FOR SEAMLESS INTEGRATION OF OPCODE */
/* WRITING AND ANY OTHER STATIC OBJECT DECLARATIONS */

#define INSTRUCTION_FUNCTION(VALUE) \
static INSTRUCTION* TYPE ## VALUE(U16 OPCODE);

#define INSTRUCTION_MAKE(VALUE) \
static INSTRUCTION* TYPE ## VALUE(char* NAME);

#define INSTRUCTION_GENERATE(VALUE) \
static INSTRUCTION* TYPE ## VALUE(U16 OPCODE);

#define INSTRUCTION_EXEC(VALUE) \
static INSTRUCTION* TYPE ## VALUE(U8 INSTR, CPU* CPU);

#define INSTRUCTION_NON_IMPL(VALUE) \
static INSTRUCTION* TYPE ## VALUE(CPU* CPU);

typedef struct INSTRUCTION
{
	static char* NAME;
	static U16* OPCODE;
	static U8* CYCLES;
};

typedef struct CONDITION;

#define CONDITION_FUNCTION(VALUE) \
static CONDITION* FUNCTION ## VALUE(bool CONDITION_MET);

#define CONDITION_GET(VALUE) \
static CONDITION* GET_SET ## VALUE(U32 PATTERN);

#define CONDITION_MNEMONICS(VALUE) \
static CONDITION* MNEMOMIC ## VALUE(char* TYPE);

#endif

#endif
