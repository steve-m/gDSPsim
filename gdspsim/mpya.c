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
#include "multiplier.h"
#include "xymem.h"
#include "smem.h"

static void read_stg1(struct _PipeLine *pipeP, struct _Registers *Reg);
static void read_stg2(struct _PipeLine *pipeP, struct _Registers *Reg);
static void execute(struct _PipeLine *pipeP, struct _Registers *Reg);
static int number_words(struct _PipeLine *pipeP);

static GPtrArray *machine_code(gchar *opcode_text);

static gchar *mask[]=    
  {
    "00110001 aaaaaaaa",
    "1111010d 10001100"
  };
static gchar *opcode[] = 
  {
    "MPYA a",
    "MPYA d"
  };
static gchar *comment[]= 
  {
    "B = $(a) * A[32-16]; T=$(a)",
    "$(d)=T * A[32-16]"
 };

/* This definition is global because another routine will make have
 * an array that points to all the different instruction classes.
 */
Instruction_Class MPYA_Obj =
{
  "MPYA",
  NULL, // prefetch
  NULL, // fetch
  NULL, // decode
  read_stg1, // read_stg1 (access)
  read_stg2, // read_stg2 (read)
  execute, // execute
  number_words, // number_words 
  NULL, // set_cycle_number
  2,
  mask,
  opcode,
  comment,
  machine_code
};

static void read_stg1(struct _PipeLine *pipeP, struct _Registers *Reg)
{
  switch ( pipeP->opcode_subType )
    {
    case 0:
      // B = DB * A[32-16]
      // T = DB
      smem_read_stg1(pipeP,Reg);
      return;
    case 1:
      // d = T * A[32-16]
      return;
    }

}

static void read_stg2(struct _PipeLine *pipeP, struct _Registers *Reg)
{
  switch ( pipeP->opcode_subType )
    {
    case 0:
      // B = DB * A[32-16]
      // T = DB
      smem_read_stg2(pipeP,Reg);
      if ( pipeP->word_number == 1 )
	{
	  MMR->T = Reg->DB;
	}
      return;
    case 1:
      // d = T * A[32-16]
      return;
    }

}
static void execute(struct _PipeLine *pipeP, struct _Registers *Reg)
{

  switch ( pipeP->opcode_subType )
    {
    case 0:
      if ( pipeP->word_number == 1 )
	{
	  // B = DB * A[32-16]
	  // T = DB
	  multiplier(1,2,0,1,Reg);
	  return;
	}
    case 1:
      {
	int d;
	// d = T * A[32-16]
	d = ( pipeP->current_opcode & 0x100 ) > 8;
	multiplier(0,2,0,d,Reg);
	return;
      }
    }
}

int number_words(struct _PipeLine *pipeP)
{
  switch ( pipeP->opcode_subType )
    {
    case 0:
      return num_words_for_smem(pipeP);
    case 1:
      return 1;
    }
  printf("Error! Bad subtype %s:%d\n",__FILE__,__LINE__);
  return 1;  
}


/* Generates an array of Words that this opcode text generates or NULL
 * if it doesn't. There should not be any comments in the string
 */
static GPtrArray *machine_code(gchar *opcode_text)
{
  return NULL;
}


