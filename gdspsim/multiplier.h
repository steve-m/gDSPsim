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
extern "C" {
#endif

  /* Xmux 0-3 ACx[32:16]
   *      4 DB
   *      5 CB
   *      6 BB
   *      7 P
   * Ymux same as above
   * Amux 0-3, Accumulate using ACx
   *      4-7, Accumulate by subtracting from ACx
   *      8, Don't Accumulate
   * Smux 0-3, Store in ACx register
   *      4-7, Store in ACx register with Rounding
   */
  void multiplier(int Xmux, int Ymux, int Amux, int Smux, struct _Registers *Reg);

#ifdef __cplusplus
}
#endif

#endif // __MULTIPLIER_H__
