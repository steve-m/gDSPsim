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

#include "core_def.h"
#include "c54_core.h"
#include <stdio.h>
#include "pipeline.h"
#include "register_window.h"
#include "decode.h"
#include "memory.h"
#include "find_opcode.h"
#include "memory_window.h"
#include "decode_window.h"

extern Word *PM;

void default_registers(struct _Registers *Registers);
int find_opcode(Word *start_code, Word **next_code, GPtrArray *decode_info);
int set_pipe_decodeP(Word start_code, struct _Registers *Registers);
void flush_pipeline(struct _Registers *Registers);

static struct _PipeLine *pipe_decodeP=NULL;
static struct _PipeLine *pipe_read_stg1P=NULL;
static struct _PipeLine *pipe_read_stg2P=NULL;
static struct _PipeLine *pipe_executeP=NULL;


void pipeline(struct _Registers *Registers)
{
  int wait_state;
  int PC_stall=0;

  /* Propagate pipeline pointers */
  // fixme, need to just switch pointer not all values, have to 
  // do make changes in setting decode
  *pipe_executeP = *pipe_read_stg2P;
  *pipe_read_stg2P = *pipe_read_stg1P;
  *pipe_read_stg1P = *pipe_decodeP;
 
  if ( Registers->Flush )
    {
      flush_pipeline(Registers);

      Registers->Flush = 0;
    }
  else
    {


      printf("executing 0x%x %s\n",pipe_executeP->current_opcode,pipe_executeP->opcode_object->name);

      if ( pipe_executeP->opcode_object->execute )
	pipe_executeP->opcode_object->execute(pipe_executeP,Registers);
     
      // DB is loaded with the first read argument using DAB address
      // CB is loaded with the second read argument using CAB address
      // EAB is loaded with the write address
      printf("read_stg2 0x%x %s\n",pipe_read_stg2P->current_opcode,pipe_read_stg2P->opcode_object->name);
      if ( pipe_read_stg2P->opcode_object->read_stg2 )
	pipe_read_stg2P->opcode_object->read_stg2(pipe_read_stg2P,Registers);
      
      // This is where DAB and CAB buses are loaded with the read address
      // auxiliary registers and SP are also updated
      printf("read_stg1 0x%x %s\n",pipe_read_stg1P->current_opcode,pipe_read_stg1P->opcode_object->name);
      if ( pipe_read_stg1P->opcode_object->read_stg1 )
	pipe_read_stg1P->opcode_object->read_stg1(pipe_read_stg1P,Registers);
      
      // IR is loaded with contents of PB and IR is decoded
      Registers->IR = Registers->PB;
      // get new pipe_decodeP
      PC_stall = set_pipe_decodeP(Registers->IR,Registers);
      
      printf("decode 0x%x %s\n",pipe_decodeP->current_opcode,pipe_decodeP->opcode_object->name);
      if ( pipe_decodeP->opcode_object->decode )
	pipe_decodeP->opcode_object->decode(pipe_decodeP,Registers);
      
      // PB is loaded from PAB address
      if ( PC_stall == 0 )
	Registers->PB = read_program_mem(Registers->PAB,&wait_state);
    }
    
  if ( PC_stall == 0 )
    {
      // PAB is loaded with PC
      Registers->PAB = Registers->PC;

      Registers->PC = Registers->PC + 1;
    }

  highlight_pipeline(Registers->PAB);

  fill_reg_entries(Registers);
  update_all_memory_windows();
}


void default_registers(struct _Registers *Registers)
{
  Registers->PC=0x0;
  MMR->SP=0x1000;
  {
    union _GP_Reg_Union conv;
    conv.guint64 = 0x991234;
    MMR->A=conv.gp_reg;
  }
  {
    union _GP_Reg_Union conv;
    conv.guint64 = 0x123456789a; 
    MMR->B=conv.gp_reg;
  }
  MMR->ar0=0;
  MMR->ar1=0;
  MMR->ar2=0;
  MMR->ar3=0;
  MMR->ar4=0;
  MMR->ar5=0;
  MMR->ar6=0;
  MMR->ar7=0;
}

// Returns a PC read stall. Normally zero unless the last
// instruction was something like a 1 word opcode that takes
// 2 cycles such as a DST
int set_pipe_decodeP(Word start_code, struct _Registers *Registers)
{
  printf("decoding 0x%x\n",start_code);

  if ( pipe_decodeP->word_number > 1 )
    {
      *pipe_decodeP = *pipe_decodeP;
      pipe_decodeP->word_number--;
      return 0;
    }
  else if ( pipe_decodeP->cycle_number > 1 )
    {
      *pipe_decodeP = *pipe_decodeP;
      pipe_decodeP->cycle_number--;
      return 1;
    }

  else
    {
      printf("--decodeing 0x%x\n",start_code);
      pipe_decodeP->opcode_object = find_object(start_code,
						&pipe_decodeP->opcode_subType);

      pipe_decodeP->current_opcode = start_code;
      pipe_decodeP->cycles=0;
      printf("Decoded %s\n",pipe_decodeP->opcode_object->name);
      if ( pipe_decodeP->opcode_object->number_words )
	{
	  pipe_decodeP->word_number = 
	    pipe_decodeP->opcode_object->number_words(pipe_decodeP);
	  pipe_decodeP->total_words = 
	    pipe_decodeP->word_number;
	}
      else
	{
	  pipe_decodeP->word_number = 1;
	  pipe_decodeP->total_words = 1;
	}

      // Determine the number of pipeline cycles this word of the opcode
      // takes up. Used for things like DST where 1 word takes 2 cycles
      // to store the 32-bits.
      if ( pipe_decodeP->opcode_object->set_cycle_number )
	{
	  pipe_decodeP->cycle_number =
	    pipe_decodeP->opcode_object->
	    set_cycle_number(start_code,
			     pipe_decodeP->word_number);
	}
      else
	{
	  pipe_decodeP->cycle_number = 1;
	}
      
      return 0;
    }
  printf("Warning couldn't decode prefetched statement\n");
}

void flush_pipeline(struct _Registers *Registers)
{
  extern Instruction_Class NOP_Obj;
  extern Word NOP_opcode;

  pipe_decodeP->current_opcode = NOP_opcode;
  pipe_decodeP->opcode_object = &NOP_Obj;
  pipe_decodeP->opcode_subType=0;
  pipe_decodeP->cycles=1;
  pipe_decodeP->cycle_number = 1;
  pipe_decodeP->word_number = 1;
  pipe_decodeP->operands.op0.arP=NULL;
  pipe_decodeP->operands.op1.arP=NULL;
  pipe_decodeP->operands.op2.arP=NULL;
  pipe_decodeP->operands.op3.arP=NULL;

  pipe_read_stg1P->current_opcode = NOP_opcode;
  pipe_read_stg1P->opcode_object = &NOP_Obj;
  pipe_read_stg1P->opcode_subType=0;
  pipe_read_stg1P->cycles=1;
  pipe_read_stg1P->cycle_number = 1;
  pipe_read_stg1P->word_number = 1;
  pipe_read_stg1P->operands.op0.arP=NULL;
  pipe_read_stg1P->operands.op1.arP=NULL;
  pipe_read_stg1P->operands.op2.arP=NULL;
  pipe_read_stg1P->operands.op3.arP=NULL;

  pipe_read_stg2P->current_opcode = NOP_opcode;
  pipe_read_stg2P->opcode_object = &NOP_Obj;
  pipe_read_stg2P->opcode_subType=0;
  pipe_read_stg2P->cycles=1;
  pipe_read_stg2P->cycle_number = 1;
  pipe_read_stg2P->word_number = 1;
  pipe_read_stg2P->operands.op0.arP=NULL;
  pipe_read_stg2P->operands.op1.arP=NULL;
  pipe_read_stg2P->operands.op2.arP=NULL;
  pipe_read_stg2P->operands.op3.arP=NULL;

  pipe_executeP->current_opcode = NOP_opcode;
  pipe_executeP->opcode_object = &NOP_Obj;
  pipe_executeP->opcode_subType=0;
  pipe_executeP->cycles=1;
  pipe_executeP->cycle_number = 1;
  pipe_executeP->word_number = 1;
  pipe_executeP->operands.op0.arP=NULL;
  pipe_executeP->operands.op1.arP=NULL;
  pipe_executeP->operands.op2.arP=NULL;
  pipe_executeP->operands.op3.arP=NULL;

  Registers->PB = NOP_opcode;
  Registers->IR = NOP_opcode;

}

// Should be called once to setup
struct _Registers *pipe_new()
{
  struct _Registers *Registers;

  if ( pipe_decodeP == NULL )
    pipe_decodeP=g_new(struct _PipeLine,1);
  if ( pipe_read_stg1P == NULL )
    pipe_read_stg1P=g_new(struct _PipeLine,1);
  if ( pipe_read_stg2P == NULL )
    pipe_read_stg2P=g_new(struct _PipeLine,1);
  if ( pipe_executeP == NULL )
    pipe_executeP=g_new(struct _PipeLine,1);
  Registers=g_new(struct _Registers,1);

  Registers->Flush = 1;

  // Setup default values
  // Setup memory mapped registers by setting up memory
  fill_to_mem(0,0,0x80,PROGRAM_MEM_TYPE | DATA_MEM_TYPE );
  set_MMR_ptr();

  default_registers(Registers);


  return Registers;
}
  
