/*
 * gDSPsim - GNU Digital Signal Processor Simulator
 *
 * Copyright (C) 2001, Kerry Keal, kerry@industrialmusic.com
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

#include "alu.h"
#include "memory.h"

struct _bytes
{
  unsigned char byte0;
  unsigned char byte1;
  unsigned char byte2;
  unsigned char byte3;
};

struct _words
{
  Word low;
  Word high;
};

union _bitconv
{
  gint32 i32;
  guint32 iu32;
  struct _words words;
  struct _bytes bytes;
  
};

/* 
 * Xmux 0, X operand is from DB register
 *      1, X operand is from shifter register
 *      2, X operand is from A(32-16) (ABDST)
 * Ymux 0, Y operand is from A register
 *      1, Y operand is from B register
 *      2, Y operand is from T register
 *      3, Y operand is from CB register
 * Omux 0, store answer in A
 *      1, store answer in B
 *      2, store answer in EAB
 * flag bit 0-1, 0=add 1=Y-X 2=X-Y 3=sub high word, add low word (DSADT)
 *      bit 2 shift DB and CB by 16 each (SUB,SQDST,LMS)
 *      bit 3 40 bit mode
 *      bit 4 ignored SXM
 *      bit 5 round result by adding 2^15
 *      bit 6 ADD can only set and SUB can only reset Carry bit
 */ 
void alu(int Xmux, int Ymux, int Omux, int flag, struct _Registers *Reg)
{
  union _GP_Reg_Union X,Y;
  union _bitconv t;

  X.gint64 = 0;
  Y.gint64 = 0;

  switch (Xmux)
    {
    case 0:
      if ( SXM(MMR) )
	X.gint64 = (SWord)Reg->DB;
      else
	X.words.low = Reg->DB;
      break;
    case 1:
      X.gp_reg = Reg->Shifter;
      break;
    case 2:
      {
	union _GP_Reg_Union reg_union;
	reg_union.gint64 = 0;
	reg_union.gp_reg = MMR->A;
	X.words.low = reg_union.words.high;
	if ( reg_union.words.ext & 0x1 )
	  {
	    if ( SXM(MMR) )
	      {
		// make negative
		X.gu32.high = 0xffffffff;
	      }
	      else
		{
		  X.gu32.high = 1;
		}
	  }
	break;
      }
    }

  switch (Ymux)
    {
    case 0:
      Y.gp_reg = MMR->A;
      break;
    case 1:
      Y.gp_reg = MMR->B;
      break;
    case 2:
      if ( SXM(MMR) )
	Y.gint64 = (SWord)MMR->T;
      else
	Y.words.low = MMR->T;
      break;
    case 3:
      if ( SXM(MMR) )
	Y.gint64 = (SWord)Reg->CB;
      else
	Y.words.low = Reg->CB;
      break;
    }

  if ( (flag & 3)==0 )
    {
      if (  (flag&8) && C16(MMR) )
	{
	  X.words.low = Y.words.low + X.words.low;
	  t.i32 = (gint32)Y.words.high + (gint32)X.words.high;
	  X.words.high = t.words.low;
	  X.gp_reg.byte4 = t.bytes.byte3;
	}
      else
	{
	  X.gint64 = X.gint64 + Y.gint64;
	}
    }
  else if ( (flag&3)==1 )
    {
      if ( (Xmux==1) & C16(MMR) )
	{
	  X.words.low = Y.words.low - X.words.low;
	  t.i32 = (gint32)Y.words.high - (gint32)X.words.high;
	  X.words.high = t.words.low;
	  X.gp_reg.byte4 = t.bytes.byte3;
	}
      else
	{
	  X.gint64 = Y.gint64 - X.gint64;
	}
    }
  else if ( (flag&3)==2 )
    {
      // X - Y
      if ( (flag&8) && C16(MMR) )
	{
	  // DRSUB is example of this path
	  X.words.low = X.words.low - Y.words.low;
	  t.i32 = (gint32)X.words.high - (gint32)Y.words.low;
	  X.words.high = t.words.low;
	  X.gp_reg.byte4 = t.bytes.byte3;
	}
      else
	{
	  X.gint64 = X.gint64 - Y.gint64;
	}
    }
  else if ( (flag&3)==3 )
    {
      // See DSADT
      if ( C16(MMR) )
	{
	  Y.words.high = Y.words.low;
	  X.words.low = X.words.low + Y.words.low;
	  t.i32 = (guint32)X.words.high - (guint32)Y.words.low;
	  // X.words.high = t.words.low;
	  X.gp_reg.byte4 = 0;//t.bytes.byte2;
	}
      else
	{
	  Y.words.high = Y.words.low;
	  if ( SXM(MMR) )
	    {
	      X.gint64 = (gint64)X.gi32.low - (gint64)Y.gi32.low;
	    }
	  else
	    {
	      X.guint64 = X.guint64 - Y.guint64;
	    }
	}
    }
    
  
  if ( flag & 0x20 )
    {
      // Round
      X.guint64 = X.guint64 + (1<<15);
    }


  // Check for overflow and set overflow bit
  if ( X.gint64 > 0x7fffffff )
    {
      if ( Omux==1 )
	set_OVB(MMR,1);
      else
	set_OVA(MMR,1);
      if ( OVM(MMR) )
	X.gint64 = 0x007fffffff;
    }
  else if ( X.gint64 < -((gint64)1<<31) )
    {
      if ( Omux==1 )
	set_OVB(MMR,1);
      else
	set_OVA(MMR,1);
      if ( OVM(MMR) )
	X.gint64 = 0xff80000000;
    }

  // Set C bit
  if ( (flag & 2) == 0 )
    {
      // Addition
      if ( X.guint64 & (guint64)0x100000000 )
        set_C(MMR,1);
      else if ( (flag & (1<<6)) == 0 )
        set_C(MMR,0);
    }
  else
    {
      // Subtraction
      if ( X.guint64 & (guint64)0x100000000 )
        set_C(MMR,1);
      else if ( (flag & (1<<6)) == 0 )
        set_C(MMR,0);
    }

  if ( Omux==0 )
    MMR->A = X.gp_reg;
  else if ( Omux==1 )
    MMR->B = X.gp_reg;
  else if ( Omux==2 )
    write_data_mem(Reg->EAB,X.words.low);
}
