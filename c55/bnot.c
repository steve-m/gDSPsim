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
#include <smem.h>
#include <memory.h>
#include <chip_help.h>

static gchar *mask[]=
{
  "11100011 ssssssss rrrr111x", // BNOT src, Smem
  "11101100 ssssssss rrrr011v", // BNOT Baddr, src
};

static gchar *opcode[] = 
{
  "'BNOT' r,s", // BNOT src, Smem
  "'BNOT' s,r", // BNOT Baddr, src
};

static void execute(struct _PipeLine *pipeP, struct _Registers *Reg);
static void write_stg(struct _PipeLine *pipeP, struct _Registers *Reg);

Instruction_Class BNOT_Obj =
{
  "BNOT",
  NULL, // decode
  smem_address_stg_b2, // address_stg
  NULL, // access_1
  NULL, // access_2
  smem_read_stg, // read_stg
  execute, // execute
  write_stg, // write 
  NULL, // write_plus
  2,
  mask,
  opcode,
};

static void execute(struct _PipeLine *pipeP, struct _Registers *Reg)
{
  Opcode opcode;
  int r;
  Word bit;
  union _GP_Reg_Union reg_union;

  opcode = pipeP->decode_nfo.mach_code;

  switch ( pipeP->opcode_subType )
    {
    case 0:
      // BNOT src, Smem
      r = (opcode.bop[2]>>4)&0xf;
      reg_union = get_register(r,0);
      bit = reg_union.words.low & 0xf;
      pipeP->storage1 = Reg->DB ^ (1<<bit);
      break;
    case 1:
      // BNOT Baddr, src
      r = (opcode.bop[2]>>4)&0xf;
      if ( r < 4 )
	{
	  bit = Reg->DB & 0x3f;
	  if ( bit < 40 )
	    {
	      reg_union = get_register(r,0);
	      reg_union.guint64 = reg_union.guint64 ^ (1<<bit);
	      set_register(reg_union,r);
	    }
	}
      else
	{
	  bit = Reg->DB & 0xf;
	  reg_union = get_register(r,0);
	  reg_union.guint64 = reg_union.guint64 ^ (1<<bit);
	  set_register(reg_union,r);
	}
      break;
    }
}
 
static void write_stg(struct _PipeLine *pipeP, struct _Registers *Reg)
{
  if ( pipeP->opcode_subType==1 )
    { 
      write_data_mem_long(pipeP->storage2,pipeP->storage1);
    } 
}

