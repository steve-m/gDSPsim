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
#include "decode.h"
#include "memory.h"

// mux (input control)
// 0 = A
// 1 = B
// 2 = DB
// 3 = CB
// (shift amount control)
// 0 = T
// 1 = ASM
// 2 = Immediate
// 3 = ASM-16

// output_mux
// 0 = Store in A
// 1 = Store in B
// 2 = Low Bits to EB
// 3 = High Bits to EB
// 4 = Store to Shifter register

// SXM sign extension bit
inline void shifter(Word input_mux, struct _Registers *Reg, Word shift_mux, SWord shift, Word output_mux)
{
  union _GP_Reg_Union reg;
  
  reg.guint64 = 0;

  switch ( input_mux )
    {
    case 0:
      reg.gp_reg = MMR->A;
      break;
    case 1:
      reg.gp_reg = MMR->B;
      break;
    case 2:
      reg.words.low = Reg->DB;
      break;
    case 3:
      reg.words.low = Reg->CB;
      break;
    }

  switch ( shift_mux )
    {
    case 0:
      // Shift by T
      shift = MMR->T;
      break;
    case 1:
      shift = ASM(MMR);
      break;
    case 3:
      shift = ASM(MMR)-16;
      break;
    }

  if ( SXM(MMR) )
    {
      // Sign extended
      if ( shift < 0 )
	reg.gint64 = reg.gint64 >> -shift;
      else
	reg.gint64 = reg.gint64 << shift;
    }
  else
    {
      if ( shift < 0 )
	reg.gint64 = reg.guint64 >> -shift;
      else
	reg.gint64 = reg.guint64 << shift;
    }

  switch ( output_mux )
    {
    case 0:
      MMR->A = reg.gp_reg;
      break;
    case 1:
      MMR->B = reg.gp_reg;
      break;
    case 2:
      write_data_mem(Reg->EAB,reg.words.low);
      break;
    case 3:
      write_data_mem(Reg->EAB,reg.words.high);
    case 4:
      Reg->Shifter = reg.gp_reg;
    }
}

// storage_mux 
// 0, store in A
// not 0, store in B
// Rotate register. clears guard bits, rotates bit_in in, and returns
// bit rotated out
//
// imux (input)
// 0 = A
// 1 = B 
// direction
// 0 = rotate left
// 1 = rotate right
inline int rotate(int imux, int direction, int bit_in, struct _Registers *Reg)
{
  union _GP_Reg_Union reg;
  int bit_out;
  
  reg.guint64 = 0;

  if ( imux )
    reg.gp_reg = MMR->B;
  else
    reg.gp_reg = MMR->A;
  
  if ( direction )
    {
      bit_out = reg.gint64 & 0x1;
      reg.gint64 = reg.gint64 >> 1;
      reg.guint64 = reg.guint64 & 0xffffffff;
      reg.guint64 = reg.guint64 | (bit_in<<31);
    }
  else
    {
      bit_out = (reg.gint64 & 0x80000000)>>31;
      reg.gint64 = reg.gint64 << 1;
      reg.guint64 = reg.guint64 & 0xffffffff;
      reg.guint64 = reg.guint64 | bit_in;
    }

  if ( imux )
    MMR->B = reg.gp_reg;
  else
    MMR->A = reg.gp_reg;

  return bit_out;
}
