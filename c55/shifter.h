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

#ifndef __SHIFTER_H__
#define __SHIFTER_H__

#include "chip_core.h"

#ifdef __cplusplus
extern "C" 
{
#endif

  /* input_mux (input control)
   *    0-15      Register 0-15
   *    SHFT_DB   DB Register
   *    SHFT_CB   CB Register
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
   *    SHFT_EB
   *
   */

#define SHFT_DB 16
#define SHFT_CB 17
#define SHFT_CONSTANT 8
#define SHFT_SHIFTER 16
#define SHFT_EB 17

  // Flag defines. Always use these

  // Saturation Control, one must be chosen
#define SHFT_DONT_SATURATE 0x3 // must be all bits for routine
#define SHFT_SATURATE 0x1
#define SHFT_UNSIGNED_SATURATE 0x2
#define SHFT_USE_SATD_SATA 0x0

  // overflow detection, one must be chosen
#define SHFT_SET_ACOV 0x40
#define SHFT_NO_OVERFLOW_DETECTION 0x0

  // M40, one must be chosen
#define SHFT_M40_IS_1 0x100 // must be non-zero and mutual-exclusive of below
#define SHFT_M40_IS_0 0x200 // must be non-zero and mutual-exclusive of above
#define SHFT_USE_M40 0x0

  // Sign, one must be chosen
#define SHFT_SIGN_EXTEND_USING_SXMD 0x0
#define SHFT_UNSIGNED 0x1


#define SHFT_DONT_SET_CARRY 0x80
#define SHFT_ROUND 0x40 // rounds according to RDM

inline void shifter(Word input_mux, Word shift_mux, SWord shift, 
		    unsigned int flag, Word output_mux, 
		    struct _Registers *Reg );

#ifdef __cplusplus
}
#endif

#endif //__SHIFTER_H__
