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
#include <chip_help.h>
#include <smem.h>
#include <memory.h>

static gchar *mask[]=
{
  "01110110 kkkkkkkk kkkkkkkk RRRR00rr", // BFXTR k16, ACx, dst
};

static gchar *opcode[] = 
{ 
  "'BFXTR' h,r,R",
};

static void execute(struct _PipeLine *pipeP, struct _Registers *Reg);

Instruction_Class BFXTR_Obj =
{
  "BFXTR",
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
  Opcode opcode;
  int bit_extract,bit_place,r,R;
  union _GP_Reg_Union reg_union;
  Word kword1,kword2;

  opcode = pipeP->decode_nfo.mach_code;
  
  r = opcode.bop[3]&0x3;

  reg_union = get_register(r,0);
  kword1 = (Word)opcode.bop[1]<<16 | opcode.bop[2]<<16;
  kword2 = 0;
  bit_place = 0;
  bit_extract = 0;
  while ( kword1 )
    {
      if ( kword1 & 1 )
	{
	  if ( reg_union.words.low & (Word)1<<bit_extract)
	    {
	      kword2 = kword2 | (Word)1<<bit_place;
	      bit_extract++;
	    }
	  bit_place++;
	}
      kword1 = kword1 >> 1;
    }

  R = (opcode.bop[3]>>4)&0xf;
  reg_union.guint64 = kword2;
  set_register(reg_union,R);
}
