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

#include <multiplier.h>
#include <stdlib.h>
#include <stdio.h>

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
  guint32 ui32;
  struct _words words;
  struct _bytes bytes;
  
};

/* Xmux 0-3 ACx[32:16]
 *      4 DB
 *      5 CB
 *      6 BB
 *      7 P
 * Ymux same as above
 * Amux 0-3, Accumulate using ACx
 *      4-7, Accumulate by subtracting from ACx
 *      8, Don't Accumulate
 * Smux 0-3, Store in ACx register
 *      4-7, Store in ACx register with Rounding
 */

void multiplier(int Xmux, int Ymux, int Amux, int Smux, struct _Registers *Reg)
{
  union _bitconv X,Y; // Really need 17-bits
  gint64 Result64;
  union _GP_Reg_Union reg_union;
  int saturated=0;

  switch (Xmux)
    {
    case 0:
      X.ui32 = GP_REG17_TO_UINT32(MMR->AC0);
      break;
    case 1:
      X.ui32 = GP_REG17_TO_UINT32(MMR->AC1);
      break;
    case 2:
      X.ui32 = GP_REG17_TO_UINT32(MMR->AC2);
      break;
    case 3:
      X.ui32 = GP_REG17_TO_UINT32(MMR->AC3);
      break;
    case 7:
      X.i32 = (SWord)Reg->P;
      break;
    default:
      FIXME();
      break;
    }


  switch (Ymux)
    {
    case 0:
      X.ui32 = GP_REG17_TO_UINT32(MMR->AC0);
      break;
    case 1:
      X.ui32 = GP_REG17_TO_UINT32(MMR->AC1);
      break;
    case 2:
      X.ui32 = GP_REG17_TO_UINT32(MMR->AC2);
      break;
    case 3:
      X.ui32 = GP_REG17_TO_UINT32(MMR->AC3);
      break;
    case 7:
      X.i32 = (SWord)Reg->P;
      break;
    default:
      FIXME();
      break;
    }

  // Now multiply X*Y in 17 bits to get 40 bits
  Result64 = X.i32*Y.i32;

  if ( FRCT(MMR) )
    Result64 = Result64 << 1;

  reg_union.gint64 = Result64;

  if ( Smux > 3 )
    {
      Smux = Smux - 4;
      // Rounding
      if ( RDM(MMR) || C54CM(MMR) )
	{
	  if ( reg_union.words.low > 0x8000 )
	    {
	      reg_union.guint64 = reg_union.guint64 + 0x8000;
	    }
	  else if ( (reg_union.words.low == 0x8000) && 
		    (reg_union.words.high & 1 ) )
	    {
	      reg_union.guint64 = reg_union.guint64 + 0x8000;
	    }
	  reg_union.words.low = 0;
	}
      else
	{
	  reg_union.guint64 = reg_union.guint64 + 0x8000;
	  reg_union.words.low = 0;
	}
    }

  // SATD=0 no saturation
  // SATD=1 M40=0 satuate to 00 7fff ffff   ff 8000 0000
  // SATD=1 M40=1 satuate to 7f ffff ffff   80 0000 0000

  // SATD and SMUL both on for saturation?
  if ( SMUL(MMR) && SATD(MMR) )
    {
      // check for saturation before Accumulation
      if ( M40(MMR) )
	{
	  if ( reg_union.gint64 > max_pos40 )
	    {
	      reg_union.gint64 = max_pos40;
	      saturated=1;
	    }
	  if ( reg_union.gint64 < max_neg40 )
	    {
	      reg_union.gint64 = max_neg40;
	      saturated=1;
	    }
	}
      else
	{
	  if ( reg_union.gint64 > max_pos32 )
	    {
	      reg_union.gint64 = max_pos32;
	      saturated=1;
	    }
	  if ( reg_union.gint64 < max_neg32 )
	    {
	      reg_union.gint64 = max_neg32;
	      saturated=1;
	    }
	}
    }

  // Now add 40 bits to 40 bits
  if ( Amux != 8 )
    {
      FIXME();
    }
 
  switch ( Smux )
    {
    case 0:
      MMR->AC0 = reg_union.gp_reg;
      if ( saturated )
	set_ACOV0(MMR,1);
      break;
    case 1:
      MMR->AC1 = reg_union.gp_reg;
      if ( saturated )
	set_ACOV1(MMR,1);
      break;
    case 2:
      MMR->AC2 = reg_union.gp_reg;
      if ( saturated )
	set_ACOV2(MMR,1);
      break;
    case 3:
      MMR->AC3 = reg_union.gp_reg;
      if ( saturated )
	set_ACOV3(MMR,1);
      break;
    }

}
