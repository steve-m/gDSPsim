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
      
#include <chip_core.h>
#include <multiplier.h>

static gchar *mask[]=
  {
    "0101010p rrRR100f",
  };

static gchar *opcode[] = 
{ 
  "'SQR'f r,R",
};

static void execute(struct _PipeLine *pipeP, struct _Registers *Reg);

Instruction_Class SQR_Obj =
{
  "SQR",
  NULL, // decode
  NULL, // address_stg
  NULL, // access_1
  NULL, // access_2
  NULL, // read_stg
  execute, // execute
  NULL, // write 
  NULL, // write_plus
  1,
  mask,
  opcode,
};



static void execute(struct _PipeLine *pipeP, struct _Registers *Reg)
{
  Opcode mach_code;
  int r,R,rnd;

  mach_code = pipeP->decode_nfo.mach_code;

  r=(mach_code.bop[1]&0xc0)>>6;
  R=(mach_code.bop[1]&0x30)>>4;
  rnd = (mach_code.bop[1]&0x1)<<2;
  
  multiplier(r,r,8,R+rnd,Reg);
}


