/*
 * gDSPsim - GNU Digital Signal Processor Simulator
 *
 * Copyright (C) 2001, Kerry Keal, kerry@industrialmusic.com
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

#include "c54_core.h"
#include "shifter.h"
#include <stdio.h>

static void execute(struct _PipeLine *pipeP, struct _Registers *Reg);
static GPtrArray *machine_code(gchar *opcode_text);

static gchar *mask[]=    { "1111010s 10010100" };
static gchar *opcode[] = { "SFTC s" };
static gchar *comment[]= { "Shift Conditionally" };

/* This definition is global because another routine will make have
 * an array that points to all the different instruction classes.
 */
Instruction_Class SFTC_Obj =
{
  "SFTC",
  NULL, // prefetch
  NULL, // fetch
  NULL, // decode
  NULL, // read_stg1 (access)
  NULL, // read_stg2 (read)
  execute, // execute
  NULL, // number_words 
  NULL, // set_cycle_number
  1,
  mask,
  opcode,
  comment,
  machine_code
};

static void execute(struct _PipeLine *pipeP, struct _Registers *Reg)
{
  int regNumSrc;
  union _GP_Reg_Union reg_union;

  regNumSrc = (pipeP->current_opcode&0x100)>>8;
  reg_union.guint64 = 0;

  if ( regNumSrc )
    {
      reg_union.gp_reg = MMR->B;
    }
  else
    {
      reg_union.gp_reg = MMR->A;
    }

  if ( reg_union.guint64 == 0 )
    {
      set_TC(MMR,1);
    }
  else
    {
      int bit31,bit32;

      bit31 = reg_union.guint64 >> 31;
      bit32 = reg_union.guint64 >> 32;

      bit31 = bit31 & 0x1;
      bit32 = bit32 & 0x1;

      if ( bit31 ^ bit32 )
	{
	  // 2 sign bits
	  set_TC(MMR,0);
	  reg_union.guint64 = reg_union.guint64 << 1;
	  if ( regNumSrc )
	    {
	      MMR->B = reg_union.gp_reg;
	    }
	  else
	    {
	      MMR->A = reg_union.gp_reg;
	    }

	}
      else
	{
	  set_TC(MMR,1);
	}
    }
}
/* Generates an array of Words that this opcode text generates or NULL
 * if it doesn't. There should not be any comments in the string
 */
static GPtrArray *machine_code(gchar *opcode_text)
{
  return NULL;
}


