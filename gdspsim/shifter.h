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

#ifndef __SHIFTER_H__
#define __SHIFTER_H__

#include "c54_core.h"


// mux (input control)
// 0 = A
// 1 = B
// 2 = DB
// 3 = CB
// (shift amount control)
// 0 = T
// 1 = ASM
// 2 = Immediate

// output_mux
// 0 = Store in A
// 1 = Store in B
// 2 = Low Bits to EB
// 3 = High Bits to EB

inline void shifter(Word input_mux, struct _Registers *Reg, Word shift_mux, SWord shift, Word output_mux);


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
#if 0
// SXM sign extension bit
void shifter2EB (int input_mux, int shift_mux, int output_mux, Word SXM);
// storage_mux 
// 0, store in A
// not 0, store in B

// shift, amount to shift
void shifter2GPreg(int input_mux, int shift, Word SXM, struct _Registers *Reg, Word storage_mux);
void shiftWord2GPreg(SWord input, int shift, Word SXM, struct _Registers *Reg, Word storage_mux);
void shifter2store (int input_mux, struct _Registers *Reg, int shift_mux, SWord shift, int output_mux, Word SXM);
#endif

// Rotate register. clears guard bits, rotates bit_in in, and returns
// bit rotated out
//
// imux (input)
// 0 = A
// 1 = B 
// direction
// 0 = rotate left
// 1 = rotate right
inline int rotate(int imux, int direction, int bit_in, struct _Registers *Reg);

#endif //__SHIFTER_H__
