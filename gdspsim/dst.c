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

static GPtrArray *machine_code(gchar *opcode_text);
static int set_cycle_number(Word mach_code, int word_num);
static void read_stg2(struct _PipeLine *pipeP, struct _Registers *Reg);
static void decode(struct _PipeLine *pipeP, struct _Registers *Reg);
static void execute(struct _PipeLine *pipeP, struct _Registers *Reg);

static gchar *mask[]=    { "0100111s aaaaaaaa" };
static gchar *opcode[] = { "DST s,a" };
static gchar *comment[]= { "$(s)=$(a)" };

/* This definition is global because another routine will make have
 * an array that points to all the different instruction classes.
 */
Instruction_Class DST_Obj =
{
  "DST",
  NULL, // prefetch
  NULL, // fetch
  decode, // decode
  NULL, // read_stg1 (access)
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


static void decode(struct _PipeLine *pipeP, struct _Registers *Reg)
{
  if ( pipeP->total_words > 1 && pipeP->word_number == 1 )
    {
      pipeP->storage1 = Reg->IR;
    }
}

static void read_stg2(struct _PipeLine *pipeP, struct _Registers *Reg)
{
  Word DAB;
  
  printf("Read_Stg1 for dst 0x%x\n",pipeP->current_opcode);

  if ( pipeP->total_words > 1 )
    {
      // word_number counts down from total_words
      if ( pipeP->word_number == 1 )
	{
	  Reg->EAB = update_smem_2words(pipeP->current_opcode & 0xff , 
					pipeP->storage1 , Reg);
	}
    }
  else
    {
      // This updates the auxillary registers 
      Reg->EAB = update_smem(pipeP->current_opcode & 0xff , Reg);
      printf("Read_Stg1 for mar 0x%x DAB=0x%x\n",pipeP->current_opcode,DAB);
    }
}
static void execute(struct _PipeLine *pipeP, struct _Registers *Reg)
{
  Word word2write;
  union _GP_Reg_Union reg_union;
  
  if ( pipeP->current_opcode & 0x10000 )
    {
      reg_union.gp_reg = MMR->B;
    }
  else
    {
      reg_union.gp_reg = MMR->A;
    }

  if ( pipeP->cycle_number == 2 )
    {
      // Write high byte first
      word2write = reg_union.words.high;
      write_data_mem(Reg->EAB,word2write);
    }
  else
    {
      // write low byte
      word2write = reg_union.words.low; 
      write_data_mem(Reg->EAB,word2write);
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


