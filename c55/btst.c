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
  "11100000 ssssssss rrrrvvvC", // BTST src, Smem, TCx
  "11011100 ssssssss uuuuvv0C", // BTST k4, Smem, TCx
  "11101100 ssssssss rrrr100C", // BTST Baddr, src, TCx
};

static gchar *opcode[] = 
{
  "'BTST' r,s,C", // BTST src, Smem, TCx
  "'BTST' u,s,C", // BTST k4, Smem, TCx
  "'BTST' s,r,C", // BTST Baddr, src, TCx
};

static void execute(struct _PipeLine *pipeP, struct _Registers *Reg);

Instruction_Class BTST_Obj =
{
  "BTST",
  NULL, // decode
  smem_address_stg_b2, // address_stg
  NULL, // access_1
  NULL, // access_2
  smem_read_stg, // read_stg
  execute, // execute
  NULL, // write 
  NULL, // write_plus
  3,
  mask,
  opcode,
};

static void execute(struct _PipeLine *pipeP, struct _Registers *Reg)
{
  Opcode opcode;
  int bit,r;
  union _GP_Reg_Union reg_union;

  opcode = pipeP->decode_nfo.mach_code;

  switch ( pipeP->opcode_subType )
    {
    case 0:
      // BTST src, Smem, TCx
      r = (opcode.bop[2]>>4)&0xf;
      reg_union = get_register(r,0);
      bit = reg_union.guint64 & 0xf;
      if ( (1<<bit) & Reg->DB )
	{
	  set_TCx(MMR,opcode.bop[2]&1,1);
	}
      else
	{
	  set_TCx(MMR,opcode.bop[2]&1,0);
	}
      break;

    case 1:
      // BTST k4, Smem, TC1
      bit = (opcode.bop[2]>>4)&0xf;
      if ( (1<<bit) & Reg->DB )
	{
	  set_TCx(MMR,opcode.bop[2]&1,1);
	}
      else
	{
	  set_TCx(MMR,opcode.bop[2]&1,0);
	}
      break;

    case 2:
      // BTST Baddr, src, TCx
      r = (opcode.bop[2]>>4)&0xf;
      reg_union = get_register(r,0);
      if ( r < 4 )
	{
	  bit = Reg->DB & 0x3f;
	  if ( (bit < 40) && ((1<<bit) & reg_union.guint64) )
	    {
	      set_TCx(MMR,opcode.bop[2]&1,1);
	    }
	  else
	    {
	      set_TCx(MMR,opcode.bop[2]&1,0);
	    }
	}
      else
	{
	  bit = Reg->DB & 0xf;
      
	  if ( (1<<bit) & reg_union.guint64 )
	    {
	      set_TCx(MMR,opcode.bop[2]&1,1);
	    }
	  else
	    {
	      set_TCx(MMR,opcode.bop[2]&1,0);
	    }
	}
      break;
    
    }

}
