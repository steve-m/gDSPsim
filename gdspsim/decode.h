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

#include "c54_core.h"

#ifndef __DECODE_H__
#define __DECODE_H__

extern struct _Registers *Registers;

union u_operands read_op(char info, struct _Registers *Registers, Word bits, Word lk, int *wait_state);

// Returns true if the opcode matches the mask
int check_mask(const char *mask, Word mach_code );

gchar *a_decode(gchar *mask, char info, Word start_code, WordA *location);
gchar *mw_decode(gchar *mask, char info, Word start_code, WordA *location);
gchar *r_decode(gchar *mask, char info, Word start_code, WordA *location);
gchar *z_decode(gchar *mask, char info, Word start_code, WordA *location);
gchar *sd_decode(gchar *mask, char info, Word start_code, WordA *location);
gchar *xy_decode(gchar *mask, char info, Word start_code, WordA *location);
gchar *u_decode(gchar *mask, char info, Word start_code, WordA *location);
gchar *n_decode(gchar *mask, char info, Word start_code, WordA *location);
// Extracts the signed number from data by matching info to the mask.
// It will read more program memory if needed.
SWord signed_bit_extract(char info, char *mask, Word mach_code, WordA *location);

// determine number of words this Smem addressing takes up, 1 or 2
int num_words_for_smem(Word Smem);

#endif // __DECODE_H__
