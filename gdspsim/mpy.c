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
    "001000rd aaaaaaaa",
    "1010010d xxxxyyyy",
    "0110001d aaaaaaaa nnnnnnnn nnnnnnnn",
    "1111000d 01100110 nnnnnnnn nnnnnnnn"
  };
static gchar *opcode[] = 
  {
    "MPYr a,d",
    "MPY x,y,d",
    "MPY a,#n,d",
    "MPY #n,d"
  };
static gchar *comment[]= 
  {
    "$(d)=$(a) * T",
    "$(d)=$(x) * $(y); T=$(x)",
    "$(d)=$(a) * $(n); T=$(a)",
    "$(d)=$(a) * $(n)"
 };

/* This definition is global because another routine will make have
 * an array that points to all the different instruction classes.
 */
Instruction_Class MPY_Obj =
{
  "MPY",
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
    case 0:
      // d = T * DB
      smem_read_stg1(pipeP,Reg);
      return;
    case 1:
      // T = Xmem
      // d = DB * CB + 0
      xymem_read_stg1(pipeP,Reg);
      return;
    case 2:
      // T = Smem
      // d = DB * P + 0
      if ( pipeP->word_number == 1 )
	pipeP->storage1 = Reg->IR;
      smem_read_stg1(pipeP,Reg);
      return;
    case 3:
      // d = T * P + 0
      if ( pipeP->word_number == 1 )
	pipeP->storage1 = Reg->IR;
      return;
    }
}

static void read_stg2(struct _PipeLine *pipeP, struct _Registers *Reg)
{
  switch ( pipeP->opcode_subType )
    {
    case 0:
      // d = T * DB
      smem_read_stg2(pipeP,Reg);
      return;
    case 1:
      // T = Xmem
      // d = DB * CB + 0
      xymem_read_stg2(pipeP,Reg);
      if ( pipeP->word_number == 1 )
	{
	  MMR->T = Reg->DB;
	}
      return;
    case 2:
      // T = Smem
      // d = DB * P + 0
      smem_read_stg2(pipeP,Reg);
      if ( pipeP->word_number == 1 )
	{
	  MMR->T = Reg->DB;
	  Reg->P = pipeP->storage1;
	}
      return;
    case 3:
      if ( pipeP->word_number == 1 )
	Reg->P = pipeP->storage1;
      // d = T * P + 0
      return;
    }

}
static void execute(struct _PipeLine *pipeP, struct _Registers *Reg)
{
  int d;

  if ( pipeP->word_number == 1 )
    {
      d = ( pipeP->current_opcode & 0x100 ) > 8;

      switch ( pipeP->opcode_subType )
	{
	case 0:
	  {
	    int r;
	    // d = T * DB
	    r = ( pipeP->current_opcode & 0x200 ) > 9;
	    
	    multiplier(0,1,0,d+(2*r),Reg);
	    return;
	  }
	case 1:
	  // T = Xmem
	  // d = DB * CB + 0
	  multiplier(1,3,0,d,Reg);

	  MMR->T = Reg->DB;
	  return;
	case 2:
	  // T = Smem
	  // d = DB * P + 0
	  multiplier(1,0,0,d,Reg);
	  MMR->T = Reg->DB;
	  return;
	case 3:
	  // d = T * P + 0
	  multiplier(0,0,0,d,Reg);
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
    case 2:
      return num_words_for_smem_plus1(pipeP);
    case 3:
      return 2;
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


