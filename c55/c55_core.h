/*
 * gDSPsim - GNU Digital Signal Processor Simulator
 *
 * Copyright (C) 2001-2002, Kerry Keal, kerry@industrialmusic.com
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2, as published by the Free Software Foundation.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include <glib.h>
#include <stdio.h>

#ifndef __C55_CORE_H__
#define __C55_CORE_H__
// We want to deal with the proper bit length words.
typedef guint16 Word;
typedef guint32 DWord;
typedef gint16 SWord;
typedef guint32 WordP; //type of variable used for addressing Words.
#define MAX_WORD 0xffff

#define MAX_OP_LEN 8
#define BITS_PER_PROGRAM_ACCESS 8 // minimum number of bits addressable by program mem
#define BITS_PER_ADDRESS 32
#define MAX_OP_STR_LEN 80 // max str length of a decoded opcode

//typedef struct _Word Word;
struct _Word
{
  unsigned char byte0;
  unsigned char byte1;
};

// 40 bit General Purpose Register
struct _byte_GP_Reg
{
  unsigned char byte0;
  unsigned char byte1;
  unsigned char byte2;
  unsigned char byte3;
  unsigned char byte4;
};

struct _word_GP_Reg
{
  Word word0;
  Word word1;
};

union _GP_Reg
{
  struct _byte_GP_Reg bgp;
  struct _word_GP_Reg wgp;
  DWord dword;
  WordP worda;
};

typedef union _GP_Reg GP_Reg;

struct _word_opcode
{
  Word word0;
  Word word1;
  Word word2;
  Word word3;
};


union _Opcode
{
  unsigned char bop[MAX_OP_LEN];
  Word wop[MAX_OP_LEN/2];
};

typedef union _Opcode Opcode;

struct _GP_Words
{
  Word low;
  Word high;
  Word ext;
};

struct _GP32
{
  guint32 low;
  guint32 high;
};

struct _GPi32
{
  gint32 low;
  gint32 high;
};

union _GP_Reg_Union
{
  GP_Reg gp_reg;
  guint64 guint64;
  gint64 gint64;
  struct _GP_Words words;
  struct _GP32 gu32;
  struct _GPi32 gi32;
  WordP address;
};

struct _MMR
{
  Word IER0;  // 0 - Interrupt Enable Register
  Word IFR0;  // 1 - Interrupt Flag Register
  Word ST0_55; // 2 - Status Register 0
  Word ST1_55; // 3 - Status Register 1
  Word ST3_55; // 4 - Status Register 3
  Word reserved1; // 5
  Word ST0; // 6 - Status Register 0 
  Word ST1; // 7 - Status Register 1 



  GP_Reg AC0; //  - Accumulator A Low
              //  - Accumulator A High
              //  - Accumulator A Guard Bits
  char left_over_guard_bits_AC0; // fitting 40 bit A into a 48 bit slot
  GP_Reg AC1; //  - Accumulator A Low
              //  - Accumulator A High
              //  - Accumulator A Guard Bits
  char left_over_guard_bits_AC1; // fitting 40 bit B into a 48 bit slot
  Word _T3; // duplicate
  Word TRN0;
  Word ar0;
  Word ar1;
  Word ar2;
  Word ar3;
  Word ar4;
  Word ar5;
  Word ar6;
  Word ar7;
  Word _SP;  // duplicate
  Word BK03; // 19 - Circular Buffer Size Register for ar0-ar3
  Word BRC0; // 1A - Block Repeat Counter 0
  Word _RSA0L; // 1B - Block Repeat Start Address 0 Low
  Word _REA0L; // 1C - Block Repeat End Address 0 Low
  Word PMST; // 1D - Processor Mode Status Register (C55 dont use)
  Word XPC; // 1E - Program counter extension;
  Word reserved2;

  Word T0;
  Word T1;
  Word T2;
  Word T3;


  GP_Reg AC2; //  - Accumulator A Low
              //  - Accumulator A High
              //  - Accumulator A Guard Bits
  char left_over_guard_bits_AC2; // fitting 40 bit B into a 48 bit slot
  Word CDP;

  GP_Reg AC3; //  - Accumulator A Low
              //  - Accumulator A High
              //  - Accumulator A Guard Bits
  char left_over_guard_bits_AC3; // fitting 40 bit B into a 48 bit slot

  Word DPH;
  Word reserved3;

  Word DP;
  Word PDP;

  Word BK47;
  Word BKC;
  Word BSA01;
  Word BSA23;
  Word BSA45;
  Word BSA67;
  Word BSAC;
  
  Word reserved4;

  Word TRN1;
  Word BRC1;
  Word BRS1;
  Word CSR;
  
  WordP RSA0;
  // Word RSA0H;
  // Word RSA0L;

  WordP REA0;
  // Word REA0H;
  // Word REA0L;

  WordP RSA1;
  //Word RSA1H;
  //Word RSA1L;

  WordP REA1;
  // Word REA1H;
  // Word REA1L;

  Word RPTC;

  Word IER1;
  Word IFR1;
  
  Word DBIER0;
  Word DBIER1;

  Word IVPD;
  Word IVPH;
  Word ST2_55;
  // 15 - ARMS
  // 12 - DBGM
  // 11 - EALLOW
  // 10 - RDM
  // 08 - CDPLC
  // 07-00 - AR?LC

  Word SSP;
  Word SP;
  Word SPH;
  Word CDPH;


};

extern struct _MMR *MMR;
extern struct _Registers *Reg;

#define ACOV2(Reg)((((Reg)->ST0_55)&0x8000)>>15)
#define ACOV3(Reg)((((Reg)->ST0_55)&0x4000)>>14)
#define TC1(Reg)((((Reg)->ST0_55)&0x2000)>>13)
#define TC2(Reg)((((Reg)->ST0_55)&0x1000)>>12)
#define TCx(Reg,x)( (x) ? TC2((Reg)) : TC1((Reg)) )
#define CARRY(Reg)((((Reg)->ST0_55)&0x800)>>11)
#define ACOV0(Reg)((((Reg)->ST0_55)&0x400)>>10)
#define ACOV1(Reg)((((Reg)->ST0_55)&0x200)>>9)

// Convenience macros to read status bits
#define BRAF(Reg)((((Reg)->ST1_55)&0x8000)>>15)
#define CPL(Reg)((((Reg)->ST1_55)&0x4000)>>14)
#define XF(Reg)((((Reg)->ST1_55)&0x2000)>>13)
#define HM(Reg)((((Reg)->ST1_55)&0x1000)>>12)
#define INTM(Reg)((((Reg)->ST1_55)&0x800)>>11)
#define M40(Reg)((((Reg)->ST1_55)&0x400)>>10)
#define SATD(Reg)((((Reg)->ST1_55)&0x200)>>9)
#define SXMD(Reg)((((Reg)->ST1_55)&0x100)>>8)
#define C16(Reg)((((Reg)->ST1_55)&0x80)>>7)
#define FRCT(Reg)((((Reg)->ST1_55)&0x40)>>6)
#define C54CM(Reg)((((Reg)->ST1_55)&0x20)>>5)

#define ARMS(Reg)((((Reg)->ST2_55)&0x8000)>>15)
#define DBGM(Reg)((((Reg)->ST2_55)&0x1000)>>12)
#define EALLOW(Reg)((((Reg)->ST2_55)&0x800)>>11)
#define RDM(Reg)((((Reg)->ST2_55)&0x400)>>10)
#define CDPLC(Reg)((((Reg)->ST2_55)&0x100)>>8)
#define ARLC(Reg,arf)((((Reg)->ST2_55)&(1<<(arf)))>>(arf))

#define SATA(Reg)((((Reg)->ST3_55)&0x20)>>5)
#define SMUL(Reg)((((Reg)->ST3_55)&0x2)>>1)

// Convenience macros to set status bits
#define set_ACOV2(Reg,data)((Reg)->ST0_55=((Reg)->ST0_55 & 0x7fff)|((data) & 0x1)<<15)
#define set_ACOV3(Reg,data)((Reg)->ST0_55=((Reg)->ST0_55 & 0xbfff)|((data) & 0x1)<<14)
#define set_TC1(Reg,data)((Reg)->ST0_55=((Reg)->ST0_55 & 0xdfff)|((data) & 0x1)<<13)
#define set_TC2(Reg,data)((Reg)->ST0_55=((Reg)->ST0_55 & 0xefff)|((data) & 0x1)<<12)
// if x=0, set TC1 otherwise set TC2
#define set_TCx(Reg,x,data)((Reg)->ST0_55=(x)?((Reg)->ST0_55 & 0xefff)|((data) & 0x1)<<12 : ((Reg)->ST0_55 & 0xdfff)|((data) & 0x1)<<13)
#define set_CARRY(Reg,data)((Reg)->ST0_55=((Reg)->ST0_55 & 0xf7ff)|((data) & 0x1)<<11)
#define set_ACOV0(Reg,data)((Reg)->ST0_55=((Reg)->ST0_55 & 0xfbff)|((data) & 0x1)<<10)
#define set_ACOV1(Reg,data)((Reg)->ST0_55=((Reg)->ST0_55 & 0xfdff)|((data) & 0x1)<<9)

#define set_BRAF(Reg,data)((Reg)->ST1_55=((Reg)->ST1_55 & 0x7fff)|((data) & 0x1)<<15)
#define set_M40(Reg,data)((Reg)->ST1_55=((Reg)->ST1_55 & 0xfbff)|((data) & 0x1)<<10)

#define set_ACOVx(Reg,data)({ if ((data)==0) set_ACOV0((Reg),1); else if ((data)==1) set_ACOV1((Reg),1); else if ((data)==2) set_ACOV2((Reg),1); else if ((data)==3) set_ACOV3((Reg),1); })

struct _Registers
{
  WordP PC;  // Program Counter
  Word XPC;  // Extend bits for Program Counter
  //Word PAB; // Program Address Bus. Filled with Prefetch
  //Word PB;  // Program Data. Filled during Fetch
  //Word IR;  // Instruction Register. Filled with Decode
  WordP DAB; // 1st Data Memory Bus Address
  WordP CAB; // 2nd Data Memory Bus Address
  WordP BAB; // 3rd Data Memory Bus Address
  Word DB;  // Data Memory Read from 1st Data Memory Bus
  Word CB;  // Data Memory Read from 2nd Data Memory Bus
  Word BB;
  Word EAB; // Write Bus Address
  Word EB; // Write Data Register
  Word P;
  Word PM;  // Program Memory Read for things like READA
  WordP PAR; // Program Memory Read Address for things like READA
  Word DAR; // Register to hold address for DAB mvdm,mvkd
  Word RC;  // Repeat Counter
  WordP RETA; // Return address used for fast return from interrupt
  GP_Reg Shifter;
  char left_over_guard_bits_Shifter;
  WordP Lmem1;
  WordP Lmem2;

  Word CFCT; // repeat flags

  Word AR0H;
  Word AR1H;
  Word AR2H;
  Word AR3H;
  Word AR4H;
  Word AR5H;
  Word AR6H;
  Word AR7H;

  // admin
  int Special_Flush; // Used to inialize the pipeline
  int Flush; // set =1 to flush pipeline, done in read_stg1 stage
  int RC_first_pass; // set to 1 for read_stg1 following repeat single
  // Set to 1 to stall and calls that part of the pipe and only that
  // part of the pipe again.
  int Dont_Decode; // Set to non zero to not run any more
  // fetch, decode, or prefetch, PC increment.
  int fetch_flags; // Used to step breakpoints amoung other things
  int Decode_Again; // Decode Again
  WordP PAB_last;
};

struct _decoded_opcode
{
  const struct _Instruction_Class *class;
  int sub_type;
  int length;
  int var_length;
  WordP address;
  Opcode mach_code;
};

struct _PipeLine
{
  struct _decoded_opcode decode_nfo;
  // Opcode current_opcode;
  const struct _Instruction_Class *opcode_object;
  int opcode_subType;
  int cycles; // set in read portion because for the most part extra cycles
  // are caused by reading memory locations.
  // GPtrArray *operands;
  // int num_words; // number of words this takes up, determined by decoding
  int word_number; // which word number this is for the opcode
  int total_words; // total number of words for this opcode
  //Operand_List operands;
  Word storage1; // Just a place to store something that might be needed
  Word storage2; // in processing an opcode
  int flags; // bit 0 = break point set
#define PIPE_PC_CHANGED 2
#define PIPE_SKIP_NEXT_INSTR 4 // skips next instruction starting at address stage
#define PIPE_SKIP_NEXT_EXECUTE 8 // skips next execute stage
};

/* This defines the class structure for each instruction */
typedef gchar *(*Class_Type_Func)(int type);
typedef void (*Pipeline_Func)(struct _PipeLine *pipeP, struct _Registers *Reg);
typedef int (*OpcodeNumWord_Func )(struct _PipeLine *pipeP);
typedef int (*OpcodeSetCycle_Func )(Word opcode,int word_number);
typedef GPtrArray *(*Machine_Code_Func)(gchar *opcode_text);

struct _Instruction_Class
{
  gchar *name; // opcode name

  // pipeline functions
  Pipeline_Func decode;
  Pipeline_Func address_stg; // Address
  Pipeline_Func access_1;
  Pipeline_Func access_2;
  Pipeline_Func read_stg; 
  Pipeline_Func execute;
  Pipeline_Func write;
  Pipeline_Func write_plus;
  //Pipeline_Func set_operands;

  //OpcodeNumWord_Func number_words; // function to determine number of words this operand uses
  //OpcodeSetCycle_Func set_cycle_number; // number of cycles that this word
  // takes up. Don't include wait states.

  // One opcode can have different sets of mask bits.
  int size; // number of elements in mask,opcode, and comment
  gchar **mask;
  gchar **opcode;
  // of words that it codes into.
};

typedef struct _Instruction_Class Instruction_Class;


#define FIXME()(printf("FIXME %s:%d\n",__FILE__,__LINE__))

//#define SWORD_2_GP_REG(g,s)( (g).byte0=(s).byte
#define SIGN6BIT_TO_UINT(x)( ((x) & 0x20) ? ( ( (unsigned int)(-1) ^ 0x3f)  | ((x)&0x3f)) : ((x)&0x3f) ) 

#define K8_TO_GP_REG(gp,value)( { (gp).dword = (char)value;  (gp).bgp.byte4 = (value & 0x80) ? 0xff : 0; } )

// Set type GP_Reg (gp) equal to msb<<8 | lsb, sign extending if desired
#define BYTES_TO_GP_REG(gp,sign_extend,msb,lsb)({ (gp).bgp.byte0 = (lsb); (gp).bgp.byte1 = (msb); (gp).wgp.word1 = ( (sign_extend) && (msb&0x80) ) ? 0xffff : 0x0;  (gp).bgp.byte4 = ( (sign_extend) && (msb&0x80) ) ? 0xff : 0x0;  }) 

// Set type GP_Reg (gp) equal to msb<<24 | lsb<<16, sign extending if desired
#define BYTES_SHFT16_TO_GP_REG(gp,sign_extend,msb,lsb)({ (gp).wgp.word0 = 0; (gp).bgp.byte2 = (lsb); (gp).bgp.byte3 = (msb); (gp).bgp.byte4 = ( (sign_extend) && (msb&0x80) ) ? 0xff : 0x0;  }) 

#define GP_REG_TO_UINT64(gp)( (((guint64)(gp).bgp.byte4)<< 32) | ((guint64)(gp).dword) )
// convert with sign extention
#define GP_REG_TO_INT64(gp)( (gp).bgp.byte4 & 0x80 ? (gint64)((guint64)(0xffffff0000000000) | GP_REG_TO_UINT64(gp)) : (gint64)GP_REG_TO_UINT64(gp) )

// sign extend lower 32 bits of gp reg to int64
#define GP_REG32_TO_INT64(gp)( (gint64)(gp).dword )
// convert lower 32 bits of gp reg to uint64
#define GP_REG32_TO_UINT64(gp)( (guint64)(gp).dword )

#define ENTRY_CHAR_WIDTH 8
#define ENTRY_CHAR_HEIGHT 24

// method to get memory page
#define PAGE()((MMR->DPH)<<16)
//typedef unsigned char PWord;
#define WORD_TO_PWORD(wrd,address)(((address)&0x1)?(((wrd)>>8)&0xff):((wrd)&0xff))
#define PADDR_TO_ADDR(address)((address)>>1)
#define PROG_MEM_CONV 1

// signed
#define GP_REG17_TO_INT32(x) ( ((x).bgp.byte4 & 0x1 ) ? ( (guint32)0xffff0000 | ((guint32)(x).wgp.word1) ) : ((guint32)(x).wgp.word1 ) ) 

#define max_neg32 -2147483648ll 
#define max_pos32  2147483647ll
#define max_neg40 -549755813888ll
#define max_pos40  549755813887ll


// Memory Configuration
typedef guint8 WordX; // least amount of memory accessable
typedef enum { PROG_DATA_MEM_TYPE=0, PROGRAM_MEM_TYPE=1,  DATA_MEM_TYPE=2 } MemType;
#define NUM_TYPES_OF_MEM 3 // number of different types of MemType
#define NUM_BYTES_PER_WORDP 1
static const int NUM_BYTES_PER_MEM_TYPE[NUM_TYPES_OF_MEM]={1,1,2};
static const int NUM_WORDP_TO_DISPLAY_FOR_MEM_WINDOW[NUM_TYPES_OF_MEM]={2,2,2};


#endif // __C55_CORE_H__

