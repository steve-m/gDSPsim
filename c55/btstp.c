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
#include <chip_help.h>

static gchar *mask[]=
{
  "11101100 ssssssss rrrr010v", // BTSTP Baddr, sr
};

static gchar *opcode[] = 
{
  "'BTSTP' s,r", // BTSTP Baddr, src
};

static void execute(struct _PipeLine *pipeP, struct _Registers *Reg);

Instruction_Class BTSTP_Obj =
{
  "BTSTP",
  NULL, // decode
  smem_address_stg_b2, // address_stg
  NULL, // access_1
  NULL, // access_2
  smem_read_stg, // read_stg
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
  int bit,r;
  union _GP_Reg_Union reg_union;

  opcode = pipeP->decode_nfo.mach_code;

  // BTST Baddr, src, TCx
  r = (opcode.bop[2]>>4)&0xf;
  reg_union = get_register(r,0);
  if ( r < 4 )
    {
      bit = Reg->DB & 0x3f;
      if ( (bit < 40) && ((1<<bit) & reg_union.guint64) )
	{
	  set_TC1(MMR,1);
	}
      else
	{
	  set_TC1(MMR,0);
	}
      bit++;
      if ( (bit < 40) && ((1<<bit) & reg_union.guint64) )
	{
	  set_TC2(MMR,1);
	}
      else
	{
	  set_TC2(MMR,0);
	}
    }
  else
    {
      bit = Reg->DB & 0xf;
      
      if ( (1<<bit) & reg_union.guint64 )
	{
	  set_TC1(MMR,1);
	}
      else
	{
	  set_TC1(MMR,0);
	}
      bit++;
      if ( (bit<16) && ((1<<bit) & reg_union.guint64) )
	{
	  set_TC2(MMR,1);
	}
      else
	{
	  set_TC2(MMR,0);
	}
    }
}
