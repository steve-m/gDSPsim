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
void set_pipe_decodeP(Word start_code, WordA address, 
                      struct _Registers *Registers, int flags);
void flush_pipeline(struct _Registers *Registers);

static GArray *breakpoints=NULL;
static struct _PipeLine *pipe_decodeP=NULL;
static struct _PipeLine *pipe_read_stg1P=NULL;
static struct _PipeLine *pipe_read_stg2P=NULL;
static struct _PipeLine *pipe_executeP=NULL;

// list of fileIO break points
static GList *FileIOPipeLine=NULL;

/*
 *
 * Prefetch    Load PAB
 * Fetch       Read PB
 * Decode      Set IR   (set PC here for branch)
 * read_stg1   Set DAB, Set CAB
 * read_stg2   Set DB, Set CB, Set EAB
 * execute     Set EB
 *
 */

/*
 * a1 a2 B b1
 * a3    i3
 * a4    i4
 * ...
 * b1    j1
 *
 * Prefetch  PAB=a1 PAB=a2 PAB=a3 PAB=a4 PAB=b1
 * fetch            PB=B   PB=b1   PB=i3  PB=i4  PB=j1
 * decode                  IR=B     IR=b1                 IR=j1
 * read_stg1
 * read_stg2
 * execute   
 */

/* Registers->Flush
 * This is used for branching and XC, and is set the number
 * of words in the pipeline to flush. it is usually set in
 * the last word of read_stg1 and then the decode portion 
 * will be flushed in the same sample time of the simulation. 
 * The words will still be fetch, but discarded after the fetch
 *
 * Register->Dont_Decode
 * This is used to insert a pipeline stall. This is usually
 * set in the last word of read_stg1. It inserts some NOP
 * in place of what would have been a decode. The decode statement
 * is not lost.
 */

// Return 1 if breakpoint set, 0 if breakpoint removed
int toggle_breakpoint(WordA bp)
{
  int k;

  g_return_val_if_fail(breakpoints,0);
  
  for (k=0;k<breakpoints->len;k++)
    {
      if ( g_array_index(breakpoints,WordA,k) == bp )
	{
	  // Turn break point off
	  g_array_remove_index_fast(breakpoints,k);
	  return 0;
	}
    }
  
  g_array_append_val(breakpoints,bp);
  return 1;
}

void reset_view()
{
  update_all_memory_windows(0);
  unhighlight_pipeline();
}

void update_view()
{
  extern int decode_follow_pref;

  if ( decode_follow_pref == 0 )
    highlight_pipeline(0);
  else if ( decode_follow_pref == 1 )
    highlight_pipeline(pipe_executeP->decode_nfo.address);
  else if ( decode_follow_pref == 2 )
    highlight_pipeline(pipe_decodeP->decode_nfo.address);
  else if ( decode_follow_pref == 3 )
    highlight_pipeline(Registers->PC);

  fill_reg_entries(Registers);
  update_all_memory_windows(1);
}
  

// Return 1 if it hits a breakpoint
// Return 0, otherwise
int pipeline(struct _Registers *Registers)
{
  int wait_state;
  extern Word NOP_opcode;
  int return_val=0;
  int RC_set;

  /* Propagate pipeline pointers */
  // fixme, need to just switch pointer not all values, have to 
  // do make changes in setting decode
  *pipe_executeP = *pipe_read_stg2P;
  *pipe_read_stg2P = *pipe_read_stg1P;
  *pipe_read_stg1P = *pipe_decodeP;
 

  if ( Registers->Special_Flush )
    {
      // This is used to start the pipeline and if
      // the user changes the PC on his own. Not used
      // for simulation.
      flush_pipeline(Registers);

      Registers->Special_Flush = 0;

      // Prefetch
      // PAB is loaded with PC
      Registers->PAB = Registers->PC;
	  
    }
  else
    {
      if ( pipe_executeP->flags & 0x2 )
	{
	  printf("found breakpoint on execute\n");
	  return_val = 1;
	}
      
      RC_set = Registers->RC;

      if ( pipe_executeP->opcode_object->execute )
	{
	  pipe_executeP->opcode_object->execute(pipe_executeP,Registers);

	  // Check and process fileIO breakpoints
	  if ( (FileIOPipeLine) && (pipe_executeP->cycles==0) )
	    {
	      GList *list;
	      struct _fileIO *io;

	      list = FileIOPipeLine;

	      while (list)
		{
		  io = list->data;
		  if ( io->address_reached == pipe_executeP->decode_nfo.address)
		    fileIO_process(io);
		}
	    }
	}
     
      // DB is loaded with the first read argument using DAB address
      // CB is loaded with the second read argument using CAB address
      // EAB is loaded with the write address
      if ( pipe_read_stg2P->opcode_object->read_stg2 )
	pipe_read_stg2P->opcode_object->read_stg2(pipe_read_stg2P,Registers);
      
      // This is where DAB and CAB buses are loaded with the read address
      // auxiliary registers and SP are also updated
      if ( pipe_read_stg1P->opcode_object->read_stg1 )
	{
	  pipe_read_stg1P->opcode_object->read_stg1(pipe_read_stg1P,Registers);
	}

      if ( (Registers->RC != 0) && (RC_set==0) )
	{
	  // RC was set
	  Registers->RC_first_pass = 1;
	}


      // By convention Dont_Decode takes precedence over Decode_Again
      // both of which take precedence over Flush
      if ( Registers->Dont_Decode != 0 )
	{
	  set_pipe_decodeP(NOP_opcode,Registers->PAB,Registers,0);
	}	  
      else if ( Registers->Decode_Again != 0 )
        {
          pipe_decodeP->cycles++;
          
	  if ( pipe_decodeP->opcode_object->decode )
	    pipe_decodeP->opcode_object->decode(pipe_decodeP,Registers);
        }	  
      else if ( ( Registers->RC == 0) || (Registers->RC_first_pass) ) 
	{
          if ( Registers->Flush )
            {
              Registers->PB = NOP_opcode;
              Registers->IR = NOP_opcode;
              Registers->Flush--;
            }

	  // Decode

	  // IR is loaded with contents of PB and IR is decoded
	  Registers->IR = Registers->PB;
	  if (Registers->fetch_flags & 0x1)
	    {
	      Registers->fetch_flags &= MAX_WORD - 1;
	      Registers->fetch_flags |= 0x2;
	    }

          // get new pipe_decodeP
          set_pipe_decodeP(Registers->IR,Registers->PAB_last,
                           Registers,Registers->fetch_flags);
          

	  if ( pipe_decodeP->opcode_object->decode )
	    pipe_decodeP->opcode_object->decode(pipe_decodeP,Registers);
	  

	  // Fetch

	  // PB is loaded from PAB address
	  if ( Registers->Flush )
	    {
	      Registers->PB = NOP_opcode;
	    }
	  else
	    {
              int k;
              Registers->PB = read_program_mem(Registers->PAB,&wait_state);
              
              // Look for breakpoint
              Registers->fetch_flags = 0;
              for (k=0;k<breakpoints->len;k++)
                {
                  if ( g_array_index(breakpoints,WordA,k) == Registers->PAB )
                    {
                      
                      Registers->fetch_flags |= 1;
                    }
                }
              
	    }
      
    
	  // Prefetch
          // PAB is loaded with PC
          Registers->PAB_last = Registers->PAB;
          Registers->PAB = Registers->PC;
        }
    }

  update_pipeline(Registers->PAB);

  if ( (Registers->Dont_Decode == 0) && (Registers->Decode_Again == 0) &&
       ( ( Registers->RC == 0 ) || Registers->RC_first_pass ) ) 
    {
      if ( BRAF(MMR) && (Registers->PC == Registers->REA) )
        {
          // Repeat block mode and at the end of the block
	  if ( MMR->BRC == 0 )
	    {
	      set_BRAF(MMR,0);
	      Registers->PC = Registers->PC + 1;
	    }
	  else
	    {
	      MMR->BRC--;
	      Registers->PC = Registers->RSA;
	    }
        }
      else
        {
          Registers->PC = Registers->PC + 1;
        }

      if ( Registers->RC_first_pass )
	Registers->RC_first_pass = 0;
    }
  else
    {
      // By convention Dont_Decode takes precedence over Decode_Again
      // both of which take precedence over Flush
      if ( Registers->Dont_Decode )
	{
	  Registers->Dont_Decode--;
	}
      else if ( Registers->Decode_Again )
	{
	  Registers->Decode_Again--;
	}
      if ( Registers->RC )
	{
	  Registers->RC--;
	}
    }
  return return_val;
}


void default_registers(struct _Registers *Registers)
{
  union _GP_Reg_Union conv;

  MMR->SP=0x1000;

  conv.guint64 = 0;
  MMR->A=conv.gp_reg;
  MMR->B=conv.gp_reg;
  MMR->ST0 = 0x1800;
  MMR->ST1 = 0x2900;
  MMR->ar0=0;
  MMR->ar1=0;
  MMR->ar2=0;
  MMR->ar3=0;
  MMR->ar4=0;
  MMR->ar5=0;
  MMR->ar6=0;
  MMR->ar7=0;
  Registers->PC = 0;
  Registers->PAB = 0;
  Registers->PAB_last = 0;
  Registers->PB = 0;
  Registers->IR = 0;
  Registers->DAB = 0;
  Registers->CAB = 0;
  Registers->DB = 0;
  Registers->CB = 0;
  Registers->EAB = 0;
  Registers->PM = 0;
  Registers->PAR = 0;
  Registers->RC = 0;
  Registers->Flush = 0;
  Registers->Special_Flush = 0;
  Registers->RC_first_pass = 0;
  Registers->Dont_Decode = 0;
  Registers->Decode_Again = 0;
  Registers->fetch_flags = 0;
}

// Returns a PC read stall. Normally zero unless the last
// instruction was something like a 1 word opcode that takes
// 2 cycles such as a DST
void set_pipe_decodeP(Word start_code, WordA address, struct _Registers *Registers, int flags)
{
  //int length;

  pipe_decodeP->flags = flags;
  if ( pipe_decodeP->flags )
    printf("decodeP->flags=%d\n",flags);

  if ( pipe_decodeP->word_number > 1 )
    {
      pipe_decodeP->cycles++;
      *pipe_decodeP = *pipe_decodeP;
      pipe_decodeP->word_number--;
      return;
    }

  else
    {
      pipe_decodeP->decode_nfo.mach_code[0] = start_code;
      pipe_decodeP->decode_nfo.address = address;

      if ( find_object(&pipe_decodeP->decode_nfo) )
	{
          extern Word NOP_opcode;
	  
	  printf("Can't process 0x%x Setting it to NOP\n",start_code);
	  
          return set_pipe_decodeP(NOP_opcode,address,Registers,flags);
	}
      else
	{
	  pipe_decodeP->current_opcode = start_code;
	  pipe_decodeP->cycles=0;
	  pipe_decodeP->opcode_subType = pipe_decodeP->decode_nfo.sub_type;
          pipe_decodeP->word_number = pipe_decodeP->decode_nfo.length;
          pipe_decodeP->total_words = pipe_decodeP->decode_nfo.length;
          pipe_decodeP->opcode_object = pipe_decodeP->decode_nfo.class;
	  
	}
      return;
    }
  printf("Warning couldn't decode prefetched statement\n");
}

void flush_pipeline(struct _Registers *Registers)
{
  extern Instruction_Class NOP_Obj;
  extern Word NOP_opcode;

  pipe_decodeP->current_opcode = NOP_opcode;
  pipe_decodeP->opcode_object = &NOP_Obj;
  pipe_decodeP->cycles=0;
  pipe_decodeP->word_number = 1;
  pipe_decodeP->flags = 0;
  pipe_decodeP->decode_nfo.mach_code[0] = NOP_opcode;
  pipe_decodeP->decode_nfo.address = 0x0;
  find_object(&pipe_decodeP->decode_nfo);

  pipe_read_stg1P->current_opcode = NOP_opcode;
  pipe_read_stg1P->opcode_object = &NOP_Obj;
  pipe_read_stg1P->cycles=0;
  pipe_read_stg1P->word_number = 1;
  pipe_read_stg1P->flags = 0;
  pipe_read_stg1P->decode_nfo.mach_code[0] = NOP_opcode;
  pipe_read_stg1P->decode_nfo.address = 0x0;
  find_object(&pipe_read_stg1P->decode_nfo);

  pipe_read_stg2P->current_opcode = NOP_opcode;
  pipe_read_stg2P->opcode_object = &NOP_Obj;
  pipe_read_stg2P->cycles=0;
  pipe_read_stg2P->word_number = 1;
  pipe_read_stg2P->flags = 0;
  pipe_read_stg2P->decode_nfo.mach_code[0] = NOP_opcode;
  pipe_read_stg2P->decode_nfo.address = 0x0;
  find_object(&pipe_read_stg2P->decode_nfo);

  pipe_executeP->current_opcode = NOP_opcode;
  pipe_executeP->opcode_object = &NOP_Obj;
  pipe_executeP->cycles=0;
  pipe_executeP->word_number = 1;
  pipe_executeP->flags = 0;
  pipe_executeP->decode_nfo.mach_code[0] = NOP_opcode;
  pipe_executeP->decode_nfo.address = 0x0;
  find_object(&pipe_executeP->decode_nfo);

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


  // Setup default values
  // Setup memory mapped registers by setting up memory
  fill_to_mem(0,0,0x80,PROGRAM_MEM_TYPE | DATA_MEM_TYPE );
  set_MMR_ptr();

  default_registers(Registers);

  Registers->Special_Flush = 1;

  if ( breakpoints == NULL )
    breakpoints = g_array_new(FALSE,FALSE,sizeof(WordA));

  return Registers;
}
  
/* File IO functions */
void set_fileIO_break_on_pipeline(struct _fileIO *io)
{
  FileIOPipeLine = g_list_append(FileIOPipeLine,io);
}

void update_fileIO_break_on_pipeline(struct _fileIO *io)
{
  // Nothing to do, because only one list and no data is extracted
}

void remove_fileIO_break_on_pipeline(struct _fileIO *io)
{
  GList *list;

  list = FileIOPipeLine;

  while (list)
    {
       if ( io == list->data )
	 {
	   FileIOPipeLine = g_list_remove_link(FileIOPipeLine,list);
	   return;
	 }
      list=list->next;
    }
  printf("Programming Error! Bad Call. %s:%d\n",__FILE__,__LINE__);
}
