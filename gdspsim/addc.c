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

static void execute(struct _PipeLine *pipeP, struct _Registers *Reg);

static GPtrArray *machine_code(gchar *opcode_text);

static gchar *mask[]=    { "0000011s aaaaaaaa" };
static gchar *opcode[] = { "ADDC a,s" };
static gchar *comment[]= { "$(s)=$(a)+ $(s) + C" };

/* This definition is global because another routine will make have
 * an array that points to all the different instruction classes.
 */
Instruction_Class ADDC_Obj =
{
  "ADDC",
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
  union _GP_Reg_Union reg_union;
  union _GP_Reg_Union reg_union2;
  Word temp;

  if ( pipeP->current_opcode & 0x100 )
    reg_union.gp_reg = MMR->B;
  else
    reg_union.gp_reg = MMR->A;
  
  temp = Reg->DB + C_bit(MMR);

  reg_union2.gp_reg = alu_add(reg_union.gp_reg,temp,0,SXM(MMR));
  
  if ( pipeP->current_opcode & 0x100 )
    MMR->B = reg_union2.gp_reg;
  else
    MMR->A = reg_union2.gp_reg;

}

/* Generates an array of Words that this opcode text generates or NULL
 * if it doesn't. There should not be any comments in the string
 */
static GPtrArray *machine_code(gchar *opcode_text)
{
  return NULL;
}
