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
static GPtrArray *machine_code(gchar *opcode_text);

static gchar *mask[]=    
{ 
  "0001100s aaaaaaaa",
  "111100sd 0011uuuu hhhhhhhh hhhhhhhh",
  "111100sd 01100011 hhhhhhhh hhhhhhhh",
  "111100sd 100nnnnn"
};
static gchar *opcode[] = 
{ 
  "AND s,a",
  "AND #h,u,s,d",
  "AND #h,16,s,d",
  "AND s,n,d"
};
static gchar *comment[]= 
{
  "$(s)=$(s) & $(a)",
  "$(d)=$(s) & ($(h) << $(u)",
  "$(d)=$(s) & ($(h) << 16)",
  "$(d)=$(d) & ($(s) << $(n)"
};

/* This definition is global because another routine will make have
 * an array that points to all the different instruction classes.
 */
Instruction_Class AND_Obj =
{
  "AND",
  NULL, // prefetch
  NULL, // fetch
  NULL, // decode
  read_stg1, // read_stg1 (access)
  read_stg2, // read_stg2 (read)
  execute, // execute
  num_words_for_smem_plus1, // number_words 
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
    case 0: // AND Smem,src
      smem_read_stg1(pipeP,Reg);
      break;
    case 1: // AND #h,u,s,d
    case 2: // AND #h,16,s,d
      if ( pipeP->word_number == 1 )
	{
	  pipeP->storage1 = Reg->IR;
	}
      break;
    }
}


static void read_stg2(struct _PipeLine *pipeP, struct _Registers *Reg)
{

  if ( pipeP->opcode_subType == 0)
    {
      // AND Smem,src
      smem_read_stg2(pipeP,Reg);
    }
}

static void execute(struct _PipeLine *pipeP, struct _Registers *Reg)
{
  union _GP_Reg_Union reg_union; 
  union _GP_Reg_Union reg_union2; 

  if ( pipeP->word_number == 1 )
    {
      reg_union.guint64 = 0;
      reg_union2.guint64 = 0;
      switch ( pipeP->opcode_subType )
	{
	case 0: // AND Smem,src
	  if ( (pipeP->current_opcode & 0x100) == 0 )
	    reg_union.gp_reg = MMR->A;
	  else
	    reg_union.gp_reg = MMR->B;

	  reg_union.guint64 = reg_union.guint64 & Reg->DB;

	  if ( (pipeP->current_opcode & 0x100) == 0 )
	    MMR->A = reg_union.gp_reg;
	  else
	    MMR->B = reg_union.gp_reg;
	  break;
	case 1: // AND #h,u,s,d
	  if ( (pipeP->current_opcode & 0x200) == 0 )
	    reg_union.gp_reg = MMR->A;
	  else
	    reg_union.gp_reg = MMR->B;

	  reg_union2.words.low = pipeP->storage1;
	  reg_union2.guint64 = reg_union.guint64 << 
	    (pipeP->current_opcode & 0xf);
	  reg_union.guint64 = reg_union.guint64 & reg_union2.guint64;

	  if ( (pipeP->current_opcode & 0x100) == 0 )
	    MMR->A = reg_union.gp_reg;
	  else
	    MMR->B = reg_union.gp_reg;
	  break;
    case 2: // AND #h,16,s,d
	  if ( (pipeP->current_opcode & 0x200) == 0 )
	    reg_union.gp_reg = MMR->A;
	  else
	    reg_union.gp_reg = MMR->B;

	  reg_union2.words.high = pipeP->storage1;
	  reg_union.guint64 = reg_union.guint64 & reg_union2.guint64;

	  if ( (pipeP->current_opcode & 0x100) == 0 )
	    MMR->A = reg_union.gp_reg;
	  else
	    MMR->B = reg_union.gp_reg;
	  break;
	case 3:
	  {
	    SWord shift;

	    if ( (pipeP->current_opcode & 0x100) == 0 )
	      reg_union.gp_reg = MMR->A;
	    else
	      reg_union.gp_reg = MMR->B;
	    
	    if ( (pipeP->current_opcode & 0x200) == 0 )
	      reg_union2.gp_reg = MMR->A;
	    else
	      reg_union.gp_reg = MMR->B;
	    
	    shift = signed_5bit_extract(pipeP->current_opcode);
	    
	    reg_union2.guint64 = reg_union.guint64 & ( reg_union2.guint64 << shift);
	    
	    if ( (pipeP->current_opcode & 0x100) == 0 )
	      MMR->A = reg_union.gp_reg;
	    else
	      MMR->B = reg_union.gp_reg;
	    break;
	  }
      break;
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

