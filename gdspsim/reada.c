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
#include "memory.h"
#include "smem.h"

static void read_stg1(struct _PipeLine *pipeP, struct _Registers *Reg);
static void read_stg2(struct _PipeLine *pipeP, struct _Registers *Reg);
static void execute(struct _PipeLine *pipeP, struct _Registers *Reg);
static GPtrArray *machine_code(gchar *opcode_text);

static gchar *mask[]=    { "01111110 aaaaaaaa" };
static gchar *opcode[] = { "READA a" };
static gchar *comment[]= { "a=PROG(A)" };

/* This definition is global because another routine will make have
 * an array that points to all the different instruction classes.
 */
Instruction_Class READA_Obj =
{
  "READA",
  NULL, // prefetch
  NULL, // fetch
  NULL, // decode
  read_stg1, // read_stg1 (access)
  read_stg2, // read_stg2 (read)
  execute, // execute
  num_words_for_smem, // number_words 
  NULL, // set_cycle_number
  1,
  mask,
  opcode,
  comment,
  machine_code
};

static void read_stg1(struct _PipeLine *pipeP, struct _Registers *Reg)
{
  if ( pipeP->word_number == 1 )
    {
      if ( pipeP->total_words > 1 )
	{
	  // It's 2 words. Save the 2nd part of the word
	  pipeP->storage1 = Reg->IR;
	}

      if ( Reg->RC_first_pass || ( Reg->RC == 0 ) )
	{
	  union _GP_Reg_Union reg_union;
	  
	  // Reg->Dont_Decode = 2;
	  reg_union.gp_reg = MMR->A;
	  Reg->PAR = reg_union.address;
	}
      else
	{
	  Reg->PAR++;
	}
    }
}

static void read_stg2(struct _PipeLine *pipeP, struct _Registers *Reg)
{
  int wait_state;

  if ( pipeP->word_number == 1 )
    {
      smem_set_EAB(pipeP,Reg);
      Reg->DB=read_data_mem_long(Reg->PAR,&wait_state);
    }
}

static void execute(struct _PipeLine *pipeP, struct _Registers *Reg)
{
  if ( pipeP->word_number == 1 )
    {
      write_data_mem(Reg->EAB,Reg->DB);
    }
}

/* Generates an array of Words that this opcode text generates or NULL
 * if it doesn't. There should not be any comments in the string
 */
static GPtrArray *machine_code(gchar *opcode_text)
{
  return NULL;
}
