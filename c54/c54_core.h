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

#include "core_def.h"

#ifndef __C54_CORE_H__
#define __C54_CORE_H__

struct _MMR
{
  // CPU Memory-Mapped Registers
  Word IMR; // 0 - Interrupt Mask Register
  Word IFR; // 1 - Interrupt Flag Register
  Word reserved1; // 2
  Word reserved2; // 3
  Word reserved3; // 4
  Word reserved4; // 5
  Word ST0; // 6 - Status Register 0  see pg 90, 4-2 spru131g.pdf
            // 15-13 : ARP
            // 12    : TC
            // 11    : C
            // 10    : OVA
            //  9    : OVB
            //  8-0  : DP
  Word ST1; // 7 - Status Register 1  see pg 92 4-4 spru131g.pdf
            // 15    : BRAF
            // 14    : CPL
            // 13    : XF
            // 12    : HM
            // 11    : INTM
            // 10    : 0
            //  9    : OVM
            //  8    : SXM
            //  7    : C16
            //  6    : FRCT
            //  5    : CMPT
            //  4-0  : ASM


  GP_Reg A; // 8 - Accumulator A Low
            // 9 - Accumulator A High
            // A - Accumulator A Guard Bits
  char left_over_guard_bits_A; // fitting 40 bit A into a 48 bit slot
  GP_Reg B;
  char left_over_guard_bits_B; // fitting 40 bit B into a 48 bit slot
  Word T;
  Word TRN;
  Word ar0;
  Word ar1;
  Word ar2;
  Word ar3;
  Word ar4;
  Word ar5;
  Word ar6;
  Word ar7;
  Word SP;
  Word BK; // 19 - Circular Buffer Size Register
  Word BRC; // 1A - Block Repeat Counter
  Word RSA; // 1B - Block Repeat Start Address
  Word REA; // 1C - Block Repeat End Address
  Word PMST; // 1D - Processor Mode Status Register
  Word XPC; // 1E - Program counter extension;
  Word reserved5; // 1F
};

struct _Registers
{
  Word PC;  // Program Counter
  Word XPC;  // Extend bits for Program Counter
  Word PAB; // Program Address Bus. Filled with Prefetch
  Word PB;  // Program Data. Filled during Fetch
  Word IR;  // Instruction Register. Filled with Decode
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
  Word RSA; // Repeat Start Address (Block Repeats)
  Word REA; // Repeat End Address (Block Repeats)
};

extern struct _MMR *MMR;


// Convenience macros to read status bits
#define DP(Reg)(((Reg)->ST0&0x1ff))
#define OVB(Reg)((((Reg)->ST0)&0x200)>>9)
#define OVA(Reg)((((Reg)->ST0)&0x400)>>10)
#define C_bit(Reg)((((Reg)->ST0)&0x800)>>11)
#define TC_bit(Reg)((((Reg)->ST0)&0x1000)>>12)
#define ARP(Reg)((((Reg)->ST0)&0xe0000)>>13)

#define ASM(Reg)(signed_5bit_extract((Reg)->ST1))
#define CMPT(Reg)((((Reg)->ST1)&0x20)>>5)
#define FRCT(Reg)((((Reg)->ST1)&0x40)>>6)
#define C16(Reg)((((Reg)->ST1)&0x80)>>7)
#define SXM(Reg)((((Reg)->ST1)&0x100)>>8)
#define OVM(Reg)((((Reg)->ST1)&0x200)>>9)
#define INTM(Reg)((((Reg)->ST1)&0x800)>>11)
#define HM(Reg)((((Reg)->ST1)&0x1000)>>12)
#define XF(Reg)((((Reg)->ST1)&0x2000)>>13)
#define CPL(Reg)((((Reg)->ST1)&0x4000)>>14)
#define BRAF(Reg)((((Reg)->ST1)&0x8000)>>15)

// Convenience macros to set status bits
#define set_DP(Reg,data)((Reg)->ST0=((Reg)->ST0 & 0xfe00)|((data) & 0x1ff))
#define set_OVB(Reg,data)((Reg)->ST0=((Reg)->ST0 & 0xfdff)|(((data) & 0x1)<<9))
#define set_OVA(Reg,data)((Reg)->ST0=((Reg)->ST0 & 0xfbff)|(((data) & 0x1)<<10))
#define set_C(Reg,data)((Reg)->ST0=((Reg)->ST0 & 0xf7ff)|(((data) & 0x1)<<11))
#define set_TC(Reg,data)((Reg)->ST0=((Reg)->ST0 & 0xefff)|(((data) & 0x1)<<12))
#define set_ARP(Reg,data)((Reg)->ST0=((Reg)->ST0 & 0x1fff)|(((data) & 0x7)<<13))

#define set_ASM(Reg,data)((Reg)->ST1=((Reg)->ST1 & 0xffe0)|((data) & 0x1f))
#define set_INTM(Reg,data)((Reg)->ST1=((Reg)->ST1 & 0xf7ff)|((data) & 0x1)<<11)
#define set_BRAF(Reg,data)((Reg)->ST1=((Reg)->ST1 & 0x7fff)|((data) & 0x1)<<15)


// Need the operands to point to registers or be a constant value
// determined from the bits.

union u_operands
{
  GP_Reg *regP;
  SWord *arP;
  Word *u_arP; 
  SWord constant;
  Word u_constant;
  Word op_word;
  SWord op_sword;
};

#if 0
struct _Operand_List
{
  int num_operands;
  union u_operands op0;
  union u_operands op1;
  union u_operands op2;
  union u_operands op3;
};

typedef struct _Operand_List Operand_List;
#endif


struct _decoded_opcode
{
  const struct _Instruction_Class *class;
  int sub_type;
  int length;
  int var_length;
  WordA address;
  Word mach_code[3];
};

struct _PipeLine
{
  struct _decoded_opcode decode_nfo;
  Word current_opcode;
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
  Pipeline_Func prefetch;
  Pipeline_Func fetch;
  Pipeline_Func decode;
  Pipeline_Func read_stg1;
  Pipeline_Func read_stg2;
  Pipeline_Func execute;
  //Pipeline_Func set_operands;

  OpcodeNumWord_Func number_words; // function to determine number of words this operand uses
  OpcodeSetCycle_Func set_cycle_number; // number of cycles that this word
  // takes up. Don't include wait states.

  // One opcode can have different sets of mask bits.
  int size; // number of elements in mask,opcode, and comment
  gchar **mask;
  gchar **opcode;
  gchar **comment;
  Machine_Code_Func machine_code; // Given some opcode text, this returns an array
  // of words that it codes into.
};

typedef struct _Instruction_Class Instruction_Class;

#define ENTRY_CHAR_WIDTH 8
#define ENTRY_CHAR_HEIGHT 24

// method to get memory page
#define PAGE()(DP(MMR)<<16)

typedef Word PWord;
#define WORD_TO_PWORD(wrd,address)((wrd))
#define PADDR_TO_ADDR(address)((address))
#define PROG_MEM_CONV 2

#endif // __C54_CORE_H__

