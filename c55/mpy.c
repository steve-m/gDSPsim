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
#include <multiplier.h>

static gchar *mask[]=
{
  "0101010p RRrr011f", // MPY[R] [ACx,] ACy
  "0101100p RRrrtt0f", // MPY[R] Tx, [ACx,] ACy
};

static gchar *opcode[] = 
{ 
  "'MPY'f r,R",
  "'MPY'f t,r,R",
};

static void execute(struct _PipeLine *pipeP, struct _Registers *Reg);

Instruction_Class MPY_Obj =
{
  "MPY",
  NULL, // decode
  NULL, // address_stg
  NULL, // access_1
  NULL, // access_2
  NULL, // read_stg
  execute, // execute
  NULL, // write 
  NULL, // write_plus
  2,
  mask,
  opcode,
};

static void execute(struct _PipeLine *pipeP, struct _Registers *Reg)
{
  Opcode opcode;
  int r,R,t,flag;

  opcode = pipeP->decode_nfo.mach_code;

  switch ( pipeP->opcode_subType )
    {
    case 0:
      // MPY[R] [ACx,] ACy
      r=(opcode.bop[1]>>4)&0x3;
      R=(opcode.bop[1]>>6)&0x3;
      flag = (opcode.bop[1]&0x1) ? MULT_ROUND : 0;
      multiplier(r,R,MULT_NO_MAC,R,flag,Reg);
      break;

    case 1:
      // MPY[R] Tx, [ACx,] ACy
      t=(opcode.bop[1]>>2)&0x3;
      r=(opcode.bop[1]>>4)&0x3;
      R=(opcode.bop[1]>>6)&0x3;
      flag = (opcode.bop[1]&0x1) ? MULT_ROUND : 0;
      multiplier(t,r,MULT_NO_MAC,R,flag,Reg);
      break;

    }
}
