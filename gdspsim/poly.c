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
#include "smem.h"
#include "shifter.h"

static void execute(struct _PipeLine *pipeP, struct _Registers *Reg);
static GPtrArray *machine_code(gchar *opcode_text);

static gchar *mask[]=  { "00110110 aaaaaaaa" };
static gchar *opcode[] = { "POLY a" };
static gchar *comment[]=  { "A=Round(A[32:16] * T + B); B=$(a)<<16;" };

/* This definition is global because another routine will make have
 * an array that points to all the different instruction classes.
 */
Instruction_Class POLY_Obj =
{
  "POLY",
  NULL, // prefetch
  NULL, // fetch
  NULL, // decode
  smem_read_stg1, // read_stg1 (access)
  smem_read_stg2, // read_stg2 (read)
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
  
  // X operand is from T register (0)
  // Y operand is from A register (2)
  // Accumulate with B (2)
  // Store in A with Rounding ( 2 ).
  
  multiplier(0,2,2,2,Reg);

  // input DB (2)
  // Reg
  // Shift Immediate 2
  // Shift 16
  // Store in B (1)
  shifter(2,Reg,2,16,1);
}


/* Generates an array of Words that this opcode text generates or NULL
 * if it doesn't. There should not be any comments in the string
 */
static GPtrArray *machine_code(gchar *opcode_text)
{
  return NULL;
}


