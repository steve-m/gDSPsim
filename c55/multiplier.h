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

  // Inputs
  // Reg 0-7 ACx, Tx,  ACx are bits [32-16] others are sign extended
#define MULT_DB 8
#define MULT_CB 9
#define MULT_BB 10
#define MULT_P 11

  // MacMux
  // Reg 0-3 ACx
#define MULT_NO_MAC 12

  // Outputs
  // Reg 0-3 ACx

  // Flags
#define MULT_UNSIGNED_X 1
#define MULT_UNSIGNED_Y 32
#define MULT_SUBTRACT 4
#define MULT_ROUND 8
#define MULT_M40_IS_1 16
#define MULT_RIGHT_SHIFT_MAC 64


void multiplier(int Xmux, int Ymux, int MacMux, int Omux, int flag, struct _Registers *Reg);

#ifdef __cplusplus
}
#endif

#endif // __MULTIPLIER_H__
