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

static gchar *mask[]=
{
  "0010111p rrrrRRRR", // MAX [src,] dst
};

static gchar *opcode[] = 
{ 
  "'MAX' r,R", // MAX [src,] dst
};

static void execute(struct _PipeLine *pipeP, struct _Registers *Reg);

Instruction_Class MAX_Obj =
{
  "MAX",
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
  int r,R,_m40;
  Opcode opcode;
  union _GP_Reg_Union reg_union1;
  union _GP_Reg_Union reg_union2;

  r=(opcode.bop[1]>4)&0xf;
  R=(opcode.bop[1])&0xf;

  _m40 = C54CM(MMR) ? 1 : M40(MMR);

  if ( R < 4 )
    {
      reg_union1 = get_register2(r,SXMD(MMR),_m40);
      reg_union2 = get_register2(R,SXMD(MMR),_m40);
    }
  else
    {
      reg_union1 = get_register2(r,SXMD(MMR),_m40);
      if ( SXMD(MMR) )
        reg_union2.gint64 = (SWord)get_k16_reg(R);
      else
        reg_union2.guint64 = get_k16_reg(R);
    }


  if ( reg_union1.gint64 < reg_union2.gint64 )
    {
      set_CARRY(MMR,0);
      set_register(reg_union2,R);
    }
  else
    {
      set_CARRY(MMR,1);
      set_register(reg_union1,R);
    }
}
