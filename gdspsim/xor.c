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
#include "decode.h"

static void read_stg1(struct _PipeLine *pipeP, struct _Registers *Reg);
static void read_stg2(struct _PipeLine *pipeP, struct _Registers *Reg);
static void execute(struct _PipeLine *pipeP, struct _Registers *Reg);
static int number_words(struct _PipeLine *pipeP);
static GPtrArray *machine_code(gchar *opcode_text);

static gchar *mask[]=
{
  "0001110s aaaaaaaa",
  "111100sd 0101uuuu hhhhhhhh hhhhhhhh",
  "111100sd 01100101 hhhhhhhh hhhhhhhh",
  "111100sd 110nnnnn"
};
static gchar *opcode[] = 
{ 
  "XOR a,s",
  "XOR #h,u,s,d",
  "XOR #h,16,s,d",
  "XOR s,n,d"
};
static gchar *comment[]= 
{ 
  "$(s)=$(a) ^ $(s)[15-0]",
  "$(d)=$(h) << $(u) ^ $(s)",
  "$(d)=$(h) << 16 ^ $(s)",
  "$(d)=$(d) ^ $(s) << $(u)"
};

/* This definition is global because another routine will make have
 * an array that points to all the different instruction classes.
 */
Instruction_Class XOR_Obj =
{
  "XOR",
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
      smem_read_stg1(pipeP,Reg);
      return;
    case 1:
    case 2:
      if ( pipeP->word_number == 1 )
	pipeP->storage1 = Reg->IR;
      return;
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
  union _GP_Reg_Union reg_union;

  reg_union.guint64 = 0;
  if ( pipeP->word_number == 1 )
    {
      if (pipeP->opcode_subType == 0)
	{
	  if ( (pipeP->current_opcode & 0x100) != 0 )
	    {
	      reg_union.gp_reg = MMR->B;
	      reg_union.guint64 = reg_union.guint64 ^ Reg->DB;
	      MMR->B = reg_union.gp_reg;
	    }
	  else
	    {
	      reg_union.gp_reg = MMR->A;
	      reg_union.guint64 = reg_union.guint64 ^ Reg->DB;
	      MMR->A = reg_union.gp_reg;
	    }
	}
      else
	{
	  if ( (pipeP->current_opcode & 0x200) != 0 )
	    {
	      reg_union.gp_reg = MMR->B;
	    }
	  else
	    {
	      reg_union.gp_reg = MMR->A;
	    }
	  
	  switch (pipeP->opcode_subType)
	    {
	    case 1:
	      reg_union.guint64 = reg_union.guint64 ^
		(pipeP->storage1 << (pipeP->current_opcode & 0xf) );
	      break;
	    case 2:
	      reg_union.guint64 = reg_union.guint64 ^
		(pipeP->storage1 << 16 );
	      break;
	    case 3:
	      {
		union _GP_Reg_Union reg_union2;
		int shift;

		if ( (pipeP->current_opcode & 0x100) != 0 )
		  {
		reg_union2.gp_reg = MMR->B;
		  }
		else
		  {
		    reg_union2.gp_reg = MMR->A;
		  }
		
		shift = signed_bit_extract('n',mask[3],
					   pipeP->current_opcode,NULL);
		
		reg_union.guint64 = ( reg_union.guint64 << shift ) ^
		  reg_union2.guint64;
		
	      }
	    }
	  
	  if ( pipeP->current_opcode & 0x200)
	    {
	      MMR->B = reg_union.gp_reg;
	    }
	  else
	    {
	      MMR->A = reg_union.gp_reg;
	    }
	  
	}
    }
}

int number_words(struct _PipeLine *pipeP)
{
  switch (pipeP->opcode_subType)
    {
    case 0:
      return num_words_for_smem(pipeP);
    case 1:
    case 2:
      return 2;
    case 3:
    default:
      return 1;
    }
}

/* Generates an array of Words that this opcode text generates or NULL
 * if it doesn't. There should not be any comments in the string
 */
static GPtrArray *machine_code(gchar *opcode_text)
{
  return NULL;
}

