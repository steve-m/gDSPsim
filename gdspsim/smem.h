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

#ifndef __SMEM_H__
#define __SMEM_H__

#include "c54_core.h"

// The basic read_stg1 for a smem. Handles 1 or 2 word smem. Places
// the read address onto DAB. It decodes the address and adjusts any
// pointer registers if needed. 
void smem_read_stg1(struct _PipeLine *pipeP, struct _Registers *Reg);
// like above, execept for memory mapped access
void mmem_read_stg1(struct _PipeLine *pipeP, struct _Registers *Reg);
// puts first address into DAB and second address into CAB
void lmem_read_stg1(struct _PipeLine *pipeP, struct _Registers *Reg);
void smem_read_stg1_plus1(struct _PipeLine *pipeP, struct _Registers *Reg);

// The basic read_stg1 for a smem. Handles 1 or 2 word smem. Places
// the read address onto DAB.
void smem_read_stg2(struct _PipeLine *pipeP, struct _Registers *Reg);
// like above, execept for memory mapped access
void mmem_read_stg2(struct _PipeLine *pipeP, struct _Registers *Reg);
// Reads Lmem1 and Lmem2 addresses into Shifter
void lmem_read_stg2(struct _PipeLine *pipeP, struct _Registers *Reg);

// Similar to mmem_read_stg1 except it sets the EAB register instead
// of the DAB
void mmem_set_EAB(struct _PipeLine *pipeP, struct _Registers *Reg);
// Similar to smem_read_stg1 except it sets the EAB register instead
// of the DAB
void smem_set_EAB(struct _PipeLine *pipeP, struct _Registers *Reg);

// This updates the auxillary registers and returns the read address
// Generally, the CPL bit or equal 2 for memory mapped registers.
// Ref is only used for direct addressing (Smem<0x80). If Ref = 0, then the
// DP (data page pointer) times 0x80 is added to the address. If Ref = 1, then
// SP (stack pointer) is added to the address. If Ref=2, then nothing is added. 
Word update_smem( int Smem, struct _Registers *Reg, int Ref);
Word update_smem_2words( int Smem, Word next_word, struct _Registers *Reg);
Word circular_update(Word start, SWord step, Word BK);
int num_words_for_smem(struct _PipeLine *pipeP);
int num_words_for_smem_plus1(struct _PipeLine *pipeP);
Word bit_reversal(Word start, Word bit_reversed_one, SWord adjustment);

#endif //  __SMEM_H__
