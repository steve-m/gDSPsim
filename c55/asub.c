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
#include <alu.h>

static gchar *mask[]=
{
  "0001010p rrrrvvvv RRRR0010", // ASUB TAx, TAy
  "0001010p nnnnnnnn RRRR0110", // ASUB k8, TAx
  "0001010p rrrrvvvv RRRR1010", // ASUB TAx, TAy
  "0001010p nnnnnnnn RRRR1110", // ASUB k8, TAx
};

static gchar *opcode[] = 
{
  "'ASUB' r,R",
  "'ASUB' #n,R",
  "'ASUB' r,R",
  "'ASUB' #n,R",
};

static void execute(struct _PipeLine *pipeP, struct _Registers *Reg);

Instruction_Class ASUB_Obj =
{
  "ASUB",
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
  int r,R;
  Opcode opcode;
  SWord n;
  union _GP_Reg_Union reg_union1;
  union _GP_Reg_Union reg_union2;

  opcode = pipeP->decode_nfo.mach_code;
  
  switch ( pipeP->opcode_subType )
    {
    case 0:
    case 2:
      // ASUB TAx, TAy
      r = (opcode.bop[1]>>4)&0xf;
      R = (opcode.bop[2]>>4)&0xf;

      reg_union1 = get_register(r,1);
      reg_union2 = get_register(R,1);

      if ( (R>7) && ARLC(MMR,R) )
	{
	  reg_union2.gint64 = circular_update(reg_union2.words.low, R, -(SWord)reg_union1.words.low);
	}
      else
	{
	  reg_union2.gint64 = reg_union2.gint64 - reg_union1.gint64;
	}

      set_register(reg_union2,R);
      return;

    case 1:
    case 3:
      // ASUB k8, TAx
      n = (char)opcode.bop[1];
      R = (opcode.bop[2]>>4)&0xf;

      reg_union2 = get_register(R,1);

      if ( (R>7) && ARLC(MMR,R) )
	{
	  reg_union2.gint64 = circular_update(reg_union2.words.low, R, -n);
	}
      else
	{
	  reg_union2.gint64 = reg_union2.gint64 - n;
	}

      set_register(reg_union2,R);
      return;
    }
}
