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
#include "alu.h"

static void execute(struct _PipeLine *pipeP, struct _Registers *Reg);

static gchar *mask[]=    { "0101110d aaaaaaaa" };
static gchar *opcode[] = { "DSUBT a,d" };
static gchar *comment[]= { "$(d)=$(a)+T" };

Instruction_Class DSUBT_Obj =
{
  "DSUBT",
  NULL, // prefetch
  NULL, // fetch
  NULL, // decode
  lmem_read_stg1, // read_stg1 (access)
  lmem_read_stg2, // read_stg2 (read)
  execute, // execute
  NULL, // number_words 
  NULL, // set_cycle_number
  1,
  mask,
  opcode,
  comment,
  NULL // machine_code
};

/* Executes the instruction. The operands are passed as pointers to
 * Registers. These pointers point to Registers in the Reg variable.
 * Flags may also be affected or cause different execution results,
 * therefore the Register pointer is also passed. Many instructions
 * have slightly different implementations, therefore type is passed.
 */

static void execute(struct _PipeLine *pipeP, struct _Registers *Reg)
{
  if (pipeP->word_number == 1 )
    {
      alu( 1, 2, (pipeP->current_opcode & 0x100 )>>8, 10, Reg );
    }
}
