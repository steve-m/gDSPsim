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

#include <alu.h>
#include <chip_help.h>

/* Xmux 0-15         Register 0-15
 *      ALU_SHIFTER  Input from the Shifter register
 *      ALU_DB       Input from the DB register
 *      ALU_CB       Input from the DB register
 *      ALU_BB       Input from the DB register
 *      ALU_DBL      Input is dbl from the Lmem register (DB,CB)
 * Ymux 0-15         Register 0-15
 *      ALU_SHIFTER  Input from the Shifter register
 *      ALU_DB       Input from the DB register
 *      ALU_CB       Input from the DB register
 *      ALU_BB       Input from the DB register
 *      ALU_DBL      Input is dbl from the Lmem register (DB,CB)
 * Omux 0-15         Output to Register 0-15
 *      ALU_EB       Output to the EB register
 */

/*

 * flag bit 0-1 0=add 1=Y-X 
 * flag bit 0 M40, overflow done at (39,15)
================================================================
would like defaults to follow add src,dst

CARRY = bit 32 if result is accumulator, addition M40=0
      = bit 40 if result is accumulator, addition M40=1
      = bit 16 if result is 16-bit reg, addition
      = NOT(bit 32) if result is accumulator, subtraction
      = NOT(bit 40) if result is accumulator, subtraction
      = NOT(bit 16) if result is 16-bit reg, subtraction

Saturation:
 16-bit result, saturated to 16 bits if SATA=1
 accumulator result, saturated if SATD=1, saturated to 40-bits if M40=1

Overflow:
 ACOVx set if result is accumulator and overflowed 40-bit if M40=1 and
 or overflowed 32-bit if M40=0

unknown what 0x00ffffffff + 0x0000001000 = ? if SXMD=1

if result is 16-bits and input is accumulator use LSB of accumulator

if C54CM=1 then M40 is locally set to 0 when addition

*/
/*
bit_1 = 0x1 then X value is unsigned

================================================================

// bit 1  : 1=sign extend from bit 0 (SXMD)
//       : if bit_0 = 0x0 (M40=0)
//            if bit_1 = 0x1 ( SXMD )
//               guard bits are replaced with bit 31 of source
//            else ( SXMD=0 )
//               guard bits are cleared


// bit 3-2 
// 00 (0x0) use SATD (accum) or SATA (16 bit reg) for saturation
// 01 (0x4) don't saturate
// 10 (0x8) saturate
// 11 (0xc) saturate to 00ffffffff

// bit 6
// 0x40=set ACOVx on overflow using bit set with bit_0 above

 * 
 * default options
 * 16-bits + LSB(40-bits) = 16-bits
 * ? + ? = 16-bits (overflow detected at bit 15 and saturated accord SATA
 * ? + ? = 40-bits (saturated accord SATD) (ACOVx not set)


 */
/* 
 * Xmux 0, X operand is from DB register
 *      1, X operand is from shifter register
 *      2, X operand is from A(32-16) (ABDST)
 * Ymux 0, Y operand is from A register
 *      1, Y operand is from B register
 *      2, Y operand is from T register
 *      3, Y operand is from CB register
 * Omux 0, store answer in A
 *      1, store answer in B
 *      2, store answer in EAB
 * flag bit 0-1, 0=add 1=Y-X 2=X-Y 3=sub high word, add low word (DSADT)
 *      bit 2 shift DB and CB by 16 each (SUB,SQDST,LMS)
 *      bit 3 40 bit mode
 *      bit 4 ignored SXM
 *      bit 5 round result by adding 2^15
 *      bit 6 ADD can only set and SUB can only reset Carry bit
 */ 
void alu(int Xmux, int Ymux, int Omux, int flag, struct _Registers *Reg)
{
  union _GP_Reg_Union X,Y;
  int carry_bit,carry,_M40;

  if ( (flag & ALU_UNSIGNED_X) || (SXMD(MMR) == 0) )
    {
      if ( Xmux < 16 )
	X = get_register(Xmux,0);
      else if ( Xmux == ALU_SHIFTER )
	X.guint64 = GP_REG_TO_UINT64(Reg->Shifter);
      else if ( Xmux == ALU_DB )
	X.guint64 = Reg->DB;
      else if ( Xmux == ALU_CB )
	X.guint64 = Reg->CB;
      else if ( Xmux == ALU_BB )
	X.guint64 = Reg->BB;
      else if ( Xmux == ALU_DBL )
	{
	  // fixme, need to check this
	  X.guint64 = 0;
	  X.words.low = Reg->CB;
	  X.words.high = Reg->DB;
	}
      else
	{
	  printf("Error! bad Xmux value in call to alu\n");
	}
    }
  else
    {
      if ( Xmux < 16 )
	X = get_register(Xmux,1);
      else if ( Xmux == ALU_SHIFTER )
	X.gint64 = GP_REG_TO_INT64(Reg->Shifter);
      else if ( Xmux == ALU_DB )
	X.gint64 = (SWord)Reg->DB;
      else if ( Xmux == ALU_CB )
	X.gint64 = (SWord)Reg->CB;
      else if ( Xmux == ALU_BB )
	X.gint64 = (SWord)Reg->BB;
      else if ( Xmux == ALU_DBL )
	{
	  // fixme, need to check this
	  X.gint64 = (SWord)Reg->DB;
	  X.guint64 = X.guint64 << 16;
	  X.guint64 = X.guint64 | Reg->CB;
	}
      else
	{
	  printf("Error! bad Xmux value in call to alu\n");
	}
    }

  if ( (flag & ALU_UNSIGNED_Y) || (SXMD(MMR) == 0) )
    {
      if ( Ymux < 16 )
	Y = get_register(Ymux,0);
      else if ( Ymux == ALU_SHIFTER )
	Y.guint64 = GP_REG_TO_UINT64(Reg->Shifter);
      else if ( Ymux == ALU_DB )
	Y.guint64 = Reg->DB;
      else if ( Ymux == ALU_CB )
	Y.guint64 = Reg->CB;
      else if ( Ymux == ALU_BB )
	Y.guint64 = Reg->BB;
      else if ( Ymux == ALU_DBL )
	{
	  // fixme, need to check this
	  Y.guint64 = 0;
	  Y.words.low = Reg->CB;
	  Y.words.high = Reg->DB;
	}
      else
	{
	  printf("Error! bad Ymux value in call to alu\n");
	}
    }
  else
    {
      if ( Ymux < 16 )
	Y = get_register(Ymux,1);
      else if ( Ymux == ALU_SHIFTER )
	Y.gint64 = GP_REG_TO_INT64(Reg->Shifter);
      else if ( Ymux == ALU_DB )
	Y.gint64 = (SWord)Reg->DB;
      else if ( Ymux == ALU_CB )
	Y.gint64 = (SWord)Reg->CB;
      else if ( Ymux == ALU_BB )
	Y.gint64 = (SWord)Reg->BB;
      else if ( Ymux == ALU_DBL )
	{
	  // fixme, need to check this
	  Y.gint64 = (SWord)Reg->DB;
	  Y.guint64 = Y.guint64 << 16;
	  Y.guint64 = Y.guint64 | Reg->CB;
	}
      else
	{
	  printf("Error! bad Ymux value in call to alu\n");
	}
    }

  if ( flag & ALU_SUB )
    {
      if ( flag & ALU_WITH_CARRY )
	{
	  // fixme, check this
	  X.gint64 = Y.gint64 + X.gint64 - CARRY(MMR);
	}
      else
	{
	  X.gint64 = Y.gint64 - X.gint64;
	}
    }
  else
    {
      if ( flag & ALU_WITH_CARRY )
	{
	  X.gint64 = Y.gint64 + X.gint64 + CARRY(MMR);
	}
      else
	{
	  X.gint64 = Y.gint64 + X.gint64;
	}
    }

  // Set C bit
  /*
CARRY = bit 32 if result is accumulator, addition M40=0
      = bit 40 if result is accumulator, addition M40=1
      = bit 16 if result is 16-bit reg, addition
      = NOT(bit 32) if result is accumulator, subtraction
      = NOT(bit 40) if result is accumulator, subtraction
      = NOT(bit 16) if result is 16-bit reg, subtraction
  */
    
  _M40 = M40(MMR);
  carry_bit = ( Omux < 4 ) ? ( _M40 ? 40 : 32 ) : 16;
  carry = X.guint64 & (((guint64)1)<<carry_bit) ? 1 : 0;
  if ( flag & ALU_SUB )
    {
      // Subtraction
      set_CARRY(MMR,!carry);
    }
  else
    {
      set_CARRY(MMR,!carry);
    }
  
  if ( Omux < 16 )
    set_register(X,Omux);
  else if ( Omux == ALU_EB )
    Reg->EB = X.words.low;
  
}
