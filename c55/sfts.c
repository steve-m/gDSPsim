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
    "0100 0100 0100 rrrr", // SFTS r,#-1
    "0100 0100 0101 rrrr", // SFTS r,#1
    "0101 1100 rrqq tt01", // sfts ACr,tu,ACq
    "0001 0000 rrqq 0101 00nn nnnn  ", // sfts ACr,#n,ACq
  };
static gchar *opcode[] = 
{ 
  "SFTS r,#-1",
  "SFTS r,#1",
  "SFTS r,t,q",
  "SFTS r,#n,q",
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
  Opcode mach_code;

  mach_code = pipeP->decode_nfo.mach_code;

  switch ( pipeP->opcode_subType )
    {
    case 0:
      shifter(mach_code.bop[1]&0xf,8,-1,0,mach_code.bop[1]&0xf,Reg);
      break;
    case 1:
      shifter(mach_code.bop[1]&0xf,8,1,0,mach_code.bop[1]&0xf,Reg);
      break;
    case 2:
      shifter((mach_code.bop[1]&0xc0)>>6,(mach_code.bop[1]&0xc)>>2,0,0,(mach_code.bop[1]&0x30)>>4,Reg);
      break;
    case 3:
      {
	int n;

	n = (int)SIGN6BIT_TO_UINT(mach_code.bop[2]&0x3f);
	shifter((mach_code.bop[1]&0xc0)>>6,8,n,0,(mach_code.bop[1]&0x30)>>4,Reg);
	break;
      }
    }
}


