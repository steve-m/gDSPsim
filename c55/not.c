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
  "0011011p rrrrRRRR", // NOT [src,] dst
};

static gchar *opcode[] = 
{
  "'NOT' r,R", // NOT [src,] dst
};

static void execute(struct _PipeLine *pipeP, struct _Registers *Reg);

Instruction_Class NOT_Obj =
{
  "NOT",
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
  int r,R;
  Opcode opcode;
  union _GP_Reg_Union reg_union1;

  opcode = pipeP->decode_nfo.mach_code;

  r = (opcode.bop[1]>>4)&0xf;
  R = (opcode.bop[1])&0xf;

  reg_union1 = get_register(r,0);

  reg_union1.guint64 = reg_union1.guint64 ^ (guint64)0xffffffffffffffff;
  // reg_union1.guint64 = reg_union1.gu32.low ^ (guint32)0xffffffff;
  // reg_union1.guint64 = reg_union1.gu32.high ^ (guint32)0xffffffff;

  set_register(reg_union1,R);
}
