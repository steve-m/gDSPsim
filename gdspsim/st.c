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
#include "instruct_help.h"
#include "memory.h"
#include "xymem.h"
#include "shifter.h"
#include "decode.h"

static void read_stg1(struct _PipeLine *pipeP, struct _Registers *Reg);
static int number_words(struct _PipeLine *pipeP);
static void execute(struct _PipeLine *pipeP, struct _Registers *Reg);
static GPtrArray *machine_code(gchar *opcode_text);

static gchar *mask[]=    
{
  "10001100 aaaaaaaa",
  "10001101 aaaaaaaa",
  "01110110 aaaaaaaa hhhhhhhh hhhhhhhh"
};

static gchar *opcode[] = 
{ 
  "ST T,a",
  "ST TRN,a",
  "ST #h,a",
};

static gchar *comment[]= 
{ 
  "$(a)=T",
  "$(a)=TRN",
  "$(a)=$(h)"
};

/* This definition is global because another routine will make have
 * an array that points to all the different instruction classes.
 */
Instruction_Class ST_Obj =
{
  "ST",
  NULL, // prefetch
  NULL, // fetch
  NULL, // decode
  read_stg1, // read_stg1 (access)
  smem_set_EAB, // read_stg2 (read)
  execute, // execute
  number_words, // number_words 
  NULL, // set_cycle_number
  3,
  mask,
  opcode,
  comment,
  machine_code
};
static void read_stg1(struct _PipeLine *pipeP, struct _Registers *Reg)
{
  if ( (pipeP->opcode_subType == 2) && (pipeP->word_number==1) )
    {
      pipeP->storage1 = Reg->IR;
    }
}

/*
 * If this is direct addressing it stores the low bits of s (A,B regs) into
 * a (page 0 Memory Mapped Regs). If it's indirect it reads the address the
 * pointer points to and sets the upper 9 bits to 0 to get the page 0 address.
 */

static void execute(struct _PipeLine *pipeP, struct _Registers *Reg)
{
  if ( pipeP->word_number == 1 )
    {
      switch ( pipeP->opcode_subType )
	{
	case 0:
	  {
	    write_data_mem(Reg->EAB,MMR->T);
	    return;
	  }
	case 1: 
	  {
	    write_data_mem(Reg->EAB,MMR->TRN);
	    return;
	  }
	case 2:
	  {
	    write_data_mem(Reg->EAB,pipeP->storage1);
	    return;
	  }
	}
    }
}


int number_words(struct _PipeLine *pipeP)
{
  switch ( pipeP->opcode_subType )
    {
    case 0:
    case 1: 
    default:
      return num_words_for_smem(pipeP);
    case 2:
      return num_words_for_smem(pipeP)+1;
    }
}

/* Generates an array of Words that this opcode text generates or NULL
 * if it doesn't. There should not be any comments in the string
 */
static GPtrArray *machine_code(gchar *opcode_text)
{
  return NULL;
}

