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
#include "alu.h"
#include <stdio.h>
#include "smem.h"

static void execute(struct _PipeLine *pipeP, struct _Registers *Reg);

static GPtrArray *machine_code(gchar *opcode_text);

static gchar *mask[]=    { "0000111s aaaaaaaa" };
static gchar *opcode[] = { "SUBB a,s" };
static gchar *comment[]= { "$(s)=$(s) - Uns($(a)) - C" };

/* This definition is global because another routine will make have
 * an array that points to all the different instruction classes.
 */
Instruction_Class SUBB_Obj =
{
  "SUBB",
  NULL, // prefetch
  NULL, // fetch
  NULL, // decode
  smem_read_stg1, // read_stg1 (access)
  smem_read_stg2, // read_stg2 (read)
  execute, // execute
  num_words_for_smem, // number_words 
  NULL, // set_cycle_number
  1,
  mask,
  opcode,
  comment,
  machine_code
};


/* Executes the instruction. The operands are passed as pointers to
 * Registers. These pointers point to Registers in the Reg variable.
 * Flags may also be affected or cause different execution results,
 * therefore the Register pointer is also passed. Many instructions
 * have slightly different implementations, therefore type is passed.
 */

static void execute(struct _PipeLine *pipeP, struct _Registers *Reg)
{
  int borrow;

  borrow = C_bit(MMR) ^ 1;

  Reg->DB = Reg->DB - 1;

  alu(0,(pipeP->current_opcode & 0x100)>>8,(pipeP->current_opcode & 0x100)>>8,0x11,Reg);

}

/* Generates an array of Words that this opcode text generates or NULL
 * if it doesn't. There should not be any comments in the string
 */
static GPtrArray *machine_code(gchar *opcode_text)
{
  return NULL;
}
