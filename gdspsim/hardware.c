#include "hardware.h"
#include <stdio.h>

// See these page 98, 4-10
// spru131g.pdf C54_Reference_Set.pdf

/*

12   Test/control flag. TC stores the results of the arithmetic logic unit (ALU) test bit op-
erations. TC is affected by the BIT, BITF, BITT, CMPM, CMPR, CMPS, and SFTC
instructions. The status (set or cleared) of TC determines if the conditional branch,
call, execute, and return instructions execute.
TC = 1    if the following conditions are true:
- A bit tested by BIT or BITT is a 1.
- A compare condition tested by CMPM, CMPR, or CMPS exists between a data-
memory value and an immediate operand, AR0 and another auxiliary register, or an
accumulator high word and an accumulator low word.
- Bit 31 and bit 30 of an accumulator tested by SFTC have different values from
each other.

11    C    1    Carry is set to 1 if the result of an addition generates a carry; it is cleared to 0 if the
result of a subtraction generates a borrow. Otherwise, it is reset after an addition and
it is set after a subtraction, except for an ADD or SUB with a 16-bit shift. In these
cases, the ADD can only set and the SUB only reset the carry bit, but they cannot
affect it otherwise. Carry and borrow are defined at the 32nd bit position and are
operated at the ALU level only. The shift and rotate instructions (ROR, ROL, SFTA,
and SFTL), and the MIN, MAX, ABS, and NEG instructions also affect this bit.
10    OVA    0    Overflow flag for accumulator A. OVA is set to 1 when an overflow occurs in either
the ALU or the multiplier's adder and the destination for the result is accumulator A.
Once an overflow occurs, OVA remains set until either a reset, a BC[D], a CC[D],
an RC[D], or an XC instruction is executed using the AOV and ANOV conditions.
The RSBX instruction can also clear this bit.
9    OVB    0    Overflow flag for accumulator B. OVB is set to 1 when an overflow occurs in either
the ALU or the multiplier's adder and the destination for the result is accumulator B.
Once an overflow occurs, OVB remains set until either a reset, a BC[D], a CC[D],
an RC[D], or an XC instruction is executed using the BOV and BNOV conditions.
This RSBX instruction can also clear this bit.

*/

// Adds 40 bit op1, shifts 16 bit op2 by op2_shift and adds the result.

// Flags affected. OVA, OVB, C, ZB, ZA, TC
GP_Reg alu_add(GP_Reg op1, SWord op2, SWord op2_shift, Word SXM)
{
  union _GP_Reg_Union reg_union;
  union _GP_Reg_Union reg_union2;
  guint64 Cbit;

  reg_union.gint64 = 0;
  reg_union.gp_reg = op1;

  // Sign extend
  reg_union2.gint64 = 0;
  reg_union2.words.low = op2;

  // Save Carry bit
  Cbit = reg_union.gint64 & 0x100000000;

  if ( (SXM != 0) && ((reg_union2.guint64 & 0x8000) != 0) )
    {
      reg_union2.guint64 = reg_union2.guint64 | 0xffffffffffff0000;
    }

  reg_union2.guint64 = reg_union2.guint64 << op2_shift;

  // Set Carry bit, if it changed.
  if ( Cbit != ( reg_union2.guint64 & 0x100000000 ) )
    {
      // Set Carry bit.
      MMR->ST0 = MMR->ST0 | 0x800;
    }
  else if ( op2_shift != 16 )
    {
      // Reset Carry bit if not a 16 bit shift and Carry hasn't been set
      MMR->ST0 = MMR->ST0 & 0xf7ff;
    }


  reg_union2.gint64 = reg_union2.gint64 + reg_union.gint64;

  return reg_union2.gp_reg;

}

// Shifts 2 operand by op2_shift then adds the 2 40 bit numbers
GP_Reg alu_add40(GP_Reg op1, GP_Reg op2, SWord op2_shift, struct _Registers *Registers)
{
  GP_Reg ans;

  return ans;
}


// This converts op to a GP_Reg, then shifts it
GP_Reg word_to_GP_Reg(SWord op, SWord op_shift)
{
  GP_Reg ans;

  return ans;
}
  

// This shifts op, then converts the low bits to a word
Word GP_Reg_Low_to_Word(GP_Reg op, SWord op_shift)
{
  // Adds 40 bit op1, shifts 16 bit op2 by op2_shift and adds the result.
  Word ans;

  return ans;
}


Word *get_pointer_reg(int arf, struct _Registers *Registers, int CMPT)
{
  Word *arfP;

  if ( CMPT )
    {
      if ( arf != 0 )
	set_ARP(MMR,arf);
      else
	return get_pointer_reg(ARP(MMR),Registers,0);
    }

 switch ( arf )
   {
   case 0:
     {
       arfP = &MMR->ar0;
       return arfP;
     }
   case 1:
     {
       arfP = &MMR->ar1;
       return arfP;
     }
   case 2:
     {
       arfP = &MMR->ar2;
       return arfP;
     }
   case 3:
     {
       arfP = &MMR->ar3;
       return arfP;
     }
   case 4:
     {
       arfP = &MMR->ar4;
       return arfP;
     }
   case 5:
     {
       arfP = &MMR->ar5;
       return arfP;
     }
   case 6:
     {
       arfP = &MMR->ar6;
       return arfP;
     }
   case 7:
     {
       arfP = &MMR->ar7;
       return arfP;
     }
   }

  printf("Error decoding pointer register %s:%d\n",__FILE__,__LINE__);

  return NULL;
}  

// This returns a pointer to a memory mapped register. The code
// can be from 0 to 8
Word *get_mmr(Word mm_code, struct _Registers *Reg)
{
  Word *mmr;

  g_return_val_if_fail(mm_code<=8,NULL);

  if ( mm_code == 8 )
    {
      mmr = &MMR->SP;
      return mmr;
    }
  else
    return get_pointer_reg(mm_code,Reg,0);
}

void set_mmr(Word mm_code, Word value, struct _Registers *Reg)
{
  g_return_if_fail(mm_code<=8);

  if ( mm_code == 8 )
    {
      MMR->SP = value;
    }
  else
    set_pointer_reg(mm_code,value,Reg);
}

void set_pointer_reg(int arf, Word value, struct _Registers *Reg)
{
  switch (arf)
    {
    case 0:
      MMR->ar0 = value;
      return;
    case 1:
      MMR->ar1 = value;
      return;
    case 2:
      MMR->ar2 = value;
      return;
    case 3:
      MMR->ar3 = value;
      return;
    case 4:
      MMR->ar4 = value;
      return;
    case 5:
      MMR->ar5 = value;
      return;
    case 6:
      MMR->ar6 = value;
      return;
    case 7:
      MMR->ar7 = value;
      return;
    }
}
