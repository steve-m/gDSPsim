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

  // p Parallel
  void p_decode(gchar *ch, gchar *mask, char info, 
		struct _decoded_opcode *decode_nfo );

  // r R register 2,4 bits
  void rR_decode(gchar *ch, gchar *mask, char info, 
		struct _decoded_opcode *decode_nfo );

  // t T register 2 bits
  void t_decode(gchar *ch, gchar *mask, char info, 
		struct _decoded_opcode *decode_nfo );

  // TC0,TC1 flag 1 bit
  void C_decode(gchar *ch, gchar *mask, char info, 
		struct _decoded_opcode *decode_nfo );

  // T TRN register 1 bit
  void T_decode(gchar *ch, gchar *mask, char info, 
		struct _decoded_opcode *decode_nfo );

  // x y Xmem Ymem addressing
  void xy_decode(gchar *ch, gchar *mask, char info, 
		 struct _decoded_opcode *decode_nfo );

  // c Cmem addressing
  void c_decode(gchar *ch, gchar *mask, char info, 
		struct _decoded_opcode *decode_nfo );

  // f Rounding (f is for floor, and letters were running out)
  void f_decode(gchar *ch, gchar *mask, char info, 
		struct _decoded_opcode *decode_nfo );

  // F Rounding (F is for floor, and letters were running out)
  // this round is rnd()
  void F_decode(gchar *ch, gchar *mask, char info, 
		struct _decoded_opcode *decode_nfo);
  // G close up F Rounding (F is for floor, and letters were running out)
  void G_decode(gchar *ch, gchar *mask, char info, 
		struct _decoded_opcode *decode_nfo);

  // s Smem decoding 8 bits
  void s_decode(gchar *ch, gchar *mask, char info, 
		struct _decoded_opcode *decode_nfo );

  // C Cmem addressing
  // c Condition 7-bits relation 2 bits
  // S swap code 6 bits
  // u unsigned int bits vary
  void u_decode(gchar *ch, gchar *mask, char info, 
		struct _decoded_opcode *decode_nfo );
  // n signed int bits vary
  void n_decode(gchar *ch, gchar *mask, char info, 
		struct _decoded_opcode *decode_nfo );
  // h hexidecial bits vary
  void h_decode(gchar *ch, gchar *mask, char info, 
		struct _decoded_opcode *decode_nfo );

  // A unsigned flag, think Abs
  void A_decode(gchar *ch, gchar *mask, char info, 
		struct _decoded_opcode *decode_nfo );
  // U unsigned wrapper start uns(
  void U_decode(gchar *ch, gchar *mask, char info, 
		struct _decoded_opcode *decode_nfo );
  // V unsigned wrapper end )
  void V_decode(gchar *ch, gchar *mask, char info, 
		struct _decoded_opcode *decode_nfo );

  // 3 Update of t3 1=update 1 bit
  void t3_decode(gchar *ch, gchar *mask, char info, 
		 struct _decoded_opcode *decode_nfo );
  
  // 1=TC2 0=CARRY (used for rol,ror)
  void zZ_decode(gchar *ch, gchar *mask, char info, 
		 struct _decoded_opcode *decode_nfo );
  
  // v reserved
  void v_decode(gchar *ch, gchar *mask, char info, 
		struct _decoded_opcode *decode_nfo );
  // extract signed decimal number where the sign bit
  // is assumed
  void m_decode(gchar *ch, gchar *mask, char info, 
		struct _decoded_opcode *decode_nfo);
  // 4 40bit keyword applied
  // ? shift out code
  
  // Extracts the signed number from data by matching info to the mask.
  // It will read more program memory if needed.
  SWord signed_bit_extract(char info, char *mask,
			   struct _decoded_opcode *decode_nfo );

  SWord decode_signed_bit_extract(char info, char *mask, 
				  struct _decoded_opcode *decode_nfo);

  // Extracts the 5 least signifigants bits, with the 5th bit being the sign. 
  SWord signed_5bit_extract(Word mach_code);

  // Extracts the unsigned number from data by matching info to the mask.
  // Also returns the word number that the first match occured. num_mask
  // is the number of char info in string mask.
  Word bit_extract(char info, char *mask, struct _decoded_opcode *decode_nfo, 
		   int *worn_num, int *num_mask, int *length);
  gboolean one_bit_extract(char info, char *mask, 
			   struct _decoded_opcode *decode_nfo);
  Word xor_bit_extract(char info, char *mask, 
		       struct _decoded_opcode *decode_nfo, 
		       int *worn_num, int *num_mask);

  
  // Returns 1 if condition is true
  int check_condition(Word bits);
  
#ifdef __cplusplus
}
#endif

#endif // __DECODE_H__
