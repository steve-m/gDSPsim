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

#ifndef __MULTIPLIER_H__
#define __MULTIPLIER_H__

#include <chip_core.h>

#ifdef __cplusplus
extern "C" 
{
#endif

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
 * Smux 0, Store in A register
 *      1, Store in B register
 *      2, Store in A register with Rounding
 *      3, Store in B register with Rounding
 */

// See page 4-20 (108) Vol1
/* Multiplies 17-bits by 17-bits and returns 40-bits.
 * For signed each 16-bit memory is signed extended, 
 * for unsigned, 0 is added to the MSB.
 */

void multiplier(int Xmux, int Ymux, int Amux, int Smux, struct _Registers *Reg);

#ifdef __cplusplus
}
#endif

#endif // __MULTIPLIER_H__
