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
      if ( sign_extend )
	reg_union.gint64 = GP_REG_TO_INT64(MMR->AC0);
      else
	reg_union.guint64 = GP_REG_TO_UINT64(MMR->AC0);
      return reg_union;
    case 1:
      if ( sign_extend )
	reg_union.gint64 = GP_REG_TO_INT64(MMR->AC1);
      else
	reg_union.guint64 = GP_REG_TO_UINT64(MMR->AC1);
      return reg_union;
    case 2:
      if ( sign_extend )
	reg_union.gint64 = GP_REG_TO_INT64(MMR->AC2);
      else
	reg_union.guint64 = GP_REG_TO_UINT64(MMR->AC2);
      return reg_union;
    case 3:
      if ( sign_extend )
	reg_union.gint64 = GP_REG_TO_INT64(MMR->AC3);
      else
	reg_union.guint64 = GP_REG_TO_UINT64(MMR->AC3);
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


// returns a register, extends to 40 bits if _M40 is set
union _GP_Reg_Union get_register2(int r, int sign_extend, int _M40)
{
  union _GP_Reg_Union reg_union;

  reg_union.gint64 = 0;

  switch ( r )
    {
    case 0:
      if ( _M40 )
	{
	  if ( sign_extend )
	    reg_union.gint64 = GP_REG_TO_INT64(MMR->AC0);
	  else
	    reg_union.gint64 = GP_REG_TO_UINT64(MMR->AC0);
	}
      else
	{
	  if ( sign_extend )
	    reg_union.gint64 = GP_REG32_TO_INT64(MMR->AC0);
	  else
	    reg_union.gint64 = GP_REG32_TO_UINT64(MMR->AC0);
	}
      return reg_union;
    case 1:
      if ( _M40 )
	{
	  if ( sign_extend )
	    reg_union.gint64 = GP_REG_TO_INT64(MMR->AC1);
	  else
	    reg_union.gint64 = GP_REG_TO_UINT64(MMR->AC1);
	}
      else
	{
	  if ( sign_extend )
	    reg_union.gint64 = GP_REG32_TO_INT64(MMR->AC1);
	  else
	    reg_union.gint64 = GP_REG32_TO_UINT64(MMR->AC1);
	}
      return reg_union;
    case 2:
      if ( _M40 )
	{
	  if ( sign_extend )
	    reg_union.gint64 = GP_REG_TO_INT64(MMR->AC2);
	  else
	    reg_union.gint64 = GP_REG_TO_UINT64(MMR->AC2);
	  return reg_union;
	}
      else
	{
	  if ( sign_extend )
	    reg_union.gint64 = GP_REG32_TO_INT64(MMR->AC2);
	  else
	    reg_union.gint64 = GP_REG32_TO_UINT64(MMR->AC2);
	}
    case 3:
      if ( _M40 )
	{
	  if ( sign_extend )
	    reg_union.gint64 = GP_REG_TO_INT64(MMR->AC3);
	  else
	    reg_union.gint64 = GP_REG_TO_UINT64(MMR->AC3);
	}
      else
	{
	  if ( sign_extend )
	    reg_union.gint64 = GP_REG32_TO_INT64(MMR->AC3);
	  else
	    reg_union.gint64 = GP_REG32_TO_UINT64(MMR->AC3);
	}
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

// Set the low 16 bits of a register
void set_k16_regLO(int r, Word value)
{
  switch ( r )
    {
    case 0:
      MMR->AC0.wgp.word0 = value;
      return;
    case 1:
      MMR->AC1.wgp.word0 = value;
      return;
    case 2:
      MMR->AC2.wgp.word0 = value;
      return;
    case 3:
      MMR->AC3.wgp.word0 = value;
      return;
    case 4:
      MMR->T0 = value;
      return;
    case 5:
      MMR->T1 = value;
      return;
    case 6:
      MMR->T2 = value;
      return;
    case 7:
      MMR->T3 = value;
      return;
    case 8:
      MMR->ar0 = value;
      return;
    case 9:
      MMR->ar1 = value;
      return;
    case 10:
      MMR->ar2 = value;
      return;
    case 11:
      MMR->ar3 = value;
      return;
    case 12:
      MMR->ar4 = value;
      return;
    case 13:
      MMR->ar5 = value;
      return;
    case 14:
      MMR->ar6 = value;
      return;
    case 15:
      MMR->ar7 = value;
      return;
    }
}

// Set the low 16 bits of a register
void set_k16_regHI(int r, Word value)
{
  switch ( r )
    {
    case 0:
      MMR->AC0.wgp.word1 = value;
      return;
    case 1:
      MMR->AC1.wgp.word1 = value;
      return;
    case 2:
      MMR->AC2.wgp.word1 = value;
      return;
    case 3:
      MMR->AC3.wgp.word1 = value;
      return;
    case 4:
      MMR->T0 = value;
      return;
    case 5:
      MMR->T1 = value;
      return;
    case 6:
      MMR->T2 = value;
      return;
    case 7:
      MMR->T3 = value;
      return;
    case 8:
      MMR->ar0 = value;
      return;
    case 9:
      MMR->ar1 = value;
      return;
    case 10:
      MMR->ar2 = value;
      return;
    case 11:
      MMR->ar3 = value;
      return;
    case 12:
      MMR->ar4 = value;
      return;
    case 13:
      MMR->ar5 = value;
      return;
    case 14:
      MMR->ar6 = value;
      return;
    case 15:
      MMR->ar7 = value;
      return;
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

  if ( r < 4 )
    {
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
    }
  else
    {
      // 16-bit register
      if ( reg_union.gint64 > 0x7fff )
	{
	  if ( _SATD & 1 )
	    reg_union.gint64 = 0x7fff;
	  saturated=1;
	}
      else if ( reg_union.gint64 > -32768 )
	{
	  if ( _SATD & 1 )
	    reg_union.gint64 = -32768;
	  saturated=1;
	}  
    }

  if ( _SATD & 2 )
    set_register(reg_union,r);

  return reg_union;
}

// returns 0 if condition is false and non-zero if true
int check_condition(unsigned char cond, struct _Registers *Reg)
{
  union _GP_Reg_Union reg_union;
  int top;

  top = (cond>>4)&7;

  if ( top < 6 )
    {
      reg_union = get_register2(cond&0xf, 1, M40(MMR));

      switch ( top )
	{
	case 0:
	  if ( reg_union.gint64 == 0 )
	    return 1;
	  else
	    return 0;
	case 1:
	  if ( reg_union.gint64 == 0 )
	    return 0;
	  else
	    return 1;
	case 2:
	  if ( reg_union.gint64 < 0 )
	    return 1;
	  else
	    return 0;
	case 3:
	  if ( reg_union.gint64 <= 0 )
	    return 1;
	  else
	    return 0;
	case 4:
	  if ( reg_union.gint64 > 0 )
	    return 1;
	  else
	    return 0;
	case 5:
	  if ( reg_union.gint64 >= 0 )
	    return 1;
	  else
	    return 0;
	}
    }
  else if ( top == 6 )
    {
      switch ( cond & 0xf )
	{
	case 0:
	  return ACOV0(MMR);
	case 1:
	  return ACOV1(MMR);
	case 2:
	  return ACOV2(MMR);
	case 3:
	  return ACOV3(MMR);
	case 4:
	  return TC1(MMR);
	case 5:
	  return TC2(MMR);
	case 6:
	  return CARRY(MMR);
	case 8:
	  return TC1(MMR)&TC2(MMR);
	case 9:
	  return TC1(MMR) & !TC2(MMR);
	case 10:
	  return !TC1(MMR) & TC2(MMR);
	case 11:
	  return !TC1(MMR) & !TC2(MMR);
	default:
	  printf("Bad call %s:%d\n",__FILE__,__LINE__);
	  return 0;
	}
    }
  else if ( top == 7 )
    {
      switch ( cond & 0xf )
	{
	case 0:
	  return !ACOV0(MMR);
	case 1:
	  return !ACOV1(MMR);
	case 2:
	  return !ACOV2(MMR);
	case 3:
	  return !ACOV3(MMR);
	case 4:
	  return !TC1(MMR);
	case 5:
	  return !TC2(MMR);
	case 6:
	  return !CARRY(MMR);
	  // case 7 reserved, caught with default
	case 8:
	  return TC1(MMR) | TC2(MMR);
	case 9:
	  return TC1(MMR) | !TC2(MMR);
	case 10:
	  return !TC1(MMR) | TC2(MMR);
	case 11:
	  return !TC1(MMR) | !TC2(MMR);
	case 12:
	  return TC1(MMR) ^ TC2(MMR);
	case 13:
	  return TC1(MMR) ^ !TC2(MMR);
	case 14:
	  return !TC1(MMR) ^ TC2(MMR);
	case 15:
	  return !TC1(MMR) ^ !TC2(MMR);
	default:
	  printf("Bad call %s:%d\n",__FILE__,__LINE__);
	  return 0;
	}
    }
  else
    {
      printf("Bad call %s:%d\n",__FILE__,__LINE__);
      return 0;
    }

  return 0;
}

WordA get_extended_reg(int xr, struct _Registers *Reg)
{
  WordA ab;

  switch ( xr )
    {
    case 0:
      ab = MMR->AC0.worda;
      return ab;
    case 1:
      ab = MMR->AC0.worda;
      return ab;
    case 2:
      ab = MMR->AC0.worda;
      return ab;
    case 3:
      ab = MMR->AC0.worda;
      return ab;
    case 4:
      ab = ((WordA)(MMR->SPH & 0xff))<<16 | (WordA)MMR->SP;
      return ab;
    case 6:
      ab = ((WordA)(MMR->CDPH & 0xff))<<16 | (WordA)MMR->CDP;
      return ab;
    case 7:
      ab = ((WordA)(MMR->DPH & 0xff))<<16 | (WordA)MMR->DP;
      return ab;
    case 8:
      ab = ((WordA)(Reg->AR0H & 0xff))<<16 | (WordA)MMR->ar0;
      return ab;
    case 9:
      ab = ((WordA)(Reg->AR1H & 0xff))<<16 | (WordA)MMR->ar1;
      return ab;
    case 10:
      ab = ((WordA)(Reg->AR2H & 0xff))<<16 | (WordA)MMR->ar2;
      return ab;
    case 11:
      ab = ((WordA)(Reg->AR3H & 0xff))<<16 | (WordA)MMR->ar3;
      return ab;
    case 12:
      ab = ((WordA)(Reg->AR4H & 0xff))<<16 | (WordA)MMR->ar4;
      return ab;
    case 13:
      ab = ((WordA)(Reg->AR5H & 0xff))<<16 | (WordA)MMR->ar5;
      return ab;
    case 14:
      ab = ((WordA)(Reg->AR6H & 0xff))<<16 | (WordA)MMR->ar6;
      return ab;
    case 15:
      ab = ((WordA)(Reg->AR7H & 0xff))<<16 | (WordA)MMR->ar7;
      return ab;
    default:
      printf("Error! %s:%d\n",__FILE__,__LINE__);
      return 0;
    }
  
}

// Sets extended registers
void set_extended_reg(WordA value, int xr, struct _Registers *Reg)
{
  switch ( xr )
    {
    case 0:
      MMR->AC0.bgp.byte4 = 0;
      MMR->AC0.worda = value;
      break;
    case 1:
      MMR->AC1.bgp.byte4 = 0;
      MMR->AC1.worda = value;
      break;
    case 2:
      MMR->AC2.bgp.byte4 = 0;
      MMR->AC2.worda = value;
      break;
    case 3:
      MMR->AC3.bgp.byte4 = 0;
      MMR->AC3.worda = value;
      break;
    case 4:
      MMR->SP = (Word)value;
      MMR->SPH = (Word)(value>>16);
      break;
    case 6:
      MMR->CDP = (Word)value;
      MMR->CDPH = (Word)(value>>16);
      break;
    case 7:
      MMR->DP = (Word)value;
      MMR->DPH = (Word)(value>>16);
      break;
    case 8:
      MMR->ar0 = (Word)value;
      Reg->AR0H = (Word)(value>>16);
      break;
    case 9:
      MMR->ar1 = (Word)value;
      Reg->AR1H = (Word)(value>>16);
      break;
    case 10:
      MMR->ar2 = (Word)value;
      Reg->AR2H = (Word)(value>>16);
      break;
    case 11:
      MMR->ar3 = (Word)value;
      Reg->AR3H = (Word)(value>>16);
      break;
    case 12:
      MMR->ar4 = (Word)value;
      Reg->AR4H = (Word)(value>>16);
      break;
    case 13:
      MMR->ar5 = (Word)value;
      Reg->AR5H = (Word)(value>>16);
      break;
    case 14:
      MMR->ar6 = (Word)value;
      Reg->AR6H = (Word)(value>>16);
      break;
    case 15:
      MMR->ar7 = (Word)value;
      Reg->AR7H = (Word)(value>>16);
      break;
    }
}
