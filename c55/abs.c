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
    "0011001p rrrrRRRR", // ABS [src,] dst
  };

static gchar *opcode[] = 
{ 
  "ABS r,R",
};

static void execute(struct _PipeLine *pipeP, struct _Registers *Reg);

Instruction_Class ABS_Obj =
{
  "ABS",
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
  int r,R,_M40,_SAT;
  Opcode opcode;
  union _GP_Reg_Union reg_union1;

  opcode = pipeP->decode_nfo.mach_code;

  r = (opcode.bop[1]>>4)&0xf;
  R = (opcode.bop[1])&0xf;

  reg_union1 = get_register(r,SXMD(MMR));

  // clear CARRY if non-zero. set CARRY if zero
  if ( reg_union1.guint64 )
    {
      set_CARRY(MMR,0);
    }
  else
    {
      set_CARRY(MMR,1);
    }

  _M40 = C54CM(MMR) ? 0 : M40(MMR);
  _SAT = (R<4) ? SATD(MMR) : SATA(MMR);

  if ( reg_union1.gint64 < 0 )
    reg_union1.gint64 = -reg_union1.gint64;

  saturate(reg_union1,R,_M40,_SAT);
      
}
