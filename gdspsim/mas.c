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
    "001011rs aaaaaaaa",
    "10111rsd xxxxyyyy"
  };
static gchar *opcode[] = 
  {
    "MASr a,s",
    "MASr x,y,s,d"
  };
static gchar *comment[]= 
  {
    "$(s)=$(s) - T * $(a)",
    "$(d)=$(s) - $(x) * $(y); T=$(x)",
 };

/* This definition is global because another routine will make have
 * an array that points to all the different instruction classes.
 */
Instruction_Class MAS_Obj =
{
  "MAS",
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
  if ( pipeP->opcode_subType == 0)
    {
      smem_read_stg1(pipeP,Reg);
    }
  else
    xymem_read_stg1(pipeP,Reg);
}

static void read_stg2(struct _PipeLine *pipeP, struct _Registers *Reg)
{
  if ( pipeP->opcode_subType == 0 )
    smem_read_stg2(pipeP,Reg);
  else 
    xymem_read_stg2(pipeP,Reg);
}

static void execute(struct _PipeLine *pipeP, struct _Registers *Reg)
{
  if ( pipeP->opcode_subType == 0 )
    {
      int r,s,d;

      // X operand is from T register (0)
      // Y operand is from DB register (1)
      // Accumulate by subtracting from A or B (s+3)
      // Store in A or B ( d ). Round (r)
      d = ( pipeP->current_opcode & 0x100 ) > 8;
      s = ( pipeP->current_opcode & 0x200 ) > 9;
      r = ( pipeP->current_opcode & 0x400 ) > 10;
      
      multiplier(0,1,s+3,d+(2*r),Reg);
    }
  else if ( pipeP->opcode_subType == 1 )
    {
      int r,s,d;

      // X operand is from DB registers (1)
      // Y operand is from CB register (3)
      // Accumulate by subtracting from A or B (s+3)
      // Store in A or B ( d ). Round (r)
      d = ( pipeP->current_opcode & 0x100 ) > 8;
      s = ( pipeP->current_opcode & 0x200 ) > 9;
      r = ( pipeP->current_opcode & 0x400 ) > 10;
      
      multiplier(1,3,s+3,d+(2*r),Reg);
    }
}

int number_words(struct _PipeLine *pipeP)
{
  if ( pipeP->opcode_subType == 0 )
    {
      return num_words_for_smem(pipeP);
    }
  return 1;  
}


/* Generates an array of Words that this opcode text generates or NULL
 * if it doesn't. There should not be any comments in the string
 */
static GPtrArray *machine_code(gchar *opcode_text)
{
  return NULL;
}


