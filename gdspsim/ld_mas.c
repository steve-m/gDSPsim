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
#include "multiplier.h"
#include "xymem.h"
#include "smem.h"
#include "shifter.h"

static GPtrArray *machine_code(gchar *opcode_text);
static void execute(struct _PipeLine *pipeP, struct _Registers *Reg);

static gchar *mask[]= { "101011rd xxxxyyyy" };
static gchar *opcode[] = { "LD x,d || MASr y,d" };
static gchar *comment[]= { "$(d) = $(r)($(d) - ($(y) * T)); $(d)=$(x) << 16" };

/* This definition is global because another routine will make have
 * an array that points to all the different instruction classes.
 */
Instruction_Class LD_MAS_Obj =
{
  "LD_MAS",
  NULL, // prefetch
  NULL, // fetch
  NULL, // decode
  xymem_read_stg1, // read_stg1 (access)
  xymem_read_stg2, // read_stg2 (read)
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
  int d,r;

  // A or B = Xmem << 16
  // input DB (2)
  // Reg
  // shift_mux Immediate (2)
  // Shift 16
  // Store in A or B (d)
  d = pipeP->current_opcode & 0x100 >> 8;
  shifter(2,Reg,2,16,d);

  // (d) = (d) + ( Xmem * T )
  // Xmux is from T (0)
  // Ymux is from CB (3)
  // Accumulate by subtracting from B or A ((d^1)+3)
  // Store in B or A with rounding ((d^1)+2*r)
  d = d^1;
  r = pipeP->current_opcode & 0x200 >> 9;
  multiplier(0,3,d+3,d+(2*r),Reg);
}

/* Generates an array of Words that this opcode text generates or NULL
 * if it doesn't. There should not be any comments in the string
 */
static GPtrArray *machine_code(gchar *opcode_text)
{
  return NULL;
}


