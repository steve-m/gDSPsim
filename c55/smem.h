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

  // Sets DAB
  void smem_address_stg_b2(struct _PipeLine *pipeP, struct _Registers *Reg);
  
  // Sets DB
  void smem_read_stg(struct _PipeLine *pipeP, struct _Registers *Reg);

  // Sets DB2 (DWord)
  void smem_read_stg_dbl(struct _PipeLine *pipeP, struct _Registers *Reg);
  
  // Sets EAB
  void smem_set_EAB_b2(struct _PipeLine *pipeP, struct _Registers *Reg);
  
  Word smem_decode(int p, int mod, unsigned char b1, 
		    unsigned char b2, unsigned char b3);
  
  Word bit_reversal(Word start, Word bit_reversed_one, SWord adjustment);
  
  // r register number 0-15
  void set_k16_reg(int r, Word value, int sign_extend);
  
  // r register number 0-3
  void set_k32_reg(int r, DWord value, int sign_extend40);
  
  // r=0, HI(AC0)=value1
  //      HI(AC1)=value2
  // r=2, HI(AC2)=value1
  //      HI(AC3)=value2
  void set_k16_pair_HIreg(int r, Word value1, Word value2, int sign_extend);

  // r=0, HI(AC0)=value1
  //      HI(AC1)=value2
  // r=2, HI(AC2)=value1
  //      HI(AC3)=value2
  void set_k16_pair_LOreg(int r, Word value1, Word value2, int sign_extend);

  // r register number 0-15, returns low bits for accumulators
  Word get_k16_reg(int r);

  // r register number 0-15, returns hi bits for accumulators
  Word get_k16_regHI(int r, int rnd);

   // r register number 0-3, returns 32bits for accumulators
  DWord get_k32_reg(int r);

 // p=0-7
  Word *get_pointer_reg(int p);

  Word circular_update(Word start, int p, SWord step);

#ifdef __cplusplus
}
#endif

#endif // __SMEM_C55_H__
