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
#include "multiplier.h"

static GPtrArray *machine_code(gchar *opcode_text);
static void read_stg1(struct _PipeLine *pipeP, struct _Registers *Reg);
static void read_stg2(struct _PipeLine *pipeP, struct _Registers *Reg);
static void execute(struct _PipeLine *pipeP, struct _Registers *Reg);

static gchar *mask[]=    
{ 
  "001101r1 aaaaaaaa",
  "111101sd 1000100r"
};
static gchar *opcode[] = 
{ 
  "MACA a,B",
  "MACA T,s,d"
};
static gchar *comment[]= 
{ 
  "$(B)=$(a) * A[32-16] + B; T=$(s)",
  "$(d)= T * A[32-16] + $(s)"
};

/* This definition is global because another routine will make have
 * an array that points to all the different instruction classes.
 */
Instruction_Class MACA_Obj =
{
  "MACA",
  NULL, // prefetch
  NULL, // fetch
  NULL, // decode
  read_stg1, // read_stg1 (access)
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

static void read_stg1(struct _PipeLine *pipeP, struct _Registers *Reg)
{
  if ( pipeP->opcode_subType == 0 )
    {
      smem_read_stg1(pipeP,Reg);
    }
}

static void read_stg2(struct _PipeLine *pipeP, struct _Registers *Reg)
{
  if ( pipeP->opcode_subType == 0 )
    {
      smem_read_stg2(pipeP,Reg);
    }
}

static void execute(struct _PipeLine *pipeP, struct _Registers *Reg)
{

  if ( pipeP->opcode_subType == 0 )
    {
      int B_plus_2R;

      B_plus_2R = 1 | ((pipeP->current_opcode & 200) >> 8 );
      multiplier(2,2,2,B_plus_2R,Reg );
      MMR->T = Reg->DB;
    }
  else
    {
      int AorB1,AorB2;

      AorB1 = (pipeP->current_opcode & 0x200 ) >> 9;
      AorB2 = ((pipeP->current_opcode & 0x100 ) >> 8) +
	(pipeP->current_opcode & 0x1 );

      multiplier(0,2,AorB1,AorB2,Reg);
    }
}


/* Generates an array of Words that this opcode text generates or NULL
 * if it doesn't. There should not be any comments in the string
 */
static GPtrArray *machine_code(gchar *opcode_text)
{
  return NULL;
}


