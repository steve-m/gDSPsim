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

struct _bytes
{
  unsigned char byte0;
  unsigned char byte1;
  unsigned char byte2;
  unsigned char byte3;
}; 
union _bitconv
{
  gint32 i32;
  guint32 iu32;
  Word word;
  struct _bytes bytes;
};

/* 
 * Xmux 0, X operand is from DB register
 *      1, X operand is from shifter register
 * Ymux 0, Y operand is from A register
 *      1, Y operand is from B register
 *      2, Y operand is from T register
 *      3, Y operand is from CB register
 * Omux 0, store answer in A
 *      1, store answer in B
 * flag bit 0-1, 0=add 1=Y-X 2=X-Y
 *      bit 2 shift DB and CB by 16 each (SUB)
 *      bit 3 40 bit mode
 *      bit 4 ignored SXM
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
      X.words.low = Reg->DB;
      break;
    case 1:
      X.gp_reg = Reg->Shifter;
      break;
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
      Y.words.low = MMR->T;
      break;
    case 3:
      Y.words.low = Reg->CB;
      break;
    }

  if ( (flag & 3)==0 )
    {
      if ( (Xmux==1) & C16(MMR) )
	{
	  X.words.low = Y.words.low + X.words.low;
	  t.i32 = Y.words.high + X.words.high;
	  X.words.high = t.word;
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
	  t.i32 = Y.words.high - X.words.high;
	  X.words.high = t.word;
	  X.gp_reg.byte4 = t.bytes.byte3;
	}
      else
	{
	  X.gint64 = Y.gint64 - X.gint64;
	}
    }
  else if ( (flag&3)==2 )
    {
      if (Ymux==2)
	{
	  // dsubt uses this path
	  if ( C16(MMR) )
	    {
	      X.words.low = X.words.low - Y.words.low;
	      t.i32 = X.words.high - Y.words.low;
	      X.words.high = t.word;
	      X.gp_reg.byte4 = t.bytes.byte3;
	    }
	  else
	    {
	      X.words.high = X.words.low;
	      X.gint64 = X.gint64 - Y.gint64;
	    }
	}
      else if ( (Xmux==1) & C16(MMR) )
	{
	  X.words.low = X.words.low - Y.words.low;
	  t.i32 = X.words.high - Y.words.high;
	  X.words.high = t.word;
	  X.gp_reg.byte4 = t.bytes.byte3;
	}
      else
	{
	  X.gint64 = X.gint64 - Y.gint64;
	}
    }

  // Check for overflow and set overflow bit
  if ( X.gint64 > 0x7fffffff )
    {
      if ( Omux )
	set_OVB(MMR,1);
      else
	set_OVA(MMR,1);
    }
  else if ( X.gint64 < -((gint64)1<<31) )
    {
      if ( Omux )
	set_OVB(MMR,1);
      else
	set_OVA(MMR,1);
    }

  if ( Omux )
    MMR->B = X.gp_reg;
  else
    MMR->A = X.gp_reg;

}
