/*
 * gDSPsim - GNU Digital Signal Processor Simulator
 *
 * Copyright (C) 2002, Kerry Keal, kerry@industrialmusic.com
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
#include "shifter.h"

static gchar *mask[]=
  {
    "0101000p rrrrv000", // SFTL dst, #1
    "0101000p rrrrv001", // SFTL dst, #­1
    "0101110p RRrrtt00", // SFTL ACx, Tx[, ACy]
    "0001000p RRrr0111 vvnnnnnn", // SFTL ACx, #SHIFTW[, ACy]
  };
static gchar *opcode[] = 
{ 
  "'SFTL' r,#1",
  "'SFTL' r,#-1",
  "'SFTL' r,t,R",
  "'SFTL' r,#n,R",
};

static void execute(struct _PipeLine *pipeP, struct _Registers *Reg);

Instruction_Class SFTL_Obj =
{
  "SFTL",
  NULL, // decode
  NULL, // address_stg
  NULL, // access_1
  NULL, // access_2
  NULL, // read_stg
  execute, // execute
  NULL, // write 
  NULL, // write_plus
  4,
  mask,
  opcode,
};



static void execute(struct _PipeLine *pipeP, struct _Registers *Reg)
{
  Opcode opcode;
  int n,flag;

  opcode = pipeP->decode_nfo.mach_code;
  flag = C54CM(MMR) ? (SHFT_M40_IS_0 | SHFT_DONT_SATURATE | SHFT_UNSIGNED |
		       SHFT_NO_OVERFLOW_DETECTION) : 
                      (SHFT_USE_M40 | SHFT_DONT_SATURATE | SHFT_UNSIGNED |
		       SHFT_NO_OVERFLOW_DETECTION);

  switch ( pipeP->opcode_subType )
    {
    case 0:
      // SFTL dst, #1
      shifter((opcode.bop[1]>>4)&0xf, 8, 1, flag,
	      (opcode.bop[1]>>4)&0xf, Reg);
      break;

    case 1:
      // SFTL dst, #­1
      shifter((opcode.bop[1]>>4)&0xf, 8, -1, flag,
	      (opcode.bop[1]>>4)&0xf, Reg);
      break;

    case 2:
      // SFTL ACx, Tx[, ACy]
      shifter((opcode.bop[1]>>4)&0x3, (opcode.bop[1]>>2)&0x3, 0, flag,
	      (opcode.bop[1]>>6)&0x3, Reg);
      break;

    case 3:
      // SFTL ACx, #SHIFTW[, ACy]
      n = (int)SIGN6BIT_TO_UINT(opcode.bop[2]&0x3f);
      shifter((opcode.bop[1]>>4)&0x3, 8, n, flag, 
	      (opcode.bop[1]>>6)&0x3, Reg);
      break;
      
    }
}


