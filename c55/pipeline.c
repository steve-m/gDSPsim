/*
 * gDSPsim - GNU Digital Signal Processor Simulator
 *
 * Copyright (C) 2001-2002, Kerry Keal, kerry@industrialmusic.com
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

#include "chip_core.h"
#include <stdio.h>
#include "pipeline.h"
#include "register_window.h"
#include "decode.h"
#include "memory.h"
#include "find_opcode.h"
#include "memory_window.h"
#include "decode_window.h"

extern Word *PM;
Opcode NOP_opcode;
struct _Instruction_Class NOP_Obj;

void default_registers(struct _Registers *Registers);
int find_opcode(Word *start_code, Word **next_code, GPtrArray *decode_info);
void set_pipe_decodeP(WordA address, 
                      struct _Registers *Registers, int flags);
void set_pipe_decodeP_to_NOP(void);
void flush_pipeline(struct _Registers *Registers);
unsigned char read_instruction_cache(WordA address);

static GArray *breakpoints=NULL;
static struct _PipeLine *pipe_decodeP=NULL;
static struct _PipeLine *pipe_address_stgP=NULL;
static struct _PipeLine *pipe_access_1P=NULL;
static struct _PipeLine *pipe_access_2P=NULL;
static struct _PipeLine *pipe_read_stgP=NULL;
static struct _PipeLine *pipe_executeP=NULL;
static struct _PipeLine *pipe_writeP=NULL;
static struct _PipeLine *pipe_write_plusP=NULL;

// list of fileIO break points
static GList *FileIOPipeLine=NULL;

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
  int return_val=0;
  int RC_set;

  /* Propagate pipeline pointers */
  // fixme, need to just switch pointer not all values, have to 
  // do make changes in setting decode
  *pipe_write_plusP = *pipe_writeP;
  *pipe_writeP = *pipe_executeP;
  *pipe_executeP = *pipe_read_stgP;
  *pipe_read_stgP = *pipe_access_2P;
  *pipe_access_2P = *pipe_access_1P;
  *pipe_access_1P = *pipe_address_stgP;
  *pipe_address_stgP = *pipe_decodeP;
 

  if ( Registers->Special_Flush )
    {
      // This is used to start the pipeline and if
      // the user changes the PC on his own. Not used
      // for simulation.
      flush_pipeline(Registers);

      Registers->Special_Flush = 0;

      // Prefetch
      // PAB is loaded with PC
      // Registers->PAB = Registers->PC;
	  
    }
  else
    {
      if ( pipe_executeP->flags & 0x2 )
	{
	  printf("found breakpoint on execute\n");
	  return_val = 1;
	}
      
      RC_set = Registers->RC;

      if ( pipe_write_plusP->opcode_object->write_plus )
	pipe_executeP->opcode_object->write_plus(pipe_write_plusP,Registers);
     
      if ( pipe_write_plusP->opcode_object->write )
	pipe_executeP->opcode_object->write(pipe_writeP,Registers);
     
      if ( pipe_executeP->opcode_object->execute )
	pipe_executeP->opcode_object->execute(pipe_executeP,Registers);
     
      // DB is loaded with the first read argument using DAB address
      // CB is loaded with the second read argument using CAB address
      // EAB is loaded with the write address
      if ( pipe_read_stgP->opcode_object->read_stg )
	pipe_read_stgP->opcode_object->read_stg(pipe_read_stgP,Registers);
      
      // This is where DAB and CAB buses are loaded with the read address
      // auxiliary registers and SP are also updated
      if ( pipe_access_2P->opcode_object->access_2 )
	{
	  pipe_access_2P->opcode_object->access_2(pipe_access_2P,Registers);
	}

      if ( pipe_access_1P->opcode_object->access_1 )
	{
	  pipe_access_1P->opcode_object->access_1(pipe_access_1P,Registers);
	}

      if ( pipe_address_stgP->opcode_object->address_stg )
	{
	  pipe_address_stgP->opcode_object->address_stg(pipe_address_stgP,Registers);
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
	  set_pipe_decodeP_to_NOP();
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
              // Registers->PB = NOP_opcode;
              // Registers->IR = NOP_opcode;
              Registers->Flush--;
            }

	  // Decode

	  // IR is loaded with contents of PB and IR is decoded
	  // Registers->IR = Registers->PB;
	  if (Registers->fetch_flags & 0x1)
	    {
	      Registers->fetch_flags &= MAX_WORD - 1;
	      Registers->fetch_flags |= 0x2;
	    }

          // get new pipe_decodeP
          set_pipe_decodeP(Registers->PC,
                           Registers,Registers->fetch_flags);
          

	  if ( pipe_decodeP->opcode_object->decode )
	    pipe_decodeP->opcode_object->decode(pipe_decodeP,Registers);
	  

	  // Fetch

	  // PB is loaded from PAB address
	  if ( Registers->Flush )
	    {
	      //Registers->PB = NOP_opcode;
	    }
	  else
	    {
              int k;
              //Registers->PB = read_program_mem(Registers->PAB,&wait_state);
              
              // Look for breakpoint
              Registers->fetch_flags = 0;
              for (k=0;k<breakpoints->len;k++)
                {
                  if ( g_array_index(breakpoints,WordA,k) == Registers->PC )
                    {
                      
                      Registers->fetch_flags |= 1;
                    }
                }
              
	    }
      
    
	  // Prefetch
          // PAB is loaded with PC
          // Registers->PAB_last = Registers->PAB;
          // Registers->PAB = Registers->PC;
        }
    }

  update_pipeline(Registers->PC);

  if ( (Registers->Dont_Decode == 0) && (Registers->Decode_Again == 0) &&
       ( ( Registers->RC == 0 ) || Registers->RC_first_pass ) ) 
    {
      if ( BRAF(MMR) && ( (Registers->PC == MMR->REA0) ||
			  (Registers->PC == MMR->REA1) ) )
        {
          // Repeat block mode and at the end of the block
	  if ( (MMR->BRC0 == 0) && (MMR->BRC1 == 0) )
	    {
	      set_BRAF(MMR,0);
	      Registers->PC = MMR->RSA0;
	    }
	  else
	    {
	      if (MMR->BRC1 == 0)
		{
		  MMR->BRC0--;
		  Registers->PC = MMR->RSA0;
		}
	      else
		{
		  MMR->BRC1--;
		  Registers->PC = MMR->RSA1;
		}
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
  MMR->AC0=conv.gp_reg;
  MMR->AC1=conv.gp_reg;
  MMR->AC2=conv.gp_reg;
  MMR->AC3=conv.gp_reg;
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

// Reads instruction out of instruction cache starting at address
void set_pipe_decodeP(WordA address, struct _Registers *Registers, int flags)
{
  //int length;
  unsigned char start_code;

  pipe_decodeP->flags = flags;
  if ( pipe_decodeP->flags )
    printf("decodeP->flags=%d\n",flags);

#if 0
  if ( pipe_decodeP->word_number > 1 )
    {
      pipe_decodeP->cycles++;
      *pipe_decodeP = *pipe_decodeP;
      pipe_decodeP->word_number--;
      return;
    }
  else
#endif
    {
      start_code = read_instruction_cache(address);

      pipe_decodeP->decode_nfo.mach_code.bop[0] = start_code;
      pipe_decodeP->decode_nfo.address = address;

      if ( find_object(&pipe_decodeP->decode_nfo) )
	{
	  printf("Can't process 0x%x Setting it to NOP\n",start_code);
	  
          return set_pipe_decodeP_to_NOP();
	}
      else
	{
	  //pipe_decodeP->current_opcode = start_code;
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
  extern Opcode NOP_opcode;

  pipe_decodeP->decode_nfo.mach_code = NOP_opcode;
  pipe_decodeP->opcode_object = &NOP_Obj;
  pipe_decodeP->cycles=0;
  pipe_decodeP->word_number = 1;
  pipe_decodeP->flags = 0;
  pipe_decodeP->decode_nfo.address = 0x0;
  find_object(&pipe_decodeP->decode_nfo);

  pipe_address_stgP->decode_nfo.mach_code = NOP_opcode;
  pipe_address_stgP->opcode_object = &NOP_Obj;
  pipe_address_stgP->cycles=0;
  pipe_address_stgP->word_number = 1;
  pipe_address_stgP->flags = 0;
  pipe_address_stgP->decode_nfo.address = 0x0;
  find_object(&pipe_address_stgP->decode_nfo);

  pipe_access_1P->decode_nfo.mach_code = NOP_opcode;
  pipe_access_1P->opcode_object = &NOP_Obj;
  pipe_access_1P->cycles=0;
  pipe_access_1P->word_number = 1;
  pipe_access_1P->flags = 0;
  pipe_access_1P->decode_nfo.address = 0x0;
  find_object(&pipe_access_1P->decode_nfo);

  pipe_access_2P->decode_nfo.mach_code = NOP_opcode;
  pipe_access_2P->opcode_object = &NOP_Obj;
  pipe_access_2P->cycles=0;
  pipe_access_2P->word_number = 1;
  pipe_access_2P->flags = 0;
  pipe_access_2P->decode_nfo.address = 0x0;
  find_object(&pipe_access_2P->decode_nfo);

  pipe_read_stgP->decode_nfo.mach_code = NOP_opcode;
  pipe_read_stgP->opcode_object = &NOP_Obj;
  pipe_read_stgP->cycles=0;
  pipe_read_stgP->word_number = 1;
  pipe_read_stgP->flags = 0;
  pipe_read_stgP->decode_nfo.address = 0x0;
  find_object(&pipe_read_stgP->decode_nfo);

  pipe_executeP->decode_nfo.mach_code = NOP_opcode;
  pipe_executeP->opcode_object = &NOP_Obj;
  pipe_executeP->cycles=0;
  pipe_executeP->word_number = 1;
  pipe_executeP->flags = 0;
  pipe_executeP->decode_nfo.address = 0x0;
  find_object(&pipe_executeP->decode_nfo);

  pipe_writeP->decode_nfo.mach_code = NOP_opcode;
  pipe_writeP->opcode_object = &NOP_Obj;
  pipe_writeP->cycles=0;
  pipe_writeP->word_number = 1;
  pipe_writeP->flags = 0;
  pipe_writeP->decode_nfo.address = 0x0;
  find_object(&pipe_writeP->decode_nfo);

  pipe_write_plusP->decode_nfo.mach_code = NOP_opcode;
  pipe_write_plusP->opcode_object = &NOP_Obj;
  pipe_write_plusP->cycles=0;
  pipe_write_plusP->word_number = 1;
  pipe_write_plusP->flags = 0;
  pipe_write_plusP->decode_nfo.address = 0x0;
  find_object(&pipe_write_plusP->decode_nfo);


}

// Should be called once to setup
struct _Registers *pipe_new()
{
  struct _Registers *Registers;

  if ( pipe_decodeP == NULL )
    pipe_decodeP=g_new(struct _PipeLine,1);

  if ( pipe_address_stgP == NULL )
    pipe_address_stgP=g_new(struct _PipeLine,1);

  if ( pipe_access_1P == NULL )
    pipe_access_1P=g_new(struct _PipeLine,1);

  if ( pipe_access_2P == NULL )
    pipe_access_2P=g_new(struct _PipeLine,1);

  if ( pipe_read_stgP == NULL )
    pipe_read_stgP=g_new(struct _PipeLine,1);

  if ( pipe_executeP == NULL )
    pipe_executeP=g_new(struct _PipeLine,1);

  if ( pipe_writeP == NULL )
    pipe_writeP=g_new(struct _PipeLine,1);

  if ( pipe_write_plusP == NULL )
    pipe_write_plusP=g_new(struct _PipeLine,1);

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
  
unsigned char read_instruction_cache(WordA address)
{
  return 0;
}
void set_pipe_decodeP_to_NOP(void)
{
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
