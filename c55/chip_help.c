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

#include <chip_help.h>

// returns a register value AC[0-3]
union _GP_Reg_Union get_register(int r, int sign_extend)
{
  union _GP_Reg_Union reg_union;

  reg_union.gint64 = 0;

  switch ( r )
    {
    case 0:
      //fixme, sign extend to 64 bits
      reg_union.gp_reg = MMR->AC0;
      return reg_union;
    case 1:
      reg_union.gp_reg = MMR->AC1;
      return reg_union;
    case 2:
      reg_union.gp_reg = MMR->AC2;
      return reg_union;
    case 3:
      reg_union.gp_reg = MMR->AC3;
      return reg_union;
    case 4:
      if ( sign_extend )
	reg_union.gint64 = (SWord)MMR->T0;
      else
	reg_union.guint64 = MMR->T0;
      return reg_union;
    case 5:
      if ( sign_extend )
	reg_union.gint64 = (SWord)MMR->T1;
      else
	reg_union.guint64 = MMR->T1;
      return reg_union;
    case 6:
      if ( sign_extend )
	reg_union.gint64 = (SWord)MMR->T2;
      else
	reg_union.guint64 = MMR->T2;
      return reg_union;
    case 7:
      if ( sign_extend )
	reg_union.gint64 = (SWord)MMR->T3;
      else
	reg_union.guint64 = MMR->T3;
      return reg_union;
    case 8:
      if ( sign_extend )
	reg_union.gint64 = (SWord)MMR->ar0;
      else
	reg_union.guint64 = MMR->ar0;
      return reg_union;
    case 9:
      if ( sign_extend )
	reg_union.gint64 = (SWord)MMR->ar1;
      else
	reg_union.guint64 = MMR->ar1;
      return reg_union;
    case 10:
      if ( sign_extend )
	reg_union.gint64 = (SWord)MMR->ar2;
      else
	reg_union.guint64 = MMR->ar2;
      return reg_union;
    case 11:
      if ( sign_extend )
	reg_union.gint64 = (SWord)MMR->ar3;
      else
	reg_union.guint64 = MMR->ar3;
      return reg_union;
    case 12:
      if ( sign_extend )
	reg_union.gint64 = (SWord)MMR->ar4;
      else
	reg_union.guint64 = MMR->ar4;
      return reg_union;
    case 13:
      if ( sign_extend )
	reg_union.gint64 = (SWord)MMR->ar5;
      else
	reg_union.guint64 = MMR->ar5;
      return reg_union;
    case 14:
      if ( sign_extend )
	reg_union.gint64 = (SWord)MMR->ar6;
      else
	reg_union.guint64 = MMR->ar6;
      return reg_union;
    case 15:
      if ( sign_extend )
	reg_union.gint64 = (SWord)MMR->ar7;
      else
	reg_union.guint64 = MMR->ar7;
      return reg_union;
    default:
      printf("Error in call to get_register %s:%d\n",__FILE__,__LINE__);
      reg_union.gint64 = 0;
      return reg_union;
    }
}

// Set ACx (r=0-15). No flags or saturation set. Uses LSB for
// 16 bit registers
void set_register(union _GP_Reg_Union reg_union, int r)
{
  switch ( r )
    {
    case 0:
      MMR->AC0 = reg_union.gp_reg;
      return;
    case 1:
      MMR->AC0 = reg_union.gp_reg;
      return;
    case 2:
      MMR->AC0 = reg_union.gp_reg;
      return;
    case 3:
      MMR->AC0 = reg_union.gp_reg;
      return;
    case 4:
      MMR->T0 = reg_union.words.low;
      return;
    case 5:
      MMR->T1 = reg_union.words.low;
      return;
    case 6:
      MMR->T2 = reg_union.words.low;
      return;
    case 7:
      MMR->T3 = reg_union.words.low;
      return;

    case 8:
      MMR->ar0 = reg_union.words.low;
      return;
    case 9:
      MMR->ar1 = reg_union.words.low;
      return;
    case 10:
      MMR->ar2 = reg_union.words.low;
      return;
    case 11:
      MMR->ar3 = reg_union.words.low;
      return;
    case 12:
      MMR->ar4 = reg_union.words.low;
      return;
    case 13:
      MMR->ar5 = reg_union.words.low;
      return;
    case 14:
      MMR->ar6 = reg_union.words.low;
      return;
    case 15:
      MMR->ar7 = reg_union.words.low;
      return;

    default:
      printf("Error!! bad call to set_register %s:%d\n",__FILE__,__LINE__);
      return;
    }
}

// Set ACOV[0-3] (r=0-3) if reg_union is saturated. Saturation
// is 40 bits if M40 (read inside function) is set and 32 bits
// if M40 is not set. Set register ACx (r=0-3) to saturated value if sat_bit
// is non-zero or to reg_union if sat_bit is zero.
void set_reg_saturate(union _GP_Reg_Union reg_union, int r, int sat_bit)
{
  int saturated=0;

  if ( M40(MMR) )
    {
      if ( reg_union.gint64 > max_pos40 )
	{
	  if ( sat_bit )
	    reg_union.gint64 = max_pos40;
	  saturated=1;
	}
      if ( reg_union.gint64 < max_neg40 )
	{
	  if ( sat_bit )
	    reg_union.gint64 = max_neg40;
	  saturated=1;
	}
    }
  else
    {
      if ( reg_union.gint64 > max_pos32 )
	{
	  if ( sat_bit )
	    reg_union.gint64 = max_pos32;
	  saturated=1;
	}
      if ( reg_union.gint64 < max_neg32 )
	{
	  if ( sat_bit )
	    reg_union.gint64 = max_neg32;
	  saturated=1;
	}
    }

  switch ( r )
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
  return;
}

// Set ACOV[0-3] (r=0-3) if reg_union is saturated. Saturation is
// 0x80 0000 0000 and 0x7f ffff ffff if _M40=1 and 
// 0xff 8000 0000 and 0x00 7fff ffff if _M40=0.
// Saturated value is returned if _SATD=1,3 and reg_union is _SATD=0,2
// ACx (r=0,3) is set to Saturated value is _SATD=2,3
union _GP_Reg_Union saturate(union _GP_Reg_Union reg_union, int r, int _M40, 
			     int _SATD)
{
  int saturated=0;

  if ( _M40 )
    {
      if ( reg_union.gint64 > max_pos40 )
	{
	  if ( _SATD & 1 )
	    reg_union.gint64 = max_pos40;
	  saturated=1;
	}
      if ( reg_union.gint64 < max_neg40 )
	{
	  if ( _SATD & 1 )
	    reg_union.gint64 = max_neg40;
	  saturated=1;
	}
    }
  else
    {
      if ( reg_union.gint64 > max_pos32 )
	{
	  if ( _SATD & 1 )
	    reg_union.gint64 = max_pos32;
	  saturated=1;
	}
      if ( reg_union.gint64 < max_neg32 )
	{
	  if ( _SATD & 1 )
	    reg_union.gint64 = max_neg32;
	  saturated=1;
	}
    }

  if ( saturated )
    {
      switch ( r )
	{
	case 0:
	  set_ACOV0(MMR,1);
	  break;
	case 1:
	  set_ACOV1(MMR,1);
	  break;
	case 2:
	  set_ACOV2(MMR,1);
	  break;
	case 3:
	  set_ACOV3(MMR,1);
	  break;
	}
    }

  if ( _SATD & 2 )
    set_register(reg_union,r);

  return reg_union;
}
