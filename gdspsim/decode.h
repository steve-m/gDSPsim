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

// Returns the number of words 1,2,3 that match the mask
// 0, if it doesn't match. Will read program memory address+1 
// and address+2 only if needed
int check_mask(const char *mask, Word mach_code, WordA address );

// This is used to decode pointer registers and the modifiers on them
gchar *a_decode(gchar *mask, char info, Word start_code, WordA *location);
// This is used to decode memory mapped registers.
gchar *b_decode(gchar *mask, char info, Word start_code, WordA *location);
gchar *c_decode(gchar *mask, char info, Word start_code, WordA *location);
// Decode memory mapped registers, STM #0,AR0 or STM #0,0x60
gchar *m_decode(gchar *mask, char info, Word start_code, WordA *location);
// Decodes 9 memory mapped registers MMRx (ARx,SP) MVMM AR2,SP
gchar *vw_decode(gchar *mask, char info, Word start_code, WordA *location);
gchar *r_decode(gchar *mask, char info, Word start_code, WordA *location);
gchar *z_decode(gchar *mask, char info, Word start_code, WordA *location);
gchar *sd_decode(gchar *mask, char info, Word start_code, WordA *location);
gchar *xy_decode(gchar *mask, char info, Word start_code, WordA *location);
// extract unsigned decimal number
gchar *u_decode(gchar *mask, char info, Word start_code, WordA *location);
// same as u_decode but adds plus 1 to the number
gchar *p_decode(gchar *mask, char info, Word start_code, WordA *location);
// Unsigned hex number
gchar *h_decode(gchar *mask, char info, Word start_code, WordA *location);
gchar *n_decode(gchar *mask, char info, Word start_code, WordA *location);
gchar *t_decode(gchar *mask, char info, Word start_code, WordA *location);
// Extracts the signed number from data by matching info to the mask.
// It will read more program memory if needed.
SWord signed_bit_extract(char info, char *mask, Word mach_code, WordA *location);
// Extracts the 5 least signifigants bits, with the 5th bit being the sign. 
SWord signed_5bit_extract(Word mach_code);
// Extracts the unsigned number from data by matching info to the mask.
// It will read more program memory if needed.
Word bit_extract(char info, char *mask, Word mach_code, WordA *location);


// Returns 1 if condition is true
int check_condition(Word bits);

#endif // __DECODE_H__
