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

static gchar *mask[]=     { "01111100 aaaaaaaa uuuuuuuu uuuuuuuu" };
static gchar *opcode[] =  { "MVPD u,a" };
static gchar *comment[] = { "$(a)=*$(u)" };

/* This definition is global because another routine will make have
 * an array that points to all the different instruction classes.
 */
Instruction_Class MVPD_Obj =
{
  "MVPD",
  NULL, // prefetch
  NULL, // fetch
  NULL, // decode
  read_stg1, // read_stg1 (access)
  read_stg2, // read_stg2 (read)
  execute, // execute
  num_words_for_smem_plus1, //num_words_for_smem_plus1, // number_words 
  NULL, // set_cycle_number
  1,
  mask,
  opcode,
  comment,
  machine_code
};

static void read_stg1(struct _PipeLine *pipeP, struct _Registers *Reg)
{
  if ( (pipeP->total_words - pipeP->word_number) == 1 )
    {
      if ( Reg->RC_first_pass || (Reg->RC == 0) )
	{
	  Reg->PAR = Reg->IR;
	}
    }
}

static void read_stg2(struct _PipeLine *pipeP, struct _Registers *Reg)
{
  int wait_state;

  smem_set_EAB(pipeP, Reg);
  if ( pipeP->word_number == 1 )
    {
      // Not sure how to do the bus

      pipeP->storage1 = read_program_mem(Reg->PAR,&wait_state);
      if ( Reg->RC )
	Reg->PAR++;
    }
}

static void execute(struct _PipeLine *pipeP, struct _Registers *Reg)
{
  if ( pipeP->word_number == 1 )
    {
      // Not sure how to do the bus

      write_data_mem(Reg->EAB,pipeP->storage1);
    }
}

/* Generates an array of Words that this opcode text generates or NULL
 * if it doesn't. There should not be any comments in the string
 */
static GPtrArray *machine_code(gchar *opcode_text)
{
  return NULL;
}


