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
#include "xymem.h"
#include "smem.h"
#include "shifter.h"

static GPtrArray *machine_code(gchar *opcode_text);
static void read_stg1(struct _PipeLine *pipeP, struct _Registers *Reg);
static void read_stg2(struct _PipeLine *pipeP, struct _Registers *Reg);
static void execute(struct _PipeLine *pipeP, struct _Registers *Reg);
static int set_cycle_number(Word mach_code, int word_num);



static gchar *mask[]=    { "0101011d aaaaaaaa"};
static gchar *opcode[] = { "DLD a,s" };
static gchar *comment[]= { "dbl($(s))=dbl($(a))" };

/* This definition is global because another routine will make have
 * an array that points to all the different instruction classes.
 */
Instruction_Class DLD_Obj =
{
  "LD",
  NULL, // prefetch
  NULL, // fetch
  NULL, // decode
  read_stg1, // read_stg1 (access)
  read_stg2, // read_stg2 (read)
  execute, // execute
  num_words_for_smem, // number_words 
  set_cycle_number, // set_cycle_number
  1,
  mask,
  opcode,
  comment,
  machine_code
};

static void read_stg1(struct _PipeLine *pipeP, struct _Registers *Reg)
{
      smem_read_stg1(pipeP,Reg);
}


static void read_stg2(struct _PipeLine *pipeP, struct _Registers *Reg)
{
      smem_read_stg2(pipeP,Reg);
}

static void execute(struct _PipeLine *pipeP, struct _Registers *Reg)
{
  union _GP_Reg_Union reg_union;

  // DLD *ARx,A

  reg_union.guint64 = 0;

  if ( pipeP->word_number == 1 )
    {
      reg_union.words.low = Reg->DB;
      if ( pipeP->current_opcode & 0x100 )
	MMR->B = reg_union.gp_reg;
      else
	MMR->A = reg_union.gp_reg;
    }
  else
    {
      reg_union.words.high = Reg->DB;
      if ( pipeP->current_opcode & 0x100 )
	MMR->B = reg_union.gp_reg;
      else
	MMR->A = reg_union.gp_reg;
    }

}

static int set_cycle_number(Word mach_code, int word_num)
{
  return 2;
}

/* Generates an array of Words that this opcode text generates or NULL
 * if it doesn't. There should not be any comments in the string
 */
static GPtrArray *machine_code(gchar *opcode_text)
{
  return NULL;
}


