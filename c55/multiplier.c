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
#include <chip_help.h>

/*
 * Multiplies Xmux variable by Ymux variable adds it to MacMux
 * variable  and stores result in Omux variable
 *
 * Xmux,Ymux        Register
 *  0-3           ACx bits 32-16
 *  4-7           Tx
 *  MULT_DB       DB
 *  MULT_CB       CB
 *  MULT_BB       BB
 *  MULT_P        P
 * 
 * MacMux
 *  0-3           ACx bits 32-16
 *  MULT_NO_MAC   Don't accumulate
 *
 * Omux
 *  0-3           ACx
 *
 * flags
 * MULT_UNSIGNED_X  X value is unsigned
 * MULT_UNSIGNED_Y  Y value is unsigned
 * MULT_SUBTRACT    When accumulating subtract
 * MULT_M40_IS_1    Act like M40 is 1
 * MULT_ROUND       Round result using RDM flag for type
 * MULT_RIGHT_SHIFT_MAC  Right Shift Accumulator by 16 before adding
 *
 * Default actions
 * Result is left shifted by 1 if FRCT set
 * Overflow is detected is SMUL set
 * Overflow value depends upon M40
 * if C54CM is set, M40 is locally set to 0
 * Value is saturated if SATD set
 */

void multiplier(int Xmux, int Ymux, int MacMux, int Omux, int flag, struct _Registers *Reg)
{
  gint32 X,Y; // Really need 17-bits
  union _GP_Reg_Union reg_union,reg_union2;
  int saturated=0;

  if ( flag & MULT_UNSIGNED_X )
    {
      switch (Xmux)
	{
	case 0:
	  X = MMR->AC0.wgp.word1;
	  break;
	case 1:
	  X = MMR->AC1.wgp.word1;
	  break;
	case 2:
	  X = MMR->AC2.wgp.word1;
	  break;
	case 3:
	  X = MMR->AC3.wgp.word1;
	  break;
	case 4:
	  X = MMR->T0;
	  break;
	case 5:
	  X = MMR->T1;
	  break;
	case 6:
	  X = MMR->T2;
	  break;
	case 7:
	  X = MMR->T3;
	  break;
	case MULT_DB:
	  X = Reg->DB;
	  break;
	case MULT_CB:
	  X = Reg->CB;
	  break;
	case MULT_BB:
	  X = Reg->BB;
	  break;
	case MULT_P:
	  X = Reg->P;
	  break;
	default:
	  FIXME();
	  break;
	}
    }
  else
    {
      // Signed
      switch (Xmux)
	{
	case 0:
	  X = GP_REG17_TO_INT32(MMR->AC0);
	  break;
	case 1:
	  X = GP_REG17_TO_INT32(MMR->AC1);
	  break;
	case 2:
	  X = GP_REG17_TO_INT32(MMR->AC2);
	  break;
	case 3:
	  X = GP_REG17_TO_INT32(MMR->AC3);
	  break;
	case 4:
	  X = (SWord)MMR->T0;
	  break;
	case 5:
	  X = (SWord)MMR->T1;
	  break;
	case 6:
	  X = (SWord)MMR->T2;
	  break;
	case 7:
	  X = (SWord)MMR->T3;
	  break;
	case MULT_DB:
	  X = (SWord)Reg->DB;
	  break;
	case MULT_CB:
	  X = (SWord)Reg->CB;
	  break;
	case MULT_BB:
	  X = (SWord)Reg->BB;
	  break;
	case MULT_P:
	  X = (SWord)Reg->P;
	  break;
	default:
	  FIXME();
	  break;
	}
    }

  if ( flag & MULT_UNSIGNED_Y )
    {
      switch (Ymux)
	{
	case 0:
	  Y = MMR->AC0.wgp.word1;
	  break;
	case 1:
	  Y = MMR->AC1.wgp.word1;
	  break;
	case 2:
	  Y = MMR->AC2.wgp.word1;
	  break;
	case 3:
	  Y = MMR->AC3.wgp.word1;
	  break;
	case 4:
	  Y = MMR->T0;
	  break;
	case 5:
	  Y = MMR->T1;
	  break;
	case 6:
	  Y = MMR->T2;
	  break;
	case 7:
	  Y = MMR->T3;
	  break;
	case MULT_DB:
	  Y = Reg->DB;
	  break;
	case MULT_CB:
	  Y = Reg->CB;
	  break;
	case MULT_BB:
	  Y = Reg->BB;
	  break;
	case MULT_P:
	  Y = Reg->P;
	  break;
	default:
	  FIXME();
	  break;
	}
    }
  else
    {
      // Signed
      switch (Ymux)
	{
	case 0:
	  Y = GP_REG17_TO_INT32(MMR->AC0);
	  break;
	case 1:
	  Y = GP_REG17_TO_INT32(MMR->AC1);
	  break;
	case 2:
	  Y = GP_REG17_TO_INT32(MMR->AC2);
	  break;
	case 3:
	  Y = GP_REG17_TO_INT32(MMR->AC3);
	  break;
	case 4:
	  Y = (SWord)MMR->T0;
	  break;
	case 5:
	  Y = (SWord)MMR->T1;
	  break;
	case 6:
	  Y = (SWord)MMR->T2;
	  break;
	case 7:
	  Y = (SWord)MMR->T3;
	  break;
	case MULT_DB:
	  Y = (SWord)Reg->DB;
	  break;
	case MULT_CB:
	  Y = (SWord)Reg->CB;
	  break;
	case MULT_BB:
	  Y = (SWord)Reg->BB;
	  break;
	case MULT_P:
	  Y = (SWord)Reg->P;
	  break;
	default:
	  FIXME();
	  break;
	}
    }


  // Now multiply X*Y in 17 bits to get 40 bits
  reg_union.gint64 = X*Y;

  if ( FRCT(MMR) )
    reg_union.gint64 = reg_union.gint64 << 1;

  // Accumulate
  if ( MacMux != MULT_NO_MAC )
    {
      switch ( MacMux )
	{
	case 0:
	  reg_union2.gint64 = GP_REG_TO_INT64(MMR->AC0);
	  break;
	case 1:
	  reg_union2.gint64 = GP_REG_TO_INT64(MMR->AC1);
	  break;
	case 2:
	  reg_union2.gint64 = GP_REG_TO_INT64(MMR->AC2);
	  break;
	case 3:
	  reg_union2.gint64 = GP_REG_TO_INT64(MMR->AC3);
	  break;
	default:
	  // error in argument
	  FIXME();
	  return;
	  break;
	}
      if ( flag & MULT_RIGHT_SHIFT_MAC )
	{
	  reg_union2.gint64 = reg_union2.gint64 >> 16;
	}
      if ( flag & MULT_SUBTRACT )
	{
	  reg_union.gint64 -= reg_union2.gint64;
	}
      else
	{
	  reg_union.gint64 += reg_union2.gint64;
	}
    }

  if ( flag & MULT_ROUND )
    {
      // Rounding
      if ( RDM(MMR) && C54CM(MMR)==0 )
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
  if ( SMUL(MMR) || SATD(MMR) )
    {
      // check for saturation before Accumulation
      if ( (flag & MULT_M40_IS_1) || M40(MMR) )
	{
	  if ( reg_union.gint64 > max_pos40 )
	    {
	      if ( SATD(MMR) )
		reg_union.gint64 = max_pos40;
	      saturated=1;
	    }
	  if ( reg_union.gint64 < max_neg40 )
	    {
	      if ( SATD(MMR) )
		reg_union.gint64 = max_neg40;
	      saturated=1;
	    }
	}
      else
	{
	  if ( reg_union.gint64 > max_pos32 )
	    {
	      if ( SATD(MMR) )
		reg_union.gint64 = max_pos32;
	      saturated=1;
	    }
	  if ( reg_union.gint64 < max_neg32 )
	    {
	      if ( SATD(MMR) )
		reg_union.gint64 = max_neg32;
	      saturated=1;
	    }
	}
    }
 
  switch ( Omux )
    {
    case 0:
      MMR->AC0 = reg_union.gp_reg;
      if ( saturated && SMUL(MMR) )
	set_ACOV0(MMR,1);
      break;
    case 1:
      MMR->AC1 = reg_union.gp_reg;
      if ( saturated && SMUL(MMR) )
	set_ACOV1(MMR,1);
      break;
    case 2:
      MMR->AC2 = reg_union.gp_reg;
      if ( saturated && SMUL(MMR) )
	set_ACOV2(MMR,1);
      break;
    case 3:
      MMR->AC3 = reg_union.gp_reg;
      if ( saturated && SMUL(MMR) )
	set_ACOV3(MMR,1);
      break;
    }

}
