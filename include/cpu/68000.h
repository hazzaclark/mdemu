/* COPYRIGHT (C) HARRY CLARK 2024 */

/* SEGA MEGA DRIVE EMULATOR */

/* THIS FILE PERTAINS TO THE MAIN FUNCTIONALITY OF THE CPU */
/* USING DOCUMENTATION, THE AMBITION IS TO FASHION THE BASE ARCHITECURE OF THE CONSOLES' */
/* FUNCTIONS WHICH WILL CORRESPOND WITH THE ACTIONS CARRIED OUT BY THE RESPECTIVE CPP FILE */

#ifndef M68K
#define M68K

/* SYSTEM INCLUDES */

#include <68K.h>

#include <assert.h>
#include <malloc.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* NESTED INCLUDES */

#include "common.h"

/*===============================================================================*/
/*							68000 DEBUG											 */
/*===============================================================================*/

/* MISCALLANEOUS TYPES FOR A LINE EXCEPTION HANDLERS */
/* THIS WILL BE USED IN DOCUMENTING CYCLE ACCURATE EXCEPTIONS AT RUNTIME */

#if M68K_LOG_ENABLE
	extern FILE* M68K_LOG_FILEHANDLE
	#define M68K_DO_LOG(A) 			if(M68K_LOG_FILEHANDLE) fprintf A
	#if M68K_LOG_1010_1111_A_LINE
		#define M68K_DO_LOG(A) 		if(M68K_LOG_FILEHANDLE) fprintf A
	#else
		#define M68K_DO_LOG_EMU(A)
	#endif
#else
	#define M68K_DO_LOG(A)
	#define M68K_DO_LOG_EMU(A)

	/* OPTIONS FOR IMMEDIATE ADDRESSING DIRECTIVES WHEN LOOKING */
	/* FOR CPU CALLBACKS */	

	#define M68K_OPT_OFF				0
	#define M68K_OPT_ON					1

#endif

#define MODE_MASK(MODE) (1 << MODE)

#define CARRY_BIT 0
#define OVERFLOW_BIT 1
#define ZERO_BIT 2
#define NEGATIVE_BIT 3
#define EXTENDED_BIT 4

/* AS OF THIS CODE REFACTOR, I AM NOT SURE IF I WILL NEED */
/* THESE STATIC TYPES */

/* I WILL KEEP THEM HERE JUST IN CASE */

#define         CARRY() BIT(REGS[0], CARRY_BIT)
#define         OVERFLOW() BIT(REGS[1], OVERFLOW_BIT)
#define         ZERO() BIT(REGS[2], ZERO_BIT)
#define         NEGATIVE() BIT(REGS[3], NEGATIVE_BIT)
#define         EXTENDED() BIT(REGS[4], EXTENDED_BIT)

#define         ADDRESS_WIDTH_8             0xFF
#define         ADDRESS_WIDTH_16            0xFFFF
#define         ADDRESS_WIDTH_32            0xFFFFFFFF
#define         SYMBOL_WIDTH                ''
#define         ADDRESS_ILLEGAL_ACCESS      (0x8000000 << 0xDFFFFF)

#if defined(USE_CPU)
#define USE_CPU
#else
#define USE_CPU

#define 	READ_BYTE(BASE, ADDR) 			(BASE)[(ADDR)^1]
#define 	READ_WORD(BASE, ADDR) 			(((BASE)[ADDR]<<8) | (BASE)[(ADDR)+1])

#define 	READ_WORD_LONG(BASE, ADDR) 		(((BASE)[(ADDR)+1]<<24) |       \
                                    		((BASE)[(ADDR)]<<16) |  		\
                                    		((BASE)[(ADDR)+3]<<8) |   		\
                                    		(BASE)[(ADDR)+2])

/*===============================================================================*/
/*							68000 MAIN CPU FUNCTIONALIY							 */
/*===============================================================================*/

/* MACROS USED FOR GETTING AND SETTING VALUES BASED ON SPECIFIC REQUIREMENTS */
/* MORE SPECIFICALLY, THIS IS TO AVOID ARRAY INDEXXING ISSUES WHEN IT COMES TO */
/* REGISTERS */

#define 	M68K_GET_DB 			CPU_68K.REGISTER_BASE
#define 	M68K_GET_DR(VALUE) 		CPU_68K->DATA_REGISTER[VALUE]
#define 	M68K_GET_AR(VALUE) 		CPU_68K->ADDRESS_REGISTER[VALUE]
#define		M68K_GET_SP(VALUE)		CPU_68K->STACK_POINTER += ((VALUE))

#define 	M68K_MASK_OUT_ABOVE_8(A) ((A) & 0xFF)
#define 	M68K_MASK_OUT_ABOVE_16(A) ((A) & 0xFFFF)
#define 	M68K_MASK_OUT_ABOVE_32(A) ((A) & 0xFFFFFFF)


/* ARBITARY MACROS TO ALLOW FOR READING AND WRITING DATA */
/* THIS RETURNS A STATIC CAST OF A SPECIFIC DATA TYPE */

extern unsigned int M68K_READ_8(unsigned int ADDRESS);
extern unsigned int M68K_READ_16(unsigned int ADDRESS);
extern unsigned int M68K_READ_32(unsigned int ADDRESS);

extern void M68K_WRITE_8(unsigned int ADDRESS, unsigned int DATA);
extern void M68K_WRITE_16(unsigned int ADDRESS, unsigned int DATA);
extern void M68K_WRITE_32(unsigned int ADDRESS, unsigned int DATA);

extern unsigned int Z80_READ(unsigned int ADDRESS);
extern void Z80_WRITE(unsigned int ADDRESS, unsigned int DATA);

extern unsigned int CTRL_READ_BYTE(unsigned int ADDRESS);
extern unsigned int CTRL_READ_WORD(unsigned int ADDRESS);
extern void CTRL_WRITE_BYTE(unsigned int ADDRESS, unsigned int DATA);
extern void CTRL_WRITE_WORD(unsigned int ADDRESS, unsigned int DATA);

#define     M68K_RETURN_ADDRESS(ADDRESS)                ((*ADDRESS) & 0xFFFFFFFFFF)

#define 		M68K_BANK_CARTRIDGE    (0x000000 << 0x9FFFFF)    			/* $000000 - $9FFFFF */
#define 		M68K_BANK_MD_IO        (0xA00000 << 0xBFFFFF)    			/* $A00000 - $BFFFFF */
#define 		M68K_BANK_VDP          (0xC00000 << 0xDFFFFF)   		 	/* $C00000 - $DFFFFF */
#define 		M68K_BANK_RAM          (0xE00000 << 0xFFFFFF)    			/* $E00000 - $FFFFFF */
#define 		M68K_BANK_TMSS_ROM     (0x000000 << 0x3FFFFF)    			/* $000000 - $3FFFFF */
#define 		M68K_BANK_PICO_IO      (0x800000 << 0x9FFFFF)    			/* $800000 - $9FFFFF (TODO) */ 
#define 		M68K_BANK_UNUSED       0xFF

#define			M68K_MIN_TMSS_SIZE		4096
#define			M68K_MAX_TMSS_SIZE		524288

#define			M68K_RAM_BYTE			64 * 1024
#define			M68K_RAM_WORD			64 * 1024 >> 1
#define			M68K_RAM_LONG			64 * 1024 >> 2

#define			M68K_LOW_BITMASK		8*7
#define			M68K_MID_BITMASK		16*7
#define			M68K_HIGH_BITMASK		24*7
#define 		M68K_MAX_BITMASK		32*7

#define			M68K_SAVE_REGISTER(TYPE, SRC, NAME, VALUE)	\

#ifdef M68K_CYCLE_CLOCK
#define M68K_CYCLE_CLOCK_SHIFT(VALUE) 		CPU_68K->INSTRUCTION_CYCLES += ((VALUE) * CPU_68K.CYCLE_RATE) >> M68K_CYCLE_CLOCK_SHIFT
#else
#define M68K_CYCLE_CLOCK_SHIFT(VALUE) 		CPU->INSTRUCTION_CYCLES += ((VALUE))
#endif

#define		M68K_MAX_INSTR_LENGTH			0x10000

#define		M68K_ADD_CYCLES(VALUE)			CPU->INSTRUCTION_CYCLES += (VALUE)
#define		M68K_USE_CYCLES(VALUE)			CPU->INSTRUCTION_CYCLES -= (VALUE)
#define		M68K_SET_CYCLES(VALUE)			CPU->INSTRUCTION_CYCLES = VALUE

#define 	EXCEPTION_RESET                    0
#define 	EXCEPTION_BUS_ERROR                2 
#define 	EXCEPTION_ADDRESS_ERROR            3
#define 	EXCEPTION_ILLEGAL_INSTRUCTION      4
#define 	EXCEPTION_ZERO_DIVIDE              5
#define 	EXCEPTION_CHK                      6
#define 	EXCEPTION_TRAPV                    7
#define 	EXCEPTION_PRIVILEGE_VIOLATION      8
#define 	EXCEPTION_TRACE                    9
#define 	EXCEPTION_1010                    10
#define 	EXCEPTION_1111                    11
#define 	EXCEPTION_FORMAT_ERROR            14
#define 	EXCEPTION_UNINITIALIZED_INTERRUPT 15
#define 	EXCEPTION_SPURIOUS_INTERRUPT      24
#define 	EXCEPTION_INTERRUPT_AUTOVECTOR    24
#define 	EXCEPTION_TRAP_BASE               32

typedef struct CPU_68K_MEMORY
{
    unsigned(*MEMORY_BASE);
    U8* MEMORY_READ_8;
    U16*  MEMORY_READ_16;
    U8* MEMORY_WRITE_8;
    U16* MEMORY_WRITE_16;

} CPU_68K_MEMORY;

typedef struct CPU_68K
{
    

    /* PUTTING THE Z80 MEMORY BANK FUNCTIONALITY */
    /* IN HERE FOR NOW UNTIL MODULARISATION WOULD BETTER SUIT */

    union Z80_MEM
    {
        unsigned(*READ)(unsigned ADDRESS);
        unsigned(*WRITE)(unsigned ADDRESS);

        unsigned int CYCLES;
		U8 ZRAM[0x2000];

    } Z80_MEM[256];

	CPU_68K_MEMORY MEMORY_MAP[256];

	/* VERY UNORGANISED TMSS MAPPER */
	/* TO:DO - FIX THE ORGANISATION */

	union TMSS
	{
		void(*ROM)(void);
		U32 READ_BANK;
		U8 ROM_MAPPER;
		bool IS_MAPPED;
		void(*RESET)(void);

	} TMSS;


	U16* STATUS_REGISTER;
	U32* INDEX_REGISTER;
    U32* REGISTER_BASE[16];
	U32* DATA_REGISTER[8];
	U32* ADDRESS_REGISTER[8];
    U32* PREVIOUS_PC;
    U32* STACK_POINTER;
	U32* INTERRUPT_SP;
	U32* MASTER_SP;
	U32* USER_STACK;
	U32* ADDRESS_STACK_POINTER;
    U32* INSTRUCTION_REGISTER;
	U32* SOURCE_FUNCTION_COUNTER;
	U32* DEST_FUNCTION_COUNTER;
	U32* VBR;
	U32* FPR[8];
	U32* FPIAR;
	U32* FPCR;
	U32* FPSR;
	U32* CACHE_CONTROL;
	U32* CACHE_ADDRESS;

	U32 TMSS_BASE[4];

    char* INSTRUCTION_MODE;
    char* TRACE_FLAG;

    unsigned int* PREVIOUS_DATA;
    unsigned int* PREVIOUS_ADDRESS;
    unsigned int* ADDRESS_RT_CHECK;
    unsigned char* ERROR_ADDRESS;
    unsigned char* ERROR_WRITE_MODE;
    unsigned char* ERROR_PC;
    UNK* ERROR_JUMP;

    S32(*INTERRUPT_CALLBACK)(unsigned INTERRUPT);
    S32(*RESET_INTERRUPT)(void);
    S32(*CPU_FUNC_CALLBACK)(unsigned FUNCTION);
	unsigned int* INT_LEVEL;

	unsigned* CPU_STOPPED;

	unsigned S_FLAG;
	unsigned* X_FLAG;
	unsigned* N_FLAG;
	unsigned* V_FLAG;
	unsigned* Z_FLAG;
	unsigned* C_FLAG;
	unsigned M_FLAG;

	unsigned* T0_FLAG;
	unsigned* T1_FLAG;


} CPU_68K;

typedef enum CPU_68K_REGS
{
	M68K_REG_TYPE,
    M68K_D0 = 0,    
    M68K_D1 = 1,
    M68K_D2 = 2,
    M68K_D3 = 3,
    M68K_D4 = 4,
    M68K_D5 = 5,
    M68K_D6 = 6,
    M68K_D7 = 7,
    M68K_A0 = 8,    
    M68K_A1 = 9,
    M68K_A2 = 10,
    M68K_A3 = 11,
    M68K_A4 = 12,
    M68K_A5 = 13,
    M68K_A6 = 14,
    M68K_A7 = 15,
    M68K_PC,    
    M68K_SR,    
    M68K_SP,    
    M68K_USP,   
    M68K_ISP,
    M68K_IR, 
	M68K_SFC,
	M68K_VBR,
	M68K_DFC,
	M68K_CACR,
	M68K_CAAR,

} CPU_68K_REGS;

typedef enum CPU_68K_FLAGS 
{
    FLAG_S,
    FLAG_X,
    FLAG_Z,
    FLAG_N,
    FLAG_C,
    FLAG_V,
	FLAG_T0,
    FLAG_T1,
	FLAG_M

} CPU_68K_FLAGS;

#define 		M68K_REG_DA				CPU->DATA_REGISTER
#define			M68K_REG_D				CPU->DATA_REGISTER
#define			M68K_REG_A				(CPU->DATA_REGISTER + 8)
#define			M68K_REG_SR				CPU->STATUS_REGISTER
#define			M68K_REG_PPC			CPU->PREVIOUS_PC
#define			M68K_REG_PC				CPU->PC
#define			M68K_REG_SP				CPU->STACK_POINTER
#define			M68K_REG_USP			CPU->USER_STACK[0]
#define			M68K_REG_ISP			CPU->INTERRUPT_SP[4]
#define			M68K_REG_MSP			CPU->MASTER_SP[6]
#define			M68K_REG_SP_FULL		CPU->REGISTER_BASE[15]
#define			M68K_REG_VBR			CPU->VBR
#define			M68K_REG_SFC			CPU->SOURCE_FUNCTION_COUNTER
#define			M68K_REG_DFC			CPU->DEST_FUNCTION_COUNTER
#define			M68K_REG_CACR			CPU->CACHE_CONTROL
#define			M68K_REG_CAAR			CPU->CACHE_ADDRESS
#define			M68K_REG_IR				CPU->INDEX_REGISTER
#define 		M68K_REG_FPR			CPU->FPR
#define			M68K_REG_FPCR			CPU->FPCR
#define			M68K_REG_FPSR			CPU->FPSR
#define			M68K_REG_FPIAR			CPU->FPIAR

#define  		M68K_FLAG_T0			CPU->T0_FLAG
#define			M68K_FLAG_T1			CPU->T1_FLAG
#define			M68K_FLAG_S				CPU->S_FLAG
#define			M68K_FLAG_M				CPU->M_FLAG
#define			M68K_FLAG_X				CPU->X_FLAG
#define			M68K_FLAG_N				CPU->N_FLAG
#define			M68K_FLAG_Z				CPU->Z_FLAG
#define			M68K_FLAG_V				CPU->V_FLAG
#define			M68K_FLAG_C				CPU->C_FLAG
#define			M68K_FLAG_INT_LVL		CPU->INT_LEVEL
#define			M68K_CPU_STOPPED		CPU->CPU_STOPPED

#define			M68K_CYC_EXCE			CPU->CYCLE_EXCEPTION
#define 		M68K_CYCLE				CPU->INSTRUCTION_CYCLES[16]

#define M68K_SAVE_INSTR(IDENTIFIER, VALUE) 					(*((char*)(IDENTIFIER)) = (char)((VALUE)))
#define	M68K_INT_LEVEL										CPU->INT_LEVEL
#define	M68K_CYC_INSTRUCTION								CPU->INSTRUCTION_CYCLES

/*===============================================================================*/
/*							68000 OPCODE FUNCTIONALIY							 */
/*===============================================================================*/

#ifdef 			USE_OPCODE_DEFS
#define 		USE_OPCODE_DEFS
#else
#define			OPERAND_NONE		                                                        0 << 0															 
#define			OPERAND_DATA_REGISTER														1 << 0
#define			OPERAND_ADDRESS_REGISTER													1 << 1
#define			OPERAND_ADDRESS_REGISTER_IND												1 << 2
#define			OPERAND_ADDRESS_REGISTER_IND_POSTINCREMENT									1 << 3
#define			OPERAND_ADDRESS_REGISTER_IND_PREDECREMENT									1 << 4
#define			OPERAND_ADDRESS_REGISTER_IND_W_DISP											1 << 5
#define			OPERAND_ADDRESS_REGISTER_IND_W_DISP_INDEX_REG								1 << 6
#define			OPERAND_ADDRESS_BASE														1 << 7
#define			OPERAND_ADDRESS_ABSOLUTE													1 << 8
#define			OPERAND_LITERAL																1 << 9
#define			OPERAND_PC_W_DISP															1 << 10
#define			OPERAND_PC_W_DISP_INDEX_REG													1 << 11
#define			OPERAND_STATUS_REGISTER_BASE												1 << 12
#define			OPERAND_CONDITION_CODE_REG_BASE												1 << 13
#define			OPERAND_USER_STACK_POINTER_REG_BASE											1 << 14
#define			OPERAND_REGISTER_LIST	

#define			OPCODE_MAX_MASK								0x10000

/* THESE MACROS REFER TO THE VARIOUS WAYS BY WHICH OPCODES ENCOMPASSES */
/* MULTIPLE OPERAND MODES OR JUST ONE MODE */

/* BOTH GOVERN THE SAME PROPERTIES OF INCLUDING DIFFERENT ADDRESSES EVOKING THEIR */
/* LENGTH HOWEVER, THEY ONLY ENCOMPASS ONE INSTRUCTION */

/* SUCH AN EXAMPLE WOULD BE COMPARISON OPERANDS WHICH ENCOMPASSES TWO OPERAND MODES */

#define			OPCODE_ADDRESS_OFFSET(VALUE)			((VALUE) << 16)
#define			OPCODE_ADRESSS_OFFSETS(VALUE, OFFSET)	(((VALUE) << 16) + OFFSET)

#endif

typedef enum CONDITION
{
    CONDITION_TRUE,
	CONDITION_FALSE,
	CONDITION_HIGHER,
	CONDITION_LOWER_OR_SAME,
	CONDITION_CARRY_CLEAR,
	CONDITION_CARRY_SET,
	CONDITION_NOT_EQUAL,
	CONDITION_EQUAL,
	CONDITION_OVERFLOW_CLEAR,
	CONDITION_OVERFLOW_SET,
	CONDITION_PLUS,
	CONDITION_MINUS,
	CONDITION_GREATER_OR_EQUAL,
	CONDITION_LESS_THAN,
	CONDITION_GREATER_THAN,
	CONDITION_LESS_OR_EQUAL,

} CONDITION;

typedef enum OPCODE_TYPE
{
    OPCODE_ORI_TO_CCR,
	OPCODE_ORI_TO_SR,
	OPCODE_ORI,
	OPCODE_ANDI_TO_CCR,
	OPCODE_ANDI_TO_SR,
	OPCODE_ANDI,
	OPCODE_SUBI,
	OPCODE_ADDI,
	OPCODE_EORI_TO_CCR,
	OPCODE_EORI_TO_SR,
	OPCODE_EORI,
	OPCODE_CMPI,
	OPCODE_BTST_STATIC,
	OPCODE_BCHG_STATIC,
	OPCODE_BCLR_STATIC,
	OPCODE_BSET_STATIC,
	OPCODE_BTST_DYNAMIC,
	OPCODE_BCHG_DYNAMIC,
	OPCODE_BCLR_DYNAMIC,
	OPCODE_BSET_DYNAMIC,
	OPCODE_MOVEP_TO_REG,
	OPCODE_MOVEP_FROM_REG,
	OPCODE_MOVEA,
	OPCODE_MOVE,
	OPCODE_MOVE_FROM_SR,
	OPCODE_MOVE_TO_CCR,
	OPCODE_MOVE_TO_SR,
	OPCODE_NEGX,
	OPCODE_CLR,
	OPCODE_NEG,
	OPCODE_NOT,
	OPCODE_EXT,
	OPCODE_NBCD,
	OPCODE_SWAP,
	OPCODE_PEA,
	OPCODE_ILLEGAL,
	OPCODE_TAS,
	OPCODE_TST,
	OPCODE_TRAP,
	OPCODE_LINK,
	OPCODE_UNLK,
	OPCODE_MOVE_TO_USP,
	OPCODE_MOVE_FROM_USP,
	OPCODE_RESET,
	OPCODE_NOP,
	OPCODE_STOP,
	OPCODE_RTE,
	OPCODE_RTS,
	OPCODE_TRAPV,
	OPCODE_RTR,
	OPCODE_JSR,
	OPCODE_JMP,
	OPCODE_MOVEM_TO_REGS,
	OPCODE_MOVEM_FROM_REGS,
	OPCODE_LEA,
	OPCODE_CHK,
	OPCODE_ADDQ,
	OPCODE_SUBQ,
	OPCODE_Scc,
	OPCODE_DBcc,
	OPCODE_BRA,
	OPCODE_BSR,
	OPCODE_BCC,
	OPCODE_MOVEQ,
	OPCODE_DIVU,
	OPCODE_DIVS,
	OPCODE_SBCD_DATA_REGS,
	OPCODE_SBCD_ADDRESS_REGS,
	OPCODE_OR_TO_REG,
	OPCODE_OR_FROM_REG,
	OPCODE_SUB_TO_REG,
	OPCODE_SUB_FROM_REG,
	OPCODE_SUBX_DATA_REGS,
	OPCODE_SUBX_ADDRESS_REGS,
	OPCODE_SUBA,
	OPCODE_EOR,
	OPCODE_CMPM,
	OPCODE_CMP,
	OPCODE_CMPA,
	OPCODE_MULU,
	OPCODE_MULS,
	OPCODE_ABCD,
	OPCODE_EXG,
	OPCODE_AND_TO_REG,
	OPCODE_AND_FROM_REG,
	OPCODE_ADD_TO_REG,
	OPCODE_ADD_FROM_REG,
	OPCODE_ADDX_DATA_REGS,
	OPCODE_ADDX_ADDRESS_REGS,
	OPCODE_ADDA,
	OPCODE_ASL_STATIC,
	OPCODE_ASR_STATIC,
	OPCODE_LSL_STATIC,
	OPCODE_LSR_STATIC,
	OPCODE_ROXL_STATIC,
	OPCODE_ROXR_STATIC,
	OPCODE_ROL_STATIC,
	OPCODE_ROR_STATIC,
	OPCODE_ASL_DYNAMIC,
	OPCODE_ASR_DYNAMIC,
	OPCODE_LSL_DYNAMIC,
	OPCODE_LSR_DYNAMIC,
	OPCODE_ROXL_DYNAMIC,
	OPCODE_ROXR_DYNAMIC,
	OPCODE_ROL_DYNAMIC,
	OPCODE_ROR_DYNAMIC,
	OPCODE_ASL_SINGLE,
	OPCODE_ASR_SINGLE,
	OPCODE_LSL_SINGLE,
	OPCODE_LSR_SINGLE,
	OPCODE_ROXL_SINGLE,
	OPCODE_ROXR_SINGLE,
	OPCODE_ROL_SINGLE,
	OPCODE_ROR_SINGLE,

} OPCODE_TYPE;

typedef enum OPCODE_MASK_MODE
{
	OPCODE_MASK_ILLEGAL = 0xFF00,
	OPCODE_MASK_LOG_BIT = 0xF1F8,
	OPCODE_MASK_LOG_OR = 0xF1FF,
	
} OPCODE_MASK_MODE;

/*===============================================================================*/
/*							68000 ADDRESSING MODES								 */
/*===============================================================================*/

/* THE FOLLOWING DIRECTIVE SERVE TO PROVIDE FUNCTIONALITY PERTAINING TOWARDS */
/* THE STRING LITERAL EVALUATION OF THESE ADDRESSING MODE TYPES */

/* THE DIFFERENTIATION BETWEEN THIS AND THE ENUM IS THAT THE ENUM WILL BE */
/* THERE TO PROVIDE LOOSE VALUES PERTAINING TOWARDS THE CYCLE ACCURATE ADDRESSABLE MODE */

#ifndef HAS_EA_ACCESS
#define HAS_EA_ACCESS

#define 	HAS_NO_EA_MODE(VALUE) 										(strcmp(VALUE, " 			") == 0)
#define		HAS_EA_ADDRESS_INDRECT(VALUE)								(strcmp(VALUE)[0] == 'AI')
#define		HAS_EA_PRE_INCREMENT(VALUE)									(strcmp(VALUE)[0] == '+')
#define		HAS_EA_POST_DECREMENT(VALUE)								(strcmp(VALUE)[0] == '-')
#define		HAS_EA_DISP(VALUE)											(strcmp(VALUE)[0] == 'D')
#define		HAS_EA_INCREMENT(VALUE)										(strcmp(VALUE)[0] == 'X')
#define		HAS_EA_WORD(VALUE)											(strcmp(VALUE)[0] == 'W')
#define		HAS_EA_LONG(VALUE)											(strcmp(VALUE)[0] == 'L')
#define		HAS_EA_PROGRAM_COUNTER_DISP(VALUE)							(strcmp(VALUE)[0] == 'PCDI')
#define		HAS_EA_PROGRAM_COUNTER_INDEX(VALUE)							(strcmp(VALUE)[0] == 'PCIX')
#define		HAS_EA_INDEX(VALUE)											(strcmp(VALUE)[0] == 'I')

typedef enum EA_MODES
{ 
	EA_NONE,
	EA_AW,
	EA_AL,
	EA_AI,
	EA_PI,
	EA_PI7,
	EA_PD,
	EA_PD7,
	EA_PCDI,
	EA_PCIX,
	EA_IX,
	EA_I

} EA_MODES;

#endif

extern CPU_68K_REGS CPU_REGS;

void INITIALISE_68K_CYCLES();
void M68K_INIT(void);
void M68K_MEM_INIT(void);
int M68K_EXEC(struct CPU_68K* CPU_68K, int CYCLES);
void M68K_INIT_OPCODE(void);
void M68K_RUN(void);
void M68K_SET_FUNC_CALLBACK(int* CALLBACK);

U16(*M68K_FETCH_INSTR(struct CPU_68K* CPU_68K));
int M68K_SET_INT_CALLBACK(int* LEVEL);
int M68K_CALLBACK_INT(int* CALLBACK, int* LEVEL);
void M68K_FUNC_CALLBACK(int* CALLBACK, int* LEVEL);

void M68K_SET_MOVE_IRQ_INT();
void M68K_CHECK_IRQ(void);
void M68K_PULSE_RESET(void);

void M68K_BUILD_OPCODE_TABLE(void);
void M68K_OPCODE_HANDLE();
void M68K_STATE_REGISTER();

void M68K_JUMP(unsigned NEW_PC);
void M68K_JUMP_VECTOR(unsigned VECTOR);
void M68K_SET_SR_IRQ(unsigned VALUE);
	
U32* CPU_ACCESS_REGISTERS(struct CPU_68K* CPU_68K, int REGISTER);
void CPU_SET_REGISTERS(struct CPU_68K* CPU_68K, int REGISTER, unsigned VALUE);

typedef CPU_68K CPU_68K_CORE;
typedef OPCODE* GENERATE_OPCODE(const OPCODE* OPCODE, UNK* MAP);

void UPDATE_TMSS_MAPPING(void);
int UPDATE_SYS_BANKING(struct CPU_68K* CPU_68K, int BANKS);

unsigned int(*LOAD_TMSS_ROM(void));
void CLEAR_TMSS_ROM();
U8 TMSS_ROM(void);
bool IS_TMSS_ENABLED();

U8 M68K_READ_BUS_BYTE(U32*  ADDRESS);
U8 M68K_READ_RAM_BYTE(U32* ADDRESS);

/*===============================================================================*/
/*							68000 EXCEPTION HANDLERS						  	 */
/*===============================================================================*/

/* THESE FUNCTIONS WILL ENCOMPASS THE EXPANSION BY WHICH THE 68000 HANDLES MEMORY EXCEPTIONS AND HANDLERS */
/* IN CROSS COMMUNICATION WITH THE PC */

/* THE IDEA IS TO NOTE DOWN THE SPECIFICS OF WHAT IS GOING ON AT A LOWER LEVEL AT EACH INSTRUCTION */
/* SUCH THAT WE ARE ABLE TO USE THESE FUNCTIONS IN A SINGLE-FORM MANNER LATER ON */

#define 		M68K_LOG_ALINE			M68K_OPT_OFF

#endif
#endif
