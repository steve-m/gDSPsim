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

#ifndef __SMEM_C55_H__
#define __SMEM_C55_H__

#ifdef __cplusplus
extern "C" {
#endif

void smem_set_DAB(int p, int mod);

Word bit_reversal_add(Word start, Word bit_reversed_one);

// r register number 0-3
void set_s16_ACx(int r, SWord value);

// p=0-7
Word *get_pointer_reg(int p);

#ifdef __cplusplus
}
#endif

#endif // __SMEM_C55_H__
