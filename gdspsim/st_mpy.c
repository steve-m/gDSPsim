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
static void read_stg2(struct _PipeLine *pipeP, struct _Registers *Reg);
static void execute(struct _PipeLine *pipeP, struct _Registers *Reg);

static gchar *mask[]= { "110011sd xxxxyyyy" };
static gchar *opcode[] = { "ST s,y || MPY x,d" };
static gchar *comment[]= { "$(d) = $(x) * T); $(y)=$(s) << ASM-16" };

/* This definition is global because another routine will make have
 * an array that points to all the different instruction classes.
 */
Instruction_Class ST_MPY_Obj =
{
  "ST_MPY",
  NULL, // prefetch
  NULL, // fetch
  NULL, // decode
  xmem_read_stg1, // read_stg1 (access)
  read_stg2, // read_stg2 (read)
  execute, // execute
  NULL, // number_words 
  NULL, // set_cycle_number
  1,
  mask,
  opcode,
  comment,
  machine_code
};

static void read_stg2(struct _PipeLine *pipeP, struct _Registers *Reg)
{
  ymem_set_EAB(pipeP,Reg);
  xmem_read_stg2(pipeP,Reg);
}

static void execute(struct _PipeLine *pipeP, struct _Registers *Reg)
{
  int s,d;

  // Ymem = src << (ASM-16)
  // input A or B (s)
  // Reg
  // shift_mux ASM-16 (3)
  // Shift 0 (not used)
  // Store Low Bits to EAB (2)
  s = pipeP->current_opcode & 0x200 >> 9;
  shifter(s,Reg,3,0,2);

  // (d) = ( Xmem * T )
  // Xmux is from T (0)
  // Ymux is from DB (1)
  // Accumulate using 0 (0)
  // Store in A or B (d)
  d = pipeP->current_opcode & 0x100 >> 8;
  multiplier(0,1,0,d,Reg);
}

/* Generates an array of Words that this opcode text generates or NULL
 * if it doesn't. There should not be any comments in the string
 */
static GPtrArray *machine_code(gchar *opcode_text)
{
  return NULL;
}


