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

// Audit: Feb 23,2002:1 wkk

#include "c54_core.h"
#include "hardware.h"
#include <stdio.h>
#include "decode.h"
#include "instruct_help.h"

static void read_stg1(struct _PipeLine *pipeP, struct _Registers *Reg);
static GPtrArray *machine_code(gchar *opcode_text);

static gchar *mask[]=    { "111110z0 cccccccc llllllll llllllll" };
static gchar *opcode[] = { "BCz l,c" };
static gchar *comment[]= { "Branch $(z) $(l),$(c)" };

/* This definition is global because another routine will make have
 * an array that points to all the different instruction classes.
 */
Instruction_Class BC_Obj =
{
  "BC",
  NULL, // prefetch
  NULL, // fetch
  NULL, // decode
  read_stg1, // read_stg1 (access)
  NULL, // read_stg2 (read)
  NULL, // execute
  return_2, // number_words 
  NULL, // set_cycle_number
  1,
  mask,
  opcode,
  comment,
  machine_code
};

static void read_stg1(struct _PipeLine *pipeP, struct _Registers *Reg)
{
  // stall by 1 cycle
  if ( pipeP->cycles == 0 )
    Reg->Decode_Again = 1;

  if ( pipeP->cycles == 2 )
    {
      if ( pipeP->word_number == 1 )
	{
	  // Check condition codes
	  if ( check_condition(pipeP->current_opcode & 0xff) )
	    {
	      Reg->PC = Reg->IR;
	      
	      if ( (pipeP->current_opcode & 0x200) == 0 )
		{
		  Reg->Flush = Reg->Flush + 2;
		}
	    }
	}
    }
}

/* Generates an array of Words that this opcode text generates or NULL
 * if it doesn't. There should not be any comments in the string
 */
static GPtrArray *machine_code(gchar *opcode_text)
{
  return NULL;
}


