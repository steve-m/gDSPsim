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

// (shift amount control)
// 0-3 = Tx
// 4 = Immediate

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
      
    }

  if ( input_mux < 4 )
    {
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
	  overflow_bit = 39;
	  sat = SATD(MMR);
	}
      else
	{
	  overflow_bit = 31;
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
      shift = (SWord)MMR->T0;
      break;
    case 1:
      shift = (SWord)MMR->T1;
      break;
    case 2:
      shift = (SWord)MMR->T2;
      break;
    case 3:
      shift = (SWord)MMR->T3;
      break;
    }


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

  if ( overflow )
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
    }
}

