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

#include "shifter.h"
#include "decode.h"
#include "memory.h"
#include "chip_core.h"

// mux (input control)
// 0 = AC0
// 1 = AC1
// 2 = AC2
// 3 = AC3
// 4 = T0
// 5 = T1
// 6 = T2
// 7 = T3
// 8-15 = ARx
// 16 = DB

// (shift amount control)
// 0-3 = Tx
// 4-7 = Tx with rounding
// 8 = Immediate

// output_mux
// 0 = AC0
// 1 = AC1
// 2 = AC2
// 3 = AC3
// 4 = T0
// 5 = T1
// 6 = T2
// 7 = T3
// 8-15 = ARx

// 16 bit numbers are automatically sign extended
inline void shifter(Word input_mux, Word shift_mux, SWord shift, 
		    Word output_mux, struct _Registers *Reg )
{
  union _GP_Reg_Union reg;
  int overflow_bit,overflow,sat=0;
  guint64 ovrflw_mask;

  reg.guint64 = 0;

  switch ( input_mux )
    {
    case 0:
      reg.gp_reg = MMR->AC0;
      break;
    case 1:
      reg.gp_reg = MMR->AC1;
      break;
    case 2:
      reg.gp_reg = MMR->AC2;
      break;
    case 3:
      reg.gp_reg = MMR->AC3;
      break;
    case 4:
      reg.gint64 = (SWord)MMR->T0;
      break;
    case 5:
      reg.gint64 = (SWord)MMR->T1;
      break;
    case 6:
      reg.gint64 = (SWord)MMR->T2;
      break;
    case 7:
      reg.gint64 = (SWord)MMR->T3;
      break;
    case 8:
      reg.gint64 = (SWord)MMR->ar0;
      break;
    case 9:
      reg.gint64 = (SWord)MMR->ar1;
      break;
    case 10:
      reg.gint64 = (SWord)MMR->ar2;
      break;
    case 11:
      reg.gint64 = (SWord)MMR->ar3;
      break;
    case 12:
      reg.gint64 = (SWord)MMR->ar4;
      break;
    case 13:
      reg.gint64 = (SWord)MMR->ar5;
      break;
    case 14:
      reg.gint64 = (SWord)MMR->ar6;
      break;
    case 15:
      reg.gint64 = (SWord)MMR->ar7;
      break;
    case 16:
      if ( SXMD(MMR) )
	reg.gint64 = (SWord)Reg->DB;
      else
	reg.guint64 = Reg->DB;
    }

  if ( input_mux < 4 )
    {
      sat = SATD(MMR);
      // It's an accumulator
      if ( (M40(MMR)==0) && (C54CM(MMR)==0) )
	{
	  if ( SXMD(MMR) )
	    {
	      if ( reg.gp_reg.bgp.byte3 & 0x80 )
		reg.gp_reg.bgp.byte4 = 0xff;
	      else
		reg.gp_reg.bgp.byte4 = 0;
	    }
	  else
	    {
	      reg.gp_reg.bgp.byte4 = 0;
	    }
	  overflow_bit = 31;
	}
      else
	{
	  overflow_bit = 39;
	}
    }
  else
    {
      overflow_bit = 15;
      sat = SATA(MMR);
    }

  switch ( shift_mux )
    {
    case 0:
    case 4:
      shift = (SWord)MMR->T0;
      break;
    case 1:
    case 5:
      shift = (SWord)MMR->T1;
      break;
    case 2:
    case 6:
      shift = (SWord)MMR->T2;
      break;
    case 3:
    case 7:
      shift = (SWord)MMR->T3;
      break;
    }

  // shift can be from 31 to -16
  if ( shift > 31 )
    shift = shift & 0x3f;
  else if ( shift < -16 )
    shift = (SWord)((Word)shift | 0xfff0 );


  // Check for overflow
  if ( shift > 0 )
    {
      ovrflw_mask =(guint64)(-1) ^ (((guint64)1<<(overflow_bit-shift+1))-1);
      if ( ((reg.guint64 & ovrflw_mask) == 0) || 
	   ((reg.guint64 & ovrflw_mask) == ovrflw_mask) )
	{
	  overflow=0;
	}
      else
	{
	  overflow=1;
	}
    }

  if ( shift < 0 )
    {
      reg.guint64 = reg.guint64 >> -shift;
    }
  else
    {
      reg.guint64 = reg.guint64 << shift;
    }

  if ( (shift_mux>=4) && (shift_mux<=7) )
    {
      // Rounding
      if ( RDM(MMR) && C54CM(MMR)==0 )
	{
	  if ( reg.words.low > 0x8000 )
	    {
	      reg.guint64 = reg.guint64 + 0x8000;
	    }
	  else if ( (reg.words.low == 0x8000) && 
		    (reg.words.high & 1 ) )
	    {
	      reg.guint64 = reg.guint64 + 0x8000;
	    }
	  reg.words.low = 0;
	}
      else
	{
	  reg.guint64 = reg.guint64 + 0x8000;
	  reg.words.low = 0;
	}
    }

  if ( overflow && sat )
    {
      if ( reg.gint64 > 0 )
	{
	  if ( overflow_bit == 15 )
	    {
	      reg.guint64 = 0x7fff;
	    }
	  else if ( overflow_bit == 31 )
	    {
	      reg.guint64 = 0x7fffffff;
	    }
	  else if ( overflow_bit == 39 )
	    {
	      reg.guint64 = 0x7fffffffff;
	    }
	}
      else
	{
	  if ( overflow_bit == 15 )
	    {
	      reg.guint64 = 0x8000;
	    }
	  else if ( overflow_bit == 31 )
	    {
	      reg.guint64 = 0x80000000;
	    }
	  else if ( overflow_bit == 39 )
	    {
	      reg.guint64 = 0x8000000000;
	    }
	}
    }


  switch ( output_mux )
    {
    case 0:
      MMR->AC0 = reg.gp_reg;
      break;
    case 1:
      MMR->AC1 = reg.gp_reg;
      break;
    case 2:
      MMR->AC2 = reg.gp_reg;
      break;
    case 3:
      MMR->AC3 = reg.gp_reg;
      break;
    case 4:
      MMR->T0 = reg.words.low;
      break;
    case 5:
      MMR->T1 = reg.words.low;
      break;
    case 6:
      MMR->T2 = reg.words.low;
      break;
    case 7:
      MMR->T3 = reg.words.low;
      break;
    case 8:
      MMR->ar0 = reg.words.low;
      break;
    case 9:
      MMR->ar1 = reg.words.low;
      break;
    case 10:
      MMR->ar2 = reg.words.low;
      break;
    case 11:
      MMR->ar3 = reg.words.low;
      break;
    case 12:
      MMR->ar4 = reg.words.low;
      break;
    case 13:
      MMR->ar5 = reg.words.low;
      break;
    case 14:
      MMR->ar6 = reg.words.low;
      break;
    case 15:
      MMR->ar7 = reg.words.low;
      break;
    }
}

