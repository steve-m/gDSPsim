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
  "0010001p rrrrRRRR", // MOV src, dst
  "0100010p 00rrRRRR", // MOV HI(ACx), TAx
  "0101001p rrrr00RR", // MOV TAx, HI(ACx)
};

static gchar *opcode[] = 
{
  "'MOV' r,R",
  "'MOV' r,R",
  "'MOV' r,R",
};

static void execute(struct _PipeLine *pipeP, struct _Registers *Reg);

Instruction_Class MOV_REG_MOVE_Obj =
{
  "MOV_REG_MOVE",
  NULL, // decode
  NULL, // address_stg
  NULL, // access_1
  NULL, // access_2
  NULL, // read_stg
  execute, // execute
  NULL, // write_stg 
  NULL, // write_plus
  3,
  mask,
  opcode,
};

static void execute(struct _PipeLine *pipeP, struct _Registers *Reg)
{
  Opcode opcode;
  int r,R;
  Word kword;
  union _GP_Reg_Union reg_union;

  opcode = pipeP->decode_nfo.mach_code;

  switch ( pipeP->opcode_subType )
    {
    case 0:
      // MOV src, dst
      r = (opcode.bop[1]>>4) & 0xf;
      R = opcode.bop[1] & 0xf;

      if ( R > 3 )
	{
	  // destination is 16-bits
	  kword = get_k16_reg(r);
	  set_k16_reg(R,kword,0);
	}
      else
	{
	  if ( r < 4 )
	    {
	      // Move from GP reg to GP reg
	      // saturate according to SATD
	      // detect overflow according to M40
	      reg_union = get_register(r,1);
	      set_reg_saturate(reg_union,R,SATD(MMR));
	    }
	  else
	    {
	      // Move from 16 bit reg to 40 bit reg
	      // sign extend according to SXMD
	      kword = get_k16_reg(r);
	      if ( SXMD(MMR) )
		reg_union.gint64 = (gint64)kword;
	      else
		reg_union.guint64 = kword;
	      set_register(reg_union,R);
	    }
	}
      return;
    case 1:
      // MOV HI(ACx), TAx
      r = (opcode.bop[1]>>4) & 0x3;
      R = opcode.bop[1] & 0xf;
   
      kword = get_k16_regHI(r,0);
      set_k16_reg(R,kword,0);

      return;

    case 2:
      // MOV TAx, HI(ACx)

      r = (opcode.bop[1]>>4) & 0xf;
      R = opcode.bop[1] & 0x3;

      if ( r < 4 )
	{
	  reg_union = get_register(r,1);
	  reg_union = saturate(reg_union,r);
	}
      else
	{
	  if ( SXMD(MMR) )
	    {
	      kword = get_k16_reg(r);
	      reg_union.gint64 = (SWord)kword;
	      reg_union.guint64 = reg_union.guint64 << 16;
	    }
	  else
	    {
	      reg_union.gint64 = 0;
	      reg_union.words.high = get_k16_reg(r);
	    }
	}
      set_register(reg_union,R);

    }
}
