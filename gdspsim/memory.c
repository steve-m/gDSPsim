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

#include "memory.h"
#include "stdio.h"
#include "memory_window.h"
#include "preferences.h"

struct _def_mem
{
  WordP start;
  WordP end;
  int wait_state;
  MemType type;
  Word *mem;
};

struct _fileIO_break
{
  WordP address;
  MemType type;
  struct _fileIO *io;
};

GList *ProgMemList=NULL;
GList *DataMemList=NULL;
GList *DataProgMemList=NULL;

GList *FileReadBreak=NULL;
GList *FileWriteBreak=NULL;

/* Read data memory and determine it's wait state. Generally on chip RAM has
 * a wait state of 0 and external RAM has a wait state of 1 or more. This
 * procedure takes into account all the memory mapping needed to get the
 * correct value */
// This will need to be more complicated soon, that's why it's
// broken out into a separate file.
Word read_mem(WordP offset, int *wait_state, MemType type, int *available)
{
  GList *list;
  struct _def_mem *mem_page;
  Word *mem_element;
  struct _fileIO_break *fIObrk;
  
  list = FileReadBreak;
  while ( list )
    {
      fIObrk = list->data;
      if ( (fIObrk->address = offset) && (fIObrk->type == type) )
	{
	  // time to do fileIO
	  fileIO_process(fIObrk->io);
	}
      list = list->next;
    }

  *available = 1;
  set_mem_changed(offset,1);
  if ( (type & DATA_MEM_TYPE) || (type & PROGRAM_MEM_TYPE) )
    {
      
      for(list=DataProgMemList;list;list=list->next)
	{
	  mem_page = (struct _def_mem *)list->data;
	  if ( ( mem_page->start <= offset ) &&
	       ( mem_page->end >= offset ) )
	    {
	      *wait_state = mem_page->wait_state;
	      mem_element = mem_page->mem + (offset-mem_page->start);
	      return *mem_element;
	    }
	}
    }

  if ( type & DATA_MEM_TYPE )
    {
      
      for(list=DataMemList;list;list=list->next)
	{
	  mem_page = (struct _def_mem *)list->data;
	  if ( ( mem_page->start <= offset ) &&
	       ( mem_page->end >= offset ) )
	    {
	      *wait_state = mem_page->wait_state;
	      mem_element = mem_page->mem + (offset-mem_page->start);
	      return *mem_element;
	    }
	}
    }
  if ( type & PROGRAM_MEM_TYPE )
    {
      
      for(list=ProgMemList;list;list=list->next)
	{
	  mem_page = (struct _def_mem *)list->data;
	  if ( ( mem_page->start <= offset ) &&
	       ( mem_page->end >= offset ) )
	    {
	      *wait_state = mem_page->wait_state;
	      mem_element = mem_page->mem + (offset-mem_page->start);
	      return *mem_element;
	    }
	}
    }

  if ( type == PROGRAM_MEM_TYPE )
    {
      if ( bound_program_mem_read && (gStopRun==0))
	{
	  gStopRun=1;
	}
    }
  else if ( type == DATA_MEM_TYPE )
    {
      if ( bound_data_mem_read && (gStopRun==0))
	{
	  gStopRun=1;
	}
      if ( add_data_mem_on_read )
	{
	  // This will put this address in avalible memory
	  create_mem(0,offset,1,type);
	  *wait_state=0;
	  return 0;
	}
    }

  *wait_state=0;
  *available = 0; // Hasn't been written too
  
  return 0;
}

/* Read data memory and determine it's wait state. Generally on chip RAM has
 * a wait state of 0 and external RAM has a wait state of 1 or more. This
 * procedure takes into account all the memory mapping needed to get the
 * correct value */
// This will need to be more complicated soon, that's why it's
// broken out into a separate file.
int write_mem(WordP offset, Word value, MemType type)
{
  GList *list;
  int wait_state;
  struct _def_mem *mem_page;
  Word *mem_element;
  struct _fileIO_break *fIObrk;
  
  list = FileWriteBreak;
  while ( list )
    {
      fIObrk = list->data;
      if ( (fIObrk->address = offset) && (fIObrk->type == type) )
	{
	  // time to do fileIO
	  fileIO_process(fIObrk->io);
	}
      list = list->next;
    }

  // This memory address has been written to
  set_mem_changed(offset,0);
  if ( DataProgMemList && 
       ((type & DATA_MEM_TYPE) || (type & PROGRAM_MEM_TYPE) ) )
    {
      
      for(list=DataProgMemList;list;list=list->next)
	{
	  mem_page = (struct _def_mem *)list->data;
	  if ( ( mem_page->start <= offset ) &&
	       ( mem_page->end >= offset ) )
	    {
	      wait_state = mem_page->wait_state;
	      mem_element = mem_page->mem + (offset-mem_page->start);
	      *mem_element = value;
	      return wait_state;
	    }
	}
    }

  if ( DataMemList && (type & DATA_MEM_TYPE) )
    {
      
      for(list=DataMemList;list;list=list->next)
	{
	  mem_page = (struct _def_mem *)list->data;
	  if ( ( mem_page->start <= offset ) &&
	       ( mem_page->end >= offset ) )
	    {
	      wait_state = mem_page->wait_state;
	      mem_element = mem_page->mem + (offset-mem_page->start);
	      *mem_element = value;
	      return wait_state;
	    }
	}
    }

  if ( ProgMemList && (type & PROGRAM_MEM_TYPE) )
    {
      
      for(list=ProgMemList;list;list=list->next)
	{
	  mem_page = (struct _def_mem *)list->data;
	  if ( ( mem_page->start <= offset ) &&
	       ( mem_page->end >= offset ) )
	    {
	      wait_state = mem_page->wait_state;
	      mem_element = mem_page->mem + (offset-mem_page->start);
	      *mem_element = value;
	      return wait_state;
	    }
	}
    }

  if ( (type==DATA_MEM_TYPE) && (add_data_mem_on_write) )
    {
      // This will put this address in avalible memory
      create_mem(&value,offset,1,type);
      return 0;
    }

  printf("Memory access violation writing 0x%x of type=%d\n",offset,type);
    
  return 0;
}

int write_program_mem(Word offset, Word value)
{
  WordP address;

  address = PAGE() | offset;

  return write_mem(address,value,PROGRAM_MEM_TYPE);
}
int write_data_mem(Word offset, Word value)
{
  WordP address;

  address = PAGE() | offset;

  return write_mem(address,value,DATA_MEM_TYPE);
}

int write_program_mem_long(WordP offset, Word value)
{
  return write_mem(offset,value,PROGRAM_MEM_TYPE);
}
int write_data_mem_long(WordP offset, Word value)
{
  return write_mem(offset,value,DATA_MEM_TYPE);
}

Word read_data_mem(Word offset, int *wait_state)
{
  int available;
  WordP address;

  address = PAGE() | offset;
  return read_mem(address,wait_state,DATA_MEM_TYPE,&available);
  if ( !available )
    printf("Warning trying to access invalid memory at address 0x%x\n",offset);
}

PWord read_program_mem(Word offset, int *wait_state)
{
  int available;
  WordP address;
  Word word;
  PWord pwrd;

  address = PAGE() | offset;
  word = read_mem(PADDR_TO_ADDR(address),wait_state,PROGRAM_MEM_TYPE,&available);
  if ( !available )
    printf("Warning trying to access invalid memory at address 0x%x\n",offset);

  pwrd = WORD_TO_PWORD(word,address);

  return pwrd;
}

Word read_data_mem_long(WordP offset, int *wait_state)
{
  int available;
  return read_mem(offset,wait_state,DATA_MEM_TYPE,&available);
  if ( !available )
    printf("Warning trying to access invalid memory at address 0x%x\n",offset);
}

Word read_program_mem_long(WordP offset, int *wait_state)
{
  int available;
  return read_mem(offset,wait_state,PROGRAM_MEM_TYPE,&available);
  if ( !available )
    printf("Warning trying to access invalid memory at address 0x%x\n",offset);
}

// Debug function to print defined memory
void print_mem_list(void)
{
  struct _def_mem *def_mem;
  GList *list;

  if ( DataProgMemList )
    {
      for (list=DataProgMemList;list;list=list->next)
	{
	  def_mem = (struct _def_mem *)list->data;
	  printf("DataProg MemUsed = 0x%x to 0x%x mem=0x%x\n",def_mem->start,def_mem->end,(unsigned int)def_mem->mem);
	}
    }
  if ( DataMemList )
    {
      for (list=DataMemList;list;list=list->next)
	{
	  def_mem = (struct _def_mem *)list->data;
	  printf("Data    MemUsed = 0x%x to 0x%x mem=0x%x\n",def_mem->start,def_mem->end,(unsigned int)def_mem->mem);
	}
    }
  if ( ProgMemList )
    {
      for (list=ProgMemList;list;list=list->next)
	{
	  def_mem = (struct _def_mem *)list->data;
	  printf("Prog     MemUsed = 0x%x to 0x%x mem=0x%x\n",def_mem->start,def_mem->end,(unsigned int)def_mem->mem);
	}
    }

}

static WordP default_start_view=0x80;
static WordP default_end_view=0x100;
void set_prog_mem_start_end(WordP start, WordP end)
{
  default_start_view=start;
  default_end_view=MIN(end,start+0x200);
}

void get_prog_mem_start_end(WordP *start, WordP *end)
{
  *start=default_start_view;
  *end=default_end_view;
}

/* Creates a node to represent a memory block and inserts it into the
 * memory list (memlist) at the correct spot. Memory to hold the memory
 * block data size is allocated if needed. No overlapping memory is 
 * allocated, so multiple links may be created.
 */
static GList *insert_mem_list( GList *memlist, WordP start, WordP end, MemType type, int wait_state)
{
  struct _def_mem *def_mem,*def_mem2,*def_mem_prev;
  GList *list,*list2;

  if ( memlist == NULL )
    {
      // create link
      def_mem = g_new(struct _def_mem,1);
      def_mem->start=start;
      def_mem->end=end;
      def_mem->type=type;
      def_mem->wait_state=wait_state;
      def_mem->mem = g_new(Word,end-start+1);

      list2 = g_list_prepend(NULL, def_mem);
      return list2;
    }

  // Insert the new section by sorting on start
  for (list=memlist;list;list=list->next)
    {
      def_mem2 = (struct _def_mem *)list->data;

      if ( start == def_mem2->start )
        {
          if ( end <= def_mem2->end )
            {
              // already allocated
              return memlist;
            }
          else
            {
              // partially allocated
              start = def_mem2->end + 1;
            }
        }
      else if ( start <= def_mem2->start )
	{
          // the new link belongs before this current link
          if ( list->prev )
            {
              def_mem_prev = (struct _def_mem *)list->prev->data;

              if ( (def_mem2->start - def_mem_prev->end) == 1 )
                {
                  // no room between current and previous link
                  if ( end <= def_mem2->end )
                    {
                      // already allocated
                      return memlist;
                    }
                  start = def_mem2->end + 1;
                }
              else
                {
                  // there is room before the current and previous
                  // link that should be allocated

                  if ( start <= def_mem_prev->end )
                    {
                      // some of the block is allocated in previous link
                      if ( end >= def_mem2->start )
                        {
                          // all of the space between the current and
                          // previous link should be allocated

                          def_mem = g_new(struct _def_mem,1);
                          def_mem->start=def_mem_prev->end+1;
                          def_mem->end=def_mem2->start-1;
                          def_mem->type=type;
                          def_mem->wait_state=wait_state;
                          def_mem->mem = g_new(Word,def_mem->end-def_mem->start+1);
                          
                          list2 = g_list_prepend(list, def_mem);
                          
                          if ( end <= def_mem2->end )
                            {
                              // we have allocated enough
                              return memlist;
                            }
                          else
                            {
                              // more to allocate
                              start = def_mem2->end + 1;
                            }
                        }
                      else
                        {
                          // some of the block is allocated in previous
                          // link and there is enough room before the
                          // current link

                          def_mem = g_new(struct _def_mem,1);
                          def_mem->start=def_mem_prev->end+1;
                          def_mem->end=end;
                          def_mem->type=type;
                          def_mem->wait_state=wait_state;
                          def_mem->mem = g_new(Word,def_mem->end-def_mem->start+1);
                          
                          list2 = g_list_prepend(list, def_mem);
                          return memlist;
                        }
                    }
                  else
                    {
                      // the desired allocation is after the end of the 
                      // previous link
                       if ( end >= def_mem2->start )
                         {
                          def_mem = g_new(struct _def_mem,1);
                          def_mem->start=start;
                          def_mem->end=def_mem2->start-1;
                          def_mem->type=type;
                          def_mem->wait_state=wait_state;
                          def_mem->mem = g_new(Word,def_mem->end-def_mem->start+1);
                          
                          list2 = g_list_prepend(list, def_mem);

                          if ( end <= def_mem2->end )
                            {
                              return memlist;
                            }
                          else
                            {
                              start = def_mem2->end+1;
                            }
                         }
                       else
                         {
                           // room for the whole allocation
                           
                           def_mem = g_new(struct _def_mem,1);
                           def_mem->start=start;
                           def_mem->end=end;
                           def_mem->type=type;
                           def_mem->wait_state=wait_state;
                           def_mem->mem = g_new(Word,def_mem->end-def_mem->start+1);
                          
                           list2 = g_list_prepend(list, def_mem);
                           return memlist;
                        }
                    }
                }
            }
          else // if ( list->prev )
            {
              // no previous link 
              if ( end < def_mem2->start )
                {
                  // enough room in front
                  def_mem = g_new(struct _def_mem,1);
                  def_mem->start=start;
                  def_mem->end=end;
                  def_mem->type=type;
                  def_mem->wait_state=wait_state;
                  def_mem->mem = g_new(Word,def_mem->end-def_mem->start+1);
                  
                  return g_list_prepend(list, def_mem);
                }
              else
                {
                  // some overlap with current link
                  def_mem = g_new(struct _def_mem,1);
                  def_mem->start=start;
                  def_mem->end=def_mem2->start-1;
                  def_mem->type=type;
                  def_mem->wait_state=wait_state;
                  def_mem->mem = g_new(Word,def_mem->end-def_mem->start+1);
                  
                  list2 = g_list_prepend(list, def_mem);
                  if ( end <= def_mem2->end )
                    {
                      return list2;
                    }
                  else
                    {
                      start = def_mem2->end+1;
                    }
                }
            }
        }
    }

  // if we got here, this must go at the end 

  def_mem = g_new(struct _def_mem,1);
  def_mem->start=start;
  def_mem->end=end;
  def_mem->type=type;
  def_mem->wait_state=wait_state;
  def_mem->mem = g_new(Word,def_mem->end-def_mem->start+1);
  
  // add to end of list
  list = g_list_last(memlist);
  list2 = g_list_append(list, def_mem);

  return memlist;
}

// assume's room already allocated and copy the given data into the memlist
static void fill_mem(GList *memlist, Word *data, WordP start, WordP end)
{
  struct _def_mem *def_mem;
  GList *list;
  Word *mem;
  int k;

  for (list=memlist;list;list=list->next)
    {
      def_mem = (struct _def_mem *)list->data;
   
      if ( (def_mem->start <= start) && (start <= def_mem->end) )
        {
          // found a spot
          if ( end > def_mem->end )
            {
              // span's more than 1 link
              mem = def_mem->mem + start - def_mem->start;
              for (k=0;k<(def_mem->end-start+1);k++)
                *mem++ = *data++;
              start=def_mem->end+1;
            }
          else
            {
              mem = def_mem->mem + start - def_mem->start;
              for (k=0;k<(end-start+1);k++)
                *mem++ = *data++;
              return;
            }
        }
    }
  return;
}
      
/* Creates a chunk of DSP memory and fills it, if data is not NULL.
 * Normally this called by the function the reads in and processes the
 * COFF file being analyzed. It is also called to generate memory-mapped
 * registers and if any memory creation is specified if DSP memory is
 * accessed that doesn't exist. */
void create_mem(Word *data, WordP start, long int size, MemType type)
{
  int wait_state=0;

  // Setup defined memory list
  if ( type == ( PROGRAM_MEM_TYPE | DATA_MEM_TYPE ) )
    {
      DataProgMemList = insert_mem_list(DataProgMemList,start,start+size-1,type,
                                        wait_state);
      if ( data )
        fill_mem(DataProgMemList,data,start,start+size-1);
    }
  if ( type == PROGRAM_MEM_TYPE )
    {
      ProgMemList = insert_mem_list(ProgMemList,start,start+size-1,type,
                                        wait_state);
      
      if ( data )
        fill_mem(ProgMemList,data,start,start+size-1);
    }
  if ( type == DATA_MEM_TYPE )
    {
      DataMemList = insert_mem_list(DataMemList,start,start+size-1,type,
                                        wait_state);
      if ( data )
        fill_mem(DataMemList,data,start,start+size-1);
    }
  return;
}

struct _MMR *MMR;

void set_MMR_ptr()
{
  struct _def_mem *def_mem;
  Word *mem;

  g_return_if_fail(DataProgMemList);

  def_mem = (struct _def_mem *)(DataProgMemList->data);

  g_return_if_fail(def_mem->start==0x0);

  mem = def_mem->mem;

  MMR = (struct _MMR *)mem;
}

Word read_port_mem(WordP offset, int *wait_state)
{
  return read_program_mem(offset,wait_state);
}

int write_port_mem(WordP offset, Word value)
{
  return write_program_mem(offset,value);
}

void set_fileIO_break_on_memory(struct _fileIO *io)
{
  struct _fileIO_break *fIObrk;
  GList **listS;

  fIObrk = g_new(struct _fileIO_break,1);

  fIObrk->address = io->address_reached;
  fIObrk->type = io->mem_type_reached;
  fIObrk->io = io;

  io->updateF = update_fileIO_break_on_memory;
  io->removeF = remove_fileIO_break_on_memory;

  if ( io->reached_how == MEMORY_WRITE )
    {
      listS = &FileWriteBreak;
    }
  else if ( io->reached_how == MEMORY_READ )
    {
      listS = &FileReadBreak;
    }
  else
    {
      printf("Programming Error! Bad Call. %s:%d\n",__FILE__,__LINE__);
    }
   
  *listS = g_list_append(*listS,fIObrk);
}

void update_fileIO_break_on_memory(struct _fileIO *io)
{
  GList *list,**listS;
  struct _fileIO_break *fIObrk;

  if ( io->reached_how == MEMORY_WRITE )
    {
      listS = &FileWriteBreak;
    }
  else if ( io->reached_how == MEMORY_READ )
    {
      listS = &FileReadBreak;
    }
  else
    {
      printf("Programming Error! Bad Call. %s:%d\n",__FILE__,__LINE__);
    }
   
  while (list)
    {
      fIObrk = list->data;
      if ( io == fIObrk->io )
	{
	  fIObrk->address = io->address_reached;
	  fIObrk->type = io->mem_type_reached;
	  fIObrk->io = io;
	}
      list=list->next;
    }
  printf("Programming Error! Bad Call. %s:%d\n",__FILE__,__LINE__);
}
    
void remove_fileIO_break_on_memory(struct _fileIO *io)
{
  GList *list,**listS;
  struct _fileIO_break *fIObrk;

  if ( io->reached_how == MEMORY_WRITE )
    {
      listS = &FileWriteBreak;
    }
  else if ( io->reached_how == MEMORY_READ )
    {
      listS = &FileReadBreak;
    }
  else
    {
      printf("Programming Error! Bad Call. %s:%d\n",__FILE__,__LINE__);
    }
   
  while (list)
    {
      fIObrk = list->data;
      if ( io == fIObrk->io )
	{
	  *listS = g_list_remove_link(*listS,list);
	  g_free(fIObrk);
	  return;
	}
      list=list->next;
    }
  printf("Programming Error! Bad Call. %s:%d\n",__FILE__,__LINE__);
}
