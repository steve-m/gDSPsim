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
#include "alu.h"
#include "smem.h"

static void read_stg1(struct _PipeLine *pipeP, struct _Registers *Reg);
static void read_stg2(struct _PipeLine *pipeP, struct _Registers *Reg);
static void execute(struct _PipeLine *pipeP, struct _Registers *Reg);

static GPtrArray *machine_code(gchar *opcode_text);

static gchar *mask[]=    { "01101011 aaaaaaaa uuuuuuuu uuuuuuuu" };
static gchar *opcode[] = { "ADDM #u,a" };
static gchar *comment[]= { "$(a) = $(a) + $(u)" };

/* This definition is global because another routine will make have
 * an array that points to all the different instruction classes.
 */
Instruction_Class ADDM_Obj =
{
  "ADDM",
  NULL, // prefetch
  NULL, // fetch
  NULL, // decode
  read_stg1, // read_stg1 (access)
  read_stg2, // read_stg2 (read)
  execute, // execute
  num_words_for_smem_plus1, // number_words 
  NULL, // set_cycle_number
  1,
  mask,
  opcode,
  comment,
  machine_code
};

static void read_stg1(struct _PipeLine *pipeP, struct _Registers *Reg)
{
  smem_read_stg1(pipeP,Reg);
  if ( (pipeP->total_words - pipeP->word_number) == 1 )
    {
      pipeP->storage2 = Reg->DAB; // Save for EAB
    }
  if ( pipeP->word_number == 1 )
    {
      pipeP->storage1 = Reg->IR;
    }
}

static void read_stg2(struct _PipeLine *pipeP, struct _Registers *Reg)
{
  smem_read_stg2(pipeP,Reg);
  if ( (pipeP->total_words == 2) && (pipeP->word_number==1) )
    Reg->EAB = pipeP->storage2;

}

/* Executes the instruction. The operands are passed as pointers to
 * Registers. These pointers point to Registers in the Reg variable.
 * Flags may also be affected or cause different execution results,
 * therefore the Register pointer is also passed. Many instructions
 * have slightly different implementations, therefore type is passed.
 */

static void execute(struct _PipeLine *pipeP, struct _Registers *Reg)
{
  Word CB;

  if ( (pipeP->total_words == 3) && (pipeP->word_number==2) )
    Reg->EAB = pipeP->storage2;

  if ( pipeP->word_number == 1 )
    {
      
      // Add Reg->DB to pipeP->storage1 and store in Reg->EAB
      // temporarily move pipeP->storage1 to Reg->CB
      CB = Reg->CB;
      CB = pipeP->storage1;
      alu(0,3,2,0,Reg);
      Reg->CB = CB;
    }
}

/* Generates an array of Words that this opcode text generates or NULL
 * if it doesn't. There should not be any comments in the string
 */
static GPtrArray *machine_code(gchar *opcode_text)
{
  return NULL;
}
