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
#include "xymem.h"
#include "shifter.h"
#include "decode.h"

static void execute(struct _PipeLine *pipeP, struct _Registers *Reg);
static GPtrArray *machine_code(gchar *opcode_text);

static gchar *mask[]=    { "111101sd 10000101" };
static gchar *opcode[] = { "ABS s,d" };
static gchar *comment[]= { "$(d)=ABS($(s))" };

/* This definition is global because another routine will make have
 * an array that points to all the different instruction classes.
 */
Instruction_Class ABS_Obj =
{
  "ABS",
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
  union _GP_Reg_Union reg_union;

  if ( pipeP->current_opcode & 0x200 )
    reg_union.gp_reg = MMR->B;
  else
    reg_union.gp_reg = MMR->A;

  if ( reg_union.gint64 < 0 )
    reg_union.gint64 = -reg_union.gint64;

  if ( pipeP->current_opcode & 0x100 )
    MMR->B = reg_union.gp_reg;
  else
    MMR->A = reg_union.gp_reg;
}

/* Generates an array of Words that this opcode text generates or NULL
 * if it doesn't. There should not be any comments in the string
 */
static GPtrArray *machine_code(gchar *opcode_text)
{
  return NULL;
}

