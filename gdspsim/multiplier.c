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



/* Xmux 0, X operand is from T register, signed
 *      1, X operand is from DB register
 *      2, X operand is from A[32:16] register
 *      3, X operand is from DB register, unsigned
 * Ymux 0, Y operand is from P register
 *      1, Y operand is from DB register
 *      2, Y operand is from A[32:16] register
 *      3, Y operand is from CB register
 *      4, Y operand is from DB register, unsigned
 * Amux 0, Accumulate using 0
 *      1, Accumulate using A
 *      2, Accumulate using B
 *      3, Accumulate by subtracting from A
 *      4, Accumulate by subtracting from B
 * Smux 0, Store in A register
 *      1, Store in B register
 *      2, Store in A register with Rounding
 *      3, Store in B register with Rounding
 */

#include "multiplier.h"
#include <stdlib.h>
#include <stdio.h>

// See page 4-20 (108) Vol1
/* Multiplies 17-bits by 17-bits and returns 40-bits.
 * For signed each 16-bit memory is signed extended, 
 * for unsigned, 0 is added to the MSB.
 */
void multiplier(int Xmux, int Ymux, int Amux, int Smux, struct _Registers *Reg)
{
  gint32 X,Y; // Really need 17-bits
  gint64 Result64;
  union _GP_Reg_Union gp_reg;

  switch (Xmux)
    {
    case 0:
      X = MMR->T;
      break;
    case 1:
      X = Reg->DB;
      break;
    case 2:
      printf("fixme %s:%d\n",__FILE__,__LINE__);
    case 3:
      X = Reg->DB;
      X = abs(X);
      break;
    }

  switch (Ymux)
    {
    case 0:
      Y = Reg->P;
      break;
    case 1:
      Y = Reg->DB;
      break;
    case 2:
      printf("fixme %s:%d\n",__FILE__,__LINE__);
      break;
    case 3:
      Y = Reg->CB;
      break;
    case 4:
      Y = Reg->DB;
      Y = abs(Y);
      break;
    }

  // Now multiply X*Y in 17 bits to get 40 bits
  Result64 = X*Y;

  // Now add 40 bits to 40 bits
  if ( Amux == 1 )
    {
      // Accumulate using A register
      gp_reg.gp_reg = MMR->A;
      Result64 = Result64 + gp_reg.gint64;
    }
  else if ( Amux == 2 )
    {
      // Accumulate using B register
      gp_reg.gp_reg = MMR->B;
      Result64 = Result64 + gp_reg.gint64;
    }
  else if ( Amux == 3 )
    {
      // Accumulate using A register
      gp_reg.gp_reg = MMR->A;
      Result64 = gp_reg.gint64 - Result64;
    }
  else if ( Amux == 4 )
    {
      // Accumulate using B register
      gp_reg.gp_reg = MMR->B;
      Result64 = gp_reg.gint64 - Result64;
    }
 

  gp_reg.gint64 = Result64;

  if ( Smux )
    {
      // Store in B register
      MMR->B = gp_reg.gp_reg;
    }
  else
    {
      MMR->A = gp_reg.gp_reg;
    }
}
