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
#include "decode.h"

static void execute(struct _PipeLine *pipeP, struct _Registers *Reg);
static void read_stg1(struct _PipeLine *pipeP, struct _Registers *Reg);
static void read_stg2(struct _PipeLine *pipeP, struct _Registers *Reg);
static GPtrArray *machine_code(gchar *opcode_text);
static int number_words(struct _PipeLine *pipeP);

static gchar *mask[]=
{
  "01000111 aaaaaaaa",
  "11101100 uuuuuuuu",
  "11110000 01110000 uuuuuuuu uuuuuuuu"
};

static gchar *opcode[] = 
{
  "RPT a",
  "RPT #u",
  "RPT #u"
};

static gchar *comment[]= 
{ 
  "RPT $(a)",
  "RPT $(u)",
  "RPT $(u)"
};

/* This definition is global because another routine will make have
 * an array that points to all the different instruction classes.
 */
Instruction_Class RPT_Obj =
{
  "RPT",
  NULL, // prefetch
  NULL, // fetch
  NULL, // decode
  read_stg1, // read_stg1 (access)
  read_stg2, // read_stg2 (read)
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
  switch ( pipeP->opcode_subType )
    {
    case 0:
      if ( pipeP->word_number == 1 )
	Reg->Dont_Decode=2;
      smem_read_stg1(pipeP,Reg);
      return;
    case 1:
      return;
    case 2:
      // always 2 words
      if ( pipeP->word_number == 1 )
	{
	  pipeP->storage1 = Reg->IR;
	}
      return;
    }
};

static void read_stg2(struct _PipeLine *pipeP, struct _Registers *Reg)
{
  if ( pipeP->opcode_subType == 0 )
    {
      smem_read_stg2(pipeP,Reg);
    }
};


static void execute(struct _PipeLine *pipeP, struct _Registers *Reg)
{
  switch ( pipeP->opcode_subType )
    {
    case 0:
      if ( pipeP->word_number == 1 )
	{
	  Reg->RC=Reg->DB;
	  Reg->RC_first_pass = 1;
	}
      return;
    case 1:
      {
	Word adjustment;

	adjustment = bit_extract('u',mask[1],pipeP->current_opcode,NULL);
	Reg->RC=adjustment;
      }
      return;
    case 2:
      if ( pipeP->word_number == 1 )
	{
	  Reg->RC=pipeP->storage1;
	}
      return;
    }
}

int number_words(struct _PipeLine *pipeP)
{
  switch ( pipeP->opcode_subType )
    {
    case 0:
      return num_words_for_smem(pipeP);
    default:
    case 1:
      return 1;
    case 2:
      return 2;
    }
}

/* Generates an array of Words that this opcode text generates or NULL
 * if it doesn't. There should not be any comments in the string
 */
static GPtrArray *machine_code(gchar *opcode_text)
{
  return NULL;
}

