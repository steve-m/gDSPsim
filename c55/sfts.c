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
    "0100010p 01v0rrrr", // SFTS dst, #­1
    "0100010p 01v1rrrr", // SFTS dst, #1
    "0101110p RRrrtt01", // SFTS ACx, Tx[, ACy]
    "0001000p RRrr0101 vvnnnnnn", // SFTS ACx, #SHIFTW[, ACy]
  };
static gchar *opcode[] = 
{ 
  "'SFTS' r,#-1",
  "'SFTS' r,#1",
  "'SFTS' r,t,R",
  "'SFTS' r,#n,R",
};

static void execute(struct _PipeLine *pipeP, struct _Registers *Reg);

Instruction_Class SFTS_Obj =
{
  "SFTS",
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

  flag = C54CM(MMR) 
    ? (SHFT_DONT_SATURATE | SHFT_NO_OVERFLOW_DETECTION | 
       SHFT_M40_IS_1 | SHFT_SIGN_EXTEND_USING_SXMD)
    : (SHFT_USE_SATD_SATA | SHFT_SIGN_EXTEND_USING_SXMD | 
       SHFT_USE_M40 | SHFT_SET_ACOV);
    
  switch ( pipeP->opcode_subType )
    {
    case 0:
      // SFTS dst, #­1
      shifter(opcode.bop[1]&0xf,SHFT_CONSTANT,-1,flag,opcode.bop[1]&0xf,Reg);
      break;

    case 1:
      // SFTS dst, #1
      shifter(opcode.bop[1]&0xf,SHFT_CONSTANT,1,flag,opcode.bop[1]&0xf,Reg);
      break;

    case 2:
      // SFTS ACx, Tx[, ACy]
      shifter((opcode.bop[1]>>4)&0x3,(opcode.bop[1]>>2)&0x3,0,flag,(opcode.bop[1]>>6)&0x3,Reg);
      break;

    case 3:
      // SFTS ACx, #SHIFTW[, ACy]
      n = (int)SIGN6BIT_TO_UINT(opcode.bop[2]&0x3f);
      shifter((opcode.bop[1]>>4)&0x3,SHFT_CONSTANT,n,flag,(opcode.bop[1]>>6)&0x3,Reg);
      break;
      
    }
}


