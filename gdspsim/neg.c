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
#include "smem.h"
#include "instruct_help.h"

static void execute(struct _PipeLine *pipeP, struct _Registers *Reg);
static GPtrArray *machine_code(gchar *opcode_text);

static gchar *mask[]=    { "111101sd 10000100" };
static gchar *opcode[] = { "NEG s,d" };
static gchar *comment[]= { "$(d)=-$(s)" };

/* This definition is global because another routine will make have
 * an array that points to all the different instruction classes.
 */
Instruction_Class NEG_Obj =
{
  "NEG",
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
  union _GP_Reg_Union reg;

  if ( pipeP->current_opcode & 0x200 )
    {
      reg.guint64 = GP_REG_2_UINT64(MMR->B);
    }
  else
    {
      reg.guint64 = GP_REG_2_UINT64(MMR->A);
    }

  reg.gint64 = -reg.gint64;

  if ( reg.gint64 > 0 )
    {
      // Check for overflow, happens for negative of 0xff80000000
      // FIXME, not sure what happens for neg of 0xf000000000 or
      // negative of 0x0100000000
      if ( (reg.gint64 > max_pos32) )
	{
	  if ( pipeP->current_opcode & 0x100 )
	    {
	      set_OVB(MMR,1);
	    }
	  else
	    {
	      set_OVA(MMR,1);
	    }

	  if ( OVM(MMR) )
	    {
	      reg.guint64 = 0x7fffffff;
	    }
	}
    }

  if ( reg.gint64 == 0 )
    {
      set_C(MMR,1);
    }
  else
    {
      set_C(MMR,0);
    }

  if ( pipeP->current_opcode & 0x100 )
    {
      MMR->B = reg.gp_reg;
    }
  else
    {
      MMR->A = reg.gp_reg;
    }

}

/* Generates an array of Words that this opcode text generates or NULL
 * if it doesn't. There should not be any comments in the string
 */
static GPtrArray *machine_code(gchar *opcode_text)
{
  return NULL;
}

