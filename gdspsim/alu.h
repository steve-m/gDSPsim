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

#ifndef __ALU_H__
#define __ALU_H__

#include "c54_core.h"

/* 
 * Xmux 0, X operand is from DB register
 *      1, X operand is from shifter register
 * Ymux 0, Y operand is from A register
 *      1, Y operand is from B register
 *      2, Y operand is from T register
 *      3, Y operand is from CB register
 * Omux 0, store answer in A
 *      1, store answer in B
 * flag bit 0, 0=add 1=Y-X
 */ 
void alu(int Xmux, int Ymux, int Omux, int flag, struct _Registers *Reg);

#endif // __ALU_H__
