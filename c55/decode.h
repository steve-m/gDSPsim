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

#include "chip_core.h"

#ifndef __DECODE_H__
#define __DECODE_H__


extern struct _Registers *Registers;

#define MAX_SUB_OP 20

typedef void (*Decode_Func)(gchar *ch, gchar *mask, gchar info, 
			    struct _decoded_opcode *decode_nfo );

#ifdef __cplusplus
extern "C" {
#endif

union u_operands read_op(char info, struct _Registers *Registers, Word bits, Word lk, int *wait_state);

// returns 1, if this mask matches the mach_code and sets the
// length and other mach_codes if needed in decode_nfo. returns
// 0, if mask doesn't match and doesnt set anything in decode_nfo.
int check_mask(const char *mask, struct _decoded_opcode *decode_nfo );

void m_decode(gchar *ch, gchar *mask, char info, 
              struct _decoded_opcode *decode_nfo );
// Decodes 9 memory mapped registers MMRx (ARx,SP) MVMM AR2,SP
  //void vw_decode(gchar *ch, gchar *mask, char info, 
  //              struct _decoded_opcode *decode_nfo );
// decodes a round
void r_decode(gchar *ch, gchar *mask, char info, 
              struct _decoded_opcode *decode_nfo );
// extract unsigned decimal number
void u_decode(gchar *ch, gchar *mask, char info, 
              struct _decoded_opcode *decode_nfo);

/void p_decode(gchar *ch, gchar *mask, char info, 
              struct _decoded_opcode *decode_nfo );
// Unsigned hex number
void h_decode(gchar *ch, gchar *mask, char info, 
              struct _decoded_opcode *decode_nfo );
// Signed number
void n_decode(gchar *ch, gchar *mask, char info, 
              struct _decoded_opcode *decode_nfo );
// decodes status bits
void t_decode(gchar *ch, gchar *mask, char info, 
              struct _decoded_opcode *decode_nfo );

// Extracts the signed number from data by matching info to the mask.
// It will read more program memory if needed.
SWord signed_bit_extract(char info, char *mask,
                         struct _decoded_opcode *decode_nfo );
SWord decode_signed_bit_extract(char info, char *mask, struct _decoded_opcode *decode_nfo);
// Extracts the 5 least signifigants bits, with the 5th bit being the sign. 
SWord signed_5bit_extract(Word mach_code);
// Extracts the unsigned number from data by matching info to the mask.
// Also returns the word number that the first match occured. num_mask
// is the number of char info in string mask.
Word bit_extract(char info, char *mask, struct _decoded_opcode *decode_nfo, 
		 int *worn_num, int *num_mask, int *length);
Word xor_bit_extract(char info, char *mask, struct _decoded_opcode *decode_nfo, 
		 int *worn_num, int *num_mask);


// Returns 1 if condition is true
int check_condition(Word bits);

#ifdef __cplusplus
}
#endif

#endif // __DECODE_H__
