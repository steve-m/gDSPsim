#include "hardware.h"
#include <stdio.h>

// See these page 98, 4-10
// spru131g.pdf C54_Reference_Set.pdf


GP_Reg alu_add(GP_Reg op1, SWord op2, SWord op2_shift, struct _Registers *Registers)
{
  // Adds 40 bit op1, shifts 16 bit op2 by op2_shift and adds the result.
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
