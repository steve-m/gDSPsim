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
#include "hardware.h"
#include <stdio.h>
#include "xymem.h"
#include "multiplier.h"

static GPtrArray *machine_code(gchar *opcode_text);
static void execute(struct _PipeLine *pipeP, struct _Registers *Reg);

static gchar *mask[]=    { "1111010d 10000111" };
static gchar *opcode[] = { "MIN d" };
static gchar *comment[]= { "$(d)=MIN(A,B)" };

/* This definition is global because another routine will make have
 * an array that points to all the different instruction classes.
 */
Instruction_Class MIN_Obj =
{
  "MIN",
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
  int AorB;
  union _GP_Reg_Union regA,regB;

  AorB = (pipeP->current_opcode & 0x100 ) >> 8;
  regA.gp_reg = MMR->A;
  regB.gp_reg = MMR->B;

  if ( regA.gint64 < regB.gint64 )
    {
      if ( AorB )
	MMR->B = regA.gp_reg;
      else
	MMR->A = regA.gp_reg;

      set_TC(MMR,0);
    }
  else
    {
      if ( AorB )
	MMR->B = regB.gp_reg;
      else
	MMR->A = regB.gp_reg;

      set_C(MMR,1);
    }
}


/* Generates an array of Words that this opcode text generates or NULL
 * if it doesn't. There should not be any comments in the string
 */
static GPtrArray *machine_code(gchar *opcode_text)
{
  return NULL;
}


