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
typedef gint16 SWord;
typedef guint32 WordA; //type of variable used for addressing Words.
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
};

typedef union _GP_Reg GP_Reg;

#if 0
struct _byte_opcode
{
  unsigned char byte0;
  unsigned char byte1;
  unsigned char byte2;
  unsigned char byte3;
  unsigned char byte4;
  unsigned char byte5;
  unsigned char byte6;
  unsigned char byte7;
};
#endif

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
  WordA address;
};;

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
  Word BAS23;
  Word BSA45;
  Word BSA67;
  Word BSAC;
  
  Word reserved4;

  Word TRN1;
  Word BRC1;
  Word BRS1;
  Word CSR;
  
  WordA RSA0;
  // Word RSA0H;
  // Word RSA0L;

  WordA REA0;
  // Word REA0H;
  // Word REA0L;

  WordA RSA1;
  //Word RSA1H;
  //Word RSA1L;

  WordA REA1;
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

// Convenience macros to set status bits
#define set_BRAF(Reg,data)((Reg)->ST1_55=((Reg)->ST1_55 & 0x7fff)|((data) & 0x1)<<15)

struct _Registers
{
  WordA PC;  // Program Counter
  Word XPC;  // Extend bits for Program Counter
  //Word PAB; // Program Address Bus. Filled with Prefetch
  //Word PB;  // Program Data. Filled during Fetch
  //Word IR;  // Instruction Register. Filled with Decode
  Word DAB; // 1st Data Memory Bus Address
  Word CAB; // 2nd Data Memory Bus Address
  Word DB;  // Data Memory Read from 1st Data Memory Bus
  Word CB;  // Data Memory Read from 2nd Data Memory Bus
  Word EAB; // Write Bus Address
  Word P;
  Word PM;  // Program Memory Read for things like READA
  WordA PAR; // Program Memory Read Address for things like READA
  Word DAR; // Register to hold address for DAB mvdm,mvkd
  Word RC;  // Repeat Counter
  Word RTN; // Return address used for fast return from interrupt
  GP_Reg Shifter;
  char left_over_guard_bits_Shifter;
  WordA Lmem1;
  WordA Lmem2;

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
  WordA PAB_last;
};

struct _decoded_opcode
{
  const struct _Instruction_Class *class;
  int sub_type;
  int length;
  int var_length;
  WordA address;
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

#define SWORD_2_GP_REG(g,s)( (g).byte0=(s).byte
#define SIGN6BIT_TO_UINT(x)( ((x) & 0x20) ? ( ( (unsigned int)(-1) ^ 0x3f)  | ((x)&0x3f)) : ((x)&0x3f) ) 

#define ENTRY_CHAR_WIDTH 8
#define ENTRY_CHAR_HEIGHT 24

// method to get memory page
#define PAGE()((MMR->DP)<<16)
typedef unsigned char PWord;
#define WORD_TO_PWORD(wrd,address)(((address)&0x1)?(((wrd)>>8)&0xff):((wrd)&0xff))
#define PADDR_TO_ADDR(address)((address)>>1)

#endif // __C55_CORE_H__

