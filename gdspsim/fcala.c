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
#include "instruct_help.h"
#include "memory.h"

static void read_stg1(struct _PipeLine *pipeP, struct _Registers *Reg);
static GPtrArray *machine_code(gchar *opcode_text);

static gchar *mask[]=    { "111101zs 11100111" };
static gchar *opcode[] = { "FCALAz s" };
static gchar *comment[]= { "Branch $(z) $(s)" };

/* This definition is global because another routine will make have
 * an array that points to all the different instruction classes.
 */
Instruction_Class FCALA_Obj =
{
  "FCALA",
  NULL, // prefetch
  NULL, // fetch
  NULL, // decode
  read_stg1, // read_stg1 (access)
  NULL, // read_stg2 (read)
  NULL, // execute
  NULL, // number_words 
  NULL, // set_cycle_number
  1,
  mask,
  opcode,
  comment,
  machine_code
};

static void read_stg1(struct _PipeLine *pipeP, struct _Registers *Reg)
{
  union _GP_Reg_Union reg_union;

  MMR->SP--;
  if (pipeP->current_opcode & 0x200)
    {
      write_data_mem(MMR->SP,Reg->PC+3);
    }
  else
    {
      write_data_mem(MMR->SP,Reg->PC+1);
      Reg->Flush = Reg->Flush + 2;
    }

  MMR->SP--;
  write_data_mem(MMR->SP,Reg->XPC);
  
  if ( pipeP->current_opcode & 0x100 )
    reg_union.gp_reg = MMR->B;
  else
    reg_union.gp_reg = MMR->A;

  Reg->PC = reg_union.words.low;
  Reg->XPC = reg_union.words.high & 0x7f;

}

/* Generates an array of Words that this opcode text generates or NULL
 * if it doesn't. There should not be any comments in the string
 */
static GPtrArray *machine_code(gchar *opcode_text)
{
  return NULL;
}


