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
     
#include "chip_core.h"
#include "smem.h"

// p is 0-7, pointer register number
// mod is type of modification 0-15, excluding  6 and 7
void smem_set_DAB(int p, int mod)
{
  Word *arfP;
  extern struct _Registers *Registers;

  arfP=get_pointer_reg(p);

  switch (mod)
    {
    case 0:
      Registers->DB = *arfP;
      return;
    case 1:
      Registers->DB = *arfP;
      *arfP = *arfP + 1;
      return;
    case 2:
      Registers->DB = *arfP;
      *arfP = *arfP - 1;
      return;
    case 3:
      Registers->DB = *arfP;
      *arfP = *arfP + MMR->T0;
      return;
    case 4:
      Registers->DB = *arfP;
      *arfP = *arfP - MMR->T0;
      return;
    case 5:
      // *(ARp(t0)
      FIXME();
      return;
    case 9:
      Registers->DB = *arfP;
      *arfP = *arfP + MMR->T1;
      return;
    case 10:
      Registers->DB = *arfP;
      *arfP = *arfP - MMR->T1;
      return;
    case 11:
      // *(ARp(t1)
      FIXME();
      return;
    case 12:
      *arfP = *arfP + 1;
      Registers->DB = *arfP;
      return;
    case 13:
      *arfP = *arfP - 1;
      Registers->DB = *arfP;
      return;
    }
}

Word bit_reversal_add(Word start, Word bit_reversed_one)
{
  Word base;
  SWord index,br_index,add;

  bit_reversed_one = bit_reversed_one<<1;

  index = start & (bit_reversed_one-1);

  add=1;
  br_index = 0;
  while (index)
    {
      index=index<<1;
      if ( index & bit_reversed_one )
	{
	  br_index = br_index + add;
	}
      add = add << 1;
    }

  // now we can make the adjustment
  br_index = br_index + 1;

  // looped around
  if ( br_index == bit_reversed_one )
    br_index=0;
  if ( br_index < 0 )
    br_index=bit_reversed_one-1;

  // reverse again
  add=1;
  index = 0;
  while (br_index)
    {
      br_index=br_index<<1;
      if ( br_index & bit_reversed_one )
	{
	  index = index + add;
	}
      add = add << 1;
    }

  base = start & ~(bit_reversed_one-1);

  return base+index;

}
     
// r register number 0-3
void set_s16_ACx(int r, SWord value)
{
  union _GP_Reg_Union reg_union;

  reg_union.gint64 = value;

  switch ( r )
    {
    case 0:
      MMR->AC0 = reg_union.gp_reg;
      break;
    case 1:
      MMR->AC1 = reg_union.gp_reg;
      break;
    case 2:
      MMR->AC0 = reg_union.gp_reg;
      break;
    case 3:
      MMR->AC1 = reg_union.gp_reg;
      break;
    }
}

Word *get_pointer_reg(int p)
{
  switch (p)
    {
    case 0:
    default:
      return &MMR->ar0;
    case 1:
      return &MMR->ar1;
    case 2:
      return &MMR->ar2;
    case 3:
      return &MMR->ar3;
    case 4:
      return &MMR->ar4;
    case 5:
      return &MMR->ar5;
    case 6:
      return &MMR->ar6;
    case 7:
      return &MMR->ar7;
    }
}
