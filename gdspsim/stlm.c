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
#include "memory.h"

static void execute(struct _PipeLine *pipeP, struct _Registers *Reg);
static GPtrArray *machine_code(gchar *opcode_text);

static gchar *mask[]=    { "1000100s mmmmmmmm" };
static gchar *opcode[] = { "STLM s,m" };
static gchar *comment[]= { "$(m)=$(s)" };

/* This definition is global because another routine will make have
 * an array that points to all the different instruction classes.
 */
Instruction_Class STLM_Obj =
{
  "STLM",
  NULL, // prefetch
  NULL, // fetch
  NULL, // decode
  NULL, // read_stg1 (access)
  mmem_set_EAB, // read_stg2 (read)
  execute, // execute
  num_words_for_smem, // number_words 
  NULL, // set_cycle_number
  1,
  mask,
  opcode,
  comment,
  machine_code
};

/*
 * If this is direct addressing it stores the low bits of s (A,B regs) into
 * a (page 0 Memory Mapped Regs). If it's indirect it reads the address the
 * pointer points to and sets the upper 9 bits to 0 to get the page 0 address.
 */

static void execute(struct _PipeLine *pipeP, struct _Registers *Reg)
{
  union _GP_Reg_Union reg;
  
  if ( pipeP->current_opcode & 0x100 )
    {
      reg.guint64 = GP_REG_2_UINT64(MMR->B);
    }
  else
    {
      reg.guint64 = GP_REG_2_UINT64(MMR->A);
    }
  
  // Check for Saturation on Store
  if ( SST(MMR) )
    {
      if ( SXM(MMR)==1 )
	{
	  if (reg.gint64 > max_pos32)
	    {
	      reg.gint64 = max_pos32;
	    }
	  else if (reg.gint64 < max_neg32)
	    {
	      reg.gint64 = max_neg32;
	    }
	}
      else
	{
	  if (reg.guint64 > 0xffffffff )
	    {
	      reg.guint64 = 0xffffffff;
	    }
	}
    }

  write_data_mem(Reg->EAB,reg.words.low);
}

/* Generates an array of Words that this opcode text generates or NULL
 * if it doesn't. There should not be any comments in the string
 */
static GPtrArray *machine_code(gchar *opcode_text)
{
  return NULL;
}

