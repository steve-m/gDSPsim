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

#include <chip_core.h>

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
 * flag bit 0, 0=add 1=Y-X
 */ 

/* Default action:
================================================================

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
================================================================

*/


#define ALU_SHIFTER 16 // Input is from shifter register
#define ALU_DB 17 // Input is from DB register
#define ALU_CB 18 // Input is from CB register
#define ALU_BB 19 // Input is from BB register
#define ALU_DBL 20 // Input is dbl from Lmem register
#define ALU_EB 16 // Output is to EB register

#define ALU_UNSIGNED_X 2
#define ALU_UNSIGNED_Y 8
#define ALU_WITH_CARRY 4
#define ALU_SUB 1 // Y-X

void alu(int Xmux, int Ymux, int Omux, int flag, struct _Registers *Reg);

#endif // __ALU_H__
