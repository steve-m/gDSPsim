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

static gchar *mask[]=
{
  "0001000p vvrr1000 vvttvvvv", // EXP ACx, Tx
};

static gchar *opcode[] = 
{ 
  "'EXP' r,t", // EXP ACx, Tx
};

static void execute(struct _PipeLine *pipeP, struct _Registers *Reg);

Instruction_Class EXP_Obj =
{
  "EXP",
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
  int r,t;
  Opcode opcode;
  SWord shift;
  union _GP_Reg_Union reg_union1;

  r=(opcode.bop[1]>4)&3;
  t=(opcode.bop[2]>4)&3;

  reg_union1 = get_register(r,1);
  if ( reg_union1.gint64 == 0 )
    {
      shift = 0x0;
    }
  else if ( reg_union1.gint64 > 0 )
    {
      shift = 0;
      while ( (reg_union1.guint64 & (guint64)0x8000000000) == 0 )
	{
	  reg_union1.guint64 = reg_union1.guint64 << 1;
	  shift++;
	}
      shift = shift - 9;
    }
  else
    {
      shift = 0;
      while ( reg_union1.guint64 & (guint64)0x8000000000 )
	{
	  reg_union1.guint64 = reg_union1.guint64 << 1;
	  shift++;
	}
      shift = shift - 9;
    }
  reg_union1.gint64 = shift;
  set_register(reg_union1,t+4);  
}
