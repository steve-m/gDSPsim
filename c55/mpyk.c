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
    "0001111p nnnnnnnn rrRRvv0f", // "'MPYK'f n,r,R"
    "01111001 nnnnnnnn nnnnnnnn rrRRvv0f", // "'mpyk'f n,r,R"
  };
static gchar *opcode[] = 
{ 
  "'MPYK'f n,r,R",
  "'MPYK'f n,r,R",
};

static void execute(struct _PipeLine *pipeP, struct _Registers *Reg);

Instruction_Class MPYK_Obj =
{
  "MPYK",
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
  Opcode mach_code;
  int r,R,flag;

  mach_code = pipeP->decode_nfo.mach_code;

  switch ( pipeP->opcode_subType )
    {
    case 0:
      r=(mach_code.bop[2]&0xc0)>>6;
      R=(mach_code.bop[2]&0x30)>>4;
      flag = (mach_code.bop[2]&0x1) ? MULT_ROUND : 0;

      Reg->P = (SWord)((char)(mach_code.bop[1]));
      multiplier(MULT_P,r,MULT_NO_MAC,R,flag,Reg);
      break;
    case 1:
      r=(mach_code.bop[3]&0xc0)>>6;
      R=(mach_code.bop[3]&0x30)>>4;
      flag = (mach_code.bop[3]&0x1) ? MULT_ROUND : 0;

      Reg->P = mach_code.bop[1]<<8 | mach_code.bop[2];
      multiplier(MULT_P,r,MULT_NO_MAC,R,flag,Reg);
      break;
    }
}


