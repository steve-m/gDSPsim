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
  "0001001p rrrrvv11 RRRR1vzZ", /// ROR BitOut, src, BitIn, dst
};

static gchar *opcode[] = 
{
  "'ROR' z,r,Z,R", // ROR BitOut, src, BitIn, dst
};

static void execute(struct _PipeLine *pipeP, struct _Registers *Reg);

Instruction_Class ROR_Obj =
{
  "ROR",
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
  union _GP_Reg_Union reg_union;
  int BitOut,BitIn;

  opcode = pipeP->decode_nfo.mach_code;

  r = (opcode.bop[1]>>4)&0xf;
  R = (opcode.bop[2]>>4)&0xf;

  reg_union = get_register(r,0);

  if ( opcode.bop[2] & 0x2 )
    {
      // BitIn is TC2
      BitIn = TC2(MMR);
    }
  else
    {
      // BitIn is CARRY
      BitIn = CARRY(MMR);
    }

  if ( R < 4 )
    {
      if ( (C54CM(MMR)==1) || (M40(MMR)==0) )
	{
	  // roll out bit 31
	  BitOut = (reg_union.guint64 & (guint64)1<<31) ? 1 : 0;
	}
      else
	{
	  // roll out bit 39
	  BitOut = (reg_union.guint64 & (guint64)1<<39) ? 1 : 0;
	}
    }
  else
    {
      BitOut = (reg_union.guint64 & (guint64)1<<15) ? 1 : 0;
    }

  if ( opcode.bop[2] & 0x1 )
    {
      // BitOut is TC2
      set_TC2(MMR,BitOut);
    }
  else
    {
      // BitOut is CARRY
      set_CARRY(MMR,BitOut);
    }

  reg_union.guint64 = reg_union.guint64 << 1;

  reg_union.guint64 = reg_union.guint64 | BitIn;

  set_register(reg_union,R);
}

