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
#include "memory.h"
#include "instruct_help.h"

static void read_stg1(struct _PipeLine *pipeP, struct _Registers *Reg);
static void read_stg2(struct _PipeLine *pipeP, struct _Registers *Reg);
static void execute(struct _PipeLine *pipeP, struct _Registers *Reg);
static GPtrArray *machine_code(gchar *opcode_text);

static gchar *mask[]=    { "01101001 aaaaaaaa hhhhhhhh hhhhhhhh" };
static gchar *opcode[] = { "ORM #h,a" };
static gchar *comment[]= { "$(a)=$(a) ^ $(h)" };

/* This definition is global because another routine will make have
 * an array that points to all the different instruction classes.
 */
Instruction_Class ORM_Obj =
{
  "ORM",
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

static void execute(struct _PipeLine *pipeP, struct _Registers *Reg)
{
  Word temp;

  if ( (pipeP->total_words == 3) && (pipeP->word_number==2) )
    Reg->EAB = pipeP->storage2;

  if ( pipeP->word_number == 1 )
    {
      temp = Reg->DB | pipeP->storage1;
      write_data_mem(Reg->EAB,temp);
    }
}

/* Generates an array of Words that this opcode text generates or NULL
 * if it doesn't. There should not be any comments in the string
 */
static GPtrArray *machine_code(gchar *opcode_text)
{
  return NULL;
}

