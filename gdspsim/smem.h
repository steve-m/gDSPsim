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
// the read address onto DAB.
void smem_read_stg1(struct _PipeLine *pipeP, struct _Registers *Reg);
// like above, execept for memory mapped access
void mmem_read_stg1(struct _PipeLine *pipeP, struct _Registers *Reg);

// The basic read_stg1 for a smem. Handles 1 or 2 word smem. Places
// the read address onto DAB.
void smem_read_stg2(struct _PipeLine *pipeP, struct _Registers *Reg);
// like above, execept for memory mapped access
void mmem_read_stg2(struct _PipeLine *pipeP, struct _Registers *Reg);

// This updates the auxillary registers and returns the read address
Word update_smem( int Smem, struct _Registers *Reg);
Word update_smem_2words( int Smem, Word next_word, struct _Registers *Reg);
Word circular_update(Word start, SWord step, Word BK);
int num_words_for_smem(struct _PipeLine *pipeP);
int num_words_for_smem_plus1(struct _PipeLine *pipeP);
Word bit_reversal(Word start, Word bit_reversed_one, SWord adjustment);

#endif //  __SMEM_H__
