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
#include <stdio.h>
#include "memory.h"
#include "smem.h"

static void read_stg2(struct _PipeLine *pipeP, struct _Registers *Reg);
static void execute(struct _PipeLine *pipeP, struct _Registers *Reg);
static GPtrArray *machine_code(gchar *opcode_text);

static gchar *mask[]=    { "10001010 aaaaaaaa" };
static gchar *opcode[] = { "POPM a" };
static gchar *comment[]= { "$(a)=TOS, SP++" };

/* This definition is global because another routine will make have
 * an array that points to all the different instruction classes.
 */
Instruction_Class POPM_Obj =
{
  "POPM",
  NULL, // prefetch
  NULL, // fetch
  NULL, // decode
  NULL, // read_stg1 (access)
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
  int wait_state;

  pipeP->storage1 = read_data_mem(MMR->SP,&wait_state);
  MMR->SP++;
  Reg->EAB = pipeP->current_opcode & 0xff;
}

static void execute(struct _PipeLine *pipeP, struct _Registers *Reg)
{
  if ( pipeP->word_number == 1 )
    {
      Word temp_DP;

      // Make sure we write Data Page 0
      temp_DP=MMR->ST0 & ~0x1ff;
      write_data_mem(Reg->EAB,pipeP->storage1);
      MMR->ST0=(MMR->ST0 & ~0x1ff) | temp_DP;
    }
}

/* Generates an array of Words that this opcode text generates or NULL
 * if it doesn't. There should not be any comments in the string
 */
static GPtrArray *machine_code(gchar *opcode_text)
{
  return NULL;
}

