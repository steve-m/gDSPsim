#include "core_def.h"

#ifndef __C54_CORE_H__
#define __C54_CORE_H__

extern struct _MMR *MMR;

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
  Word XPC; // 1D - Program counter extension;
  Word reserved5; // 1E
  Word reserved6; // 1F
};

struct _Registers
{
#if 0
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
  Word XPC; // 1D - Program counter extension;
  Word reserved5; // 1E
  Word reserved6; // 1F
#endif
 // Word PMST; // pg 96 spru131g.pdf
  // Word IPTR:8; // bits 15-7
  // Word MP_MC:1; // bit 6
  // Word OVLY:1; // bit 5
  // Word AVIS:1; // bit 4
  // Word DROM:1; // bit 3
  // Word CLKOFF:1; // bit 2
  // Word SMUL:1; // bit 1
  // Word SST:1; // bit 0
  
  // Word SXM:1; // sign extension bit  FIXME

 // Other Registers.
  //Word DP;
  //Word ASM;
  //Word ARP; // right location. fixme
  Word PC;
  //Word C16:1; // bit 7 of ST1  right location. fixme
  //Word CMPT:1;  // right location. fixme

  Word PAB; // Program Address Bus. Filled with Prefetch
  Word PB; // Program Data. Filled during Fetch
  Word IR; // Instruction Register. Filled with Decode
  Word DAB;
  Word CAB;
  Word DB;
  Word CB;
  Word EAB; // Write bus
  Word P;

  // admin
  int Flush; // set =1 to flush pipeline
};

extern struct _MMR *MMR;


// Convenience macros to read status bits
#define ARP(Reg)((((Reg)->ST0)&0x1fff)>>13)
#define SXM(Reg)((((Reg)->ST1)&0x100)>>8)
#define C16(Reg)((((Reg)->ST1)&0x80)>>7)
#define CMPT(Reg)((((Reg)->ST1)&0x20)>>5)

// Convenience macros to set status bits
#define set_ARP(Reg,data)((Reg)->ST0=((Reg)->ST0 & 0x1fff)|(((data) & 0x7)<<13))
#define set_DP(Reg,data)((Reg)->ST0=((Reg)->ST0 & 0xfe00)|((data) & 0x1ff))
#define set_ASM(Reg,data)((Reg)->ST1=((Reg)->ST1 & 0xffe0)|((data) & 0x1f))


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

struct _Operand_List
{
  int num_operands;
  union u_operands op0;
  union u_operands op1;
  union u_operands op2;
  union u_operands op3;
};

typedef struct _Operand_List Operand_List;

struct _PipeLine
{
  Word current_opcode;
  const struct _Instruction_Class *opcode_object;
  int opcode_subType;
  int cycles; // set in read portion because for the most part extra cycles
  // are caused by reading memory locations.
  // GPtrArray *operands;
  // int num_words; // number of words this takes up, determined by decoding
  int word_number; // which word number this is for the opcode
  int total_words; // total number of words for this opcode
  int cycle_number; // cycle number for each word in the opcode
  Operand_List operands;
  Word storage1; // Just a place to store something that might be needed
  Word storage2; // in processing an opcode
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


#endif // __C54_CORE_H__

