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

#include "shifter.h"

// mux (input control)
// 0 = A
// 1 = B
// 2 = DB
// 3 = CB
// (shift amount control)
// 0 = T
// 1 = ASM
// 2 = Immediate

// 0 = Low Bits to EB
// 1 = High Bits to EB

// SXM sign extension bit
void shifter2EBand_Store (int input_mux, int shift_mux, int output_mux, int SXM)
{

}

// storage_mux 
// 0, store in A
// not 0, store in B

// shift, amount to shift
void shifter2GPreg(int input_mux, int shift, int SXM, struct _Registers *Reg, int storage_mux)
{
  union _GP_Reg_Union reg;;
  
  reg.guint64 = 0;

  switch ( input_mux )
    {
    case 0:
      reg.gp_reg = MMR->A;
      break;
    case 1:
      reg.gp_reg = MMR->A;
      break;
    case 2:
      reg.words.low = Reg->DB;
      break;
    case 3:
      reg.words.low = Reg->CB;
      break;
    }

  if ( SXM )
    {
      // Sign extended
      reg.gint64 = reg.gint64 << shift;
    }
  else
    {
      reg.guint64 = reg.guint64 << shift;
    }


  if ( storage_mux )
    MMR->B =reg.gp_reg;
  else
    MMR->A =reg.gp_reg;
  return;
}
// shift, amount to shift
void shiftWord2GPreg(SWord input, int shift, int SXM, struct _Registers *Reg, int storage_mux)
{
  union _GP_Reg_Union reg;;
  
  reg.gint64 = input;

  if ( SXM )
    {
      // Sign extended
      reg.gint64 = reg.gint64 << shift;
    }
  else
    {
      reg.guint64 = reg.guint64 << shift;
    }


  if ( storage_mux )
    MMR->B =reg.gp_reg;
  else
    MMR->A =reg.gp_reg;
  return;
}
