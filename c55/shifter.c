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
#include <chip_help.h>

  /* input_mux (input control)
   *    0-15      Register 0-15
   *    SHFT_DB   DB Register
   *
   * shift_mux
   *    0-3       Use Register T0-T3 for shift amount
   *    SHFT_CONSTANT  Use a Constant passed in shift for shift amount
   *
   * shift (for use when SHFT_CONSTANT (above) set)
   *
   * flag (See below for defines)
   *
   * output_mux
   *    0-15           Register 0-15
   *    SHFT_SHIFTER   Temporary Shifter Register
   *
   */

inline void shifter(Word input_mux, Word shift_mux, SWord shift, 
		    unsigned int flag, Word output_mux, 
		    struct _Registers *Reg )
{
  union _GP_Reg_Union reg;
  int overflow_bit,overflow,_M40,sat=0;
  guint64 ovrflw_mask,carry;

  if ( (flag & SHFT_UNSIGNED) || (SXMD(MMR)==0) )
    {
      if ( input_mux < 16 )
	{
	  reg = get_register(input_mux,0);
	}
      else if ( input_mux == SHFT_DB )
	{
	  reg.gint64 = Reg->DB;
	}
    }
  else
    {
      if ( input_mux < 16 )
	{
	  reg = get_register(input_mux,1);
	}
      else if ( input_mux == SHFT_DB )
	{
	  reg.gint64 = (SWord)Reg->DB;
	}
    }

  // Set _M40 to 1 or 0 if forced otherwise set to M40
  _M40 = (flag & (SHFT_M40_IS_1 | SHFT_M40_IS_0)) ? 
    ( (flag & SHFT_M40_IS_1) ? 1 : 0 ) : M40(MMR);

  // Set overflow_bit to 16 if 16-bit register or (40,32) depending on M40
  overflow_bit = ( input_mux < 4 ) ? ( (_M40) ? 40 : 32 ) : 16;
  
  switch ( shift_mux )
    {
    case 0:
    // case 4:
      shift = (SWord)MMR->T0;
      break;
    case 1:
    // case 5:
      shift = (SWord)MMR->T1;
      break;
    case 2:
    // case 6:
      shift = (SWord)MMR->T2;
      break;
    case 3:
    // case 7:
      shift = (SWord)MMR->T3;
      break;
    }

  // adjust shift amount to allowable limits
  shift = C54CM(MMR) ? (shift%32) : (shift%16);
  
  // set CARRY
  if ( (flag & SHFT_DONT_SET_CARRY) == 0 )
    {
      if ( shift < 0 )
	{
	  carry = reg.guint64 & (guint64)1<<(1-shift);
	}
      else if ( shift > 0 )
	{
	  if ( overflow_bit > shift )
	    carry = reg.guint64 & ( (guint64)1<<(overflow_bit-shift) );
	  else
	    carry = 0;
	}
      else
	{
	  carry = 0;
	}
      if ( carry )
	{
	  set_CARRY(MMR,1);
	}
      else
	{
	  set_CARRY(MMR,0);
	}
    }

  switch (flag & SHFT_DONT_SATURATE)
    {
    case SHFT_DONT_SATURATE:
      sat = 0;
      break;
    case SHFT_SATURATE:
      sat = 1;
      break;
    case SHFT_UNSIGNED_SATURATE:
      sat = 1;
      // fixme
      break;
    case SHFT_USE_SATD_SATA:
      if (input_mux<4)
	sat = SATD(MMR);
      else
	sat = SATA(MMR);
      break;
    }


  // Check for overflow
  if ( shift > 0 )
    {
      ovrflw_mask = ((guint64)1<< (overflow_bit-shift) ) - 1;

       if ( ((reg.guint64 | ovrflw_mask) == ovrflw_mask) || 
	   ((reg.guint64 | ovrflw_mask) == (gint64)-1) )
	 {
	 }


      ovrflw_mask =(guint64)(-1) ^ (((guint64)1<<(overflow_bit-shift))-1);
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
  else
    {
      overflow=0;
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

  // overflow detection
  if ( flag & SHFT_SET_ACOV )
    {
      if ( input_mux < 4 )
	{
	  set_ACOVx(MMR,input_mux);
	}
    }

  // saturate
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

  // Set output
  if ( output_mux < 16 )
    {
      set_register(reg,output_mux);
    }
  else if ( output_mux == SHFT_SHIFTER )
    {
      Reg->Shifter = reg.gp_reg;
    }
}
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
// mod 16 if C54CM=1 and mod 32 otherwise
// 0-3 = Tx
// 4-7 = Tx with rounding
// 8 = Immediate

// Bit is shifted out into Carry, it's clear for a shift by 0

// flag

// bit 0 : 0=shift bits 0-31 (sign bit at 31 of source)
// .     : 1=shift bits 0-39   M40=1 (sign bit at 39 of source)
// shifted out bit is from either bit above, or bit 0


// bit 1  : 1=sign extend from bit 0 (SXMD)
//       : if bit_0 = 0x0 (M40=0)
//            if bit_1 = 0x1 ( SXMD )
//               guard bits are replaced with bit 31 of source
//            else ( SXMD=0 )
//               guard bits are cleared


// bit 3-2 
// 00 (0x0) use SATD (accum) or SATA (16 bit reg) for saturation
// 01 (0x4) don't saturate
// 10 (0x8) saturate
// 11 (0xc) saturate to 00ffffffff
// overflow is detected at bit_0 (M40) above

// bit 5-4
// round (done with T shift?)
// round even-odd RDM flag
// 00 (0x0) dont round

// bit 6
// 0x40=set ACOVx on overflow using bit set with bit_0 above

// bit 7
// 0x80=don't shift out to CARRY  SHFT_DONT_SET_CARRY


// bit 1 = Round
// bit 2 = Unsigned. Turns on saturations and saturates to 00ffffffff
//                   Unless C54CM=1
// bit 3 = Saturate. Turn on Saturation regardless of SATD
// bit 4 = Don't set any ACOVx flags (Logical Shift)
