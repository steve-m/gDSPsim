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
    "001010rs aaaaaaaa",
    "10110rsd xxxxyyyy",
    "111100sd 01100111 nnnnnnnn nnnnnnnn",
    "011001sd aaaaaaaa nnnnnnnn nnnnnnnn"
  };
static gchar *opcode[] = 
  {
    "MACr a,s",
    "MACr x,y,s,d",
    "MAC #n,s,d",
    "MAC a,#n,s,d"
  };
static gchar *comment[]= 
  {
    "$(s)=$(a) * T + $(s)",
    "$(d)=$(x) * $(y) + $(s); T=$(x)",
    "$(d)=T * $(n) + $(s)",
    "$(d)=$(a) * $(n) + $(s); T=$(a)"
 };

/* This definition is global because another routine will make have
 * an array that points to all the different instruction classes.
 */
Instruction_Class MAC_Obj =
{
  "MAC",
  NULL, // prefetch
  NULL, // fetch
  NULL, // decode
  read_stg1, // read_stg1 (access)
  read_stg2, // read_stg2 (read)
  execute, // execute
  number_words, // number_words 
  NULL, // set_cycle_number
  4,
  mask,
  opcode,
  comment,
  machine_code
};

static void read_stg1(struct _PipeLine *pipeP, struct _Registers *Reg)
{
  switch ( pipeP->opcode_subType )
    {
    case 1:
      xymem_read_stg1(pipeP,Reg);
      break;
    }

}
static void read_stg2(struct _PipeLine *pipeP, struct _Registers *Reg)
{
  if ( pipeP->opcode_subType == 1 )
    xymem_read_stg1(pipeP,Reg);
}
static void execute(struct _PipeLine *pipeP, struct _Registers *Reg)
{
  if ( pipeP->opcode_subType == 1 )
    {
      int r,s,d;

      // X operand is from DB registers (1)
      // Y operand is from CB register (3)
      // Accumulate using A or B (s+1)
      // Store in A or B ( d ). Round (r)
      d = ( pipeP->current_opcode & 0x10000 ) > 16;
      s = ( pipeP->current_opcode & 0x20000 ) > 17;
      r = ( pipeP->current_opcode & 0x40000 ) > 18;
      
      multiplier(1,3,s+1,d+(2*r),Reg);
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
    case 2:
      return 2;
    case 3:
      return num_words_for_smem(pipeP)+1;
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


