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
extern "C" {
#endif

// mux (input control)
// 0 = AC0
// 1 = AC1
// 2 = AC2
// 3 = AC3
// 4 = T0
// 5 = T1
// 6 = T2
// 7 = T3
// 8-15 = ARx

// (shift amount control)
// 0 = Immediate

// output_mux
// 0 = AC0
// 1 = AC1
// 2 = AC2
// 3 = AC3
// 4 = T0
// 5 = T1
// 6 = T2
// 7 = T3
// 8-15 = ARx

// SXM sign extension bit
inline void shifter(Word input_mux, Word shift_mux, SWord shift, 
		    Word output_mux, struct _Registers *Reg );

#ifdef __cplusplus
}
#endif

#endif //__SHIFTER_H__
