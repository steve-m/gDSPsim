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
	  cp_to_mem(0,offset,1,type);
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
      cp_to_mem(&value,offset,1,type);
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
	  printf("MemUsed = 0x%x to 0x%x\n",def_mem->start,def_mem->end);
	}
    }
  if ( DataMemList )
    {
      for (list=DataMemList;list;list=list->next)
	{
	  def_mem = (struct _def_mem *)list->data;
	  printf("MemUsed = 0x%x to 0x%x\n",def_mem->start,def_mem->end);
	}
    }
  if ( ProgMemList )
    {
      for (list=ProgMemList;list;list=list->next)
	{
	  def_mem = (struct _def_mem *)list->data;
	  printf("MemUsed = 0x%x to 0x%x\n",def_mem->start,def_mem->end);
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

static GList *insert_mem_list( GList *memlist, WordP start, WordP end, MemType type, int wait_state)
{
  struct _def_mem *def_mem,*def_mem2;
  GList *list,*list2;

  // Create a new memory section
  def_mem = g_new(struct _def_mem,1);
  def_mem->start=start;
  def_mem->end=end;
  def_mem->type=type;
  def_mem->wait_state=wait_state;
  def_mem->mem = g_new(Word,end-start+1);
  
  if ( memlist == NULL )
    {
      list2 = g_list_prepend(NULL, def_mem);
      return list2;
    }

  // Insert the new section by sorting on start
  for (list=memlist;list;list=list->next)
    {
      def_mem2 = (struct _def_mem *)list->data;

      if ( start <= def_mem2->start )
	{
	  list2 = g_list_prepend(list,def_mem);
	  return list2;
	}
      list2=list;
    }
  list2 = g_list_append(list2,def_mem);
  return g_list_last(list2);
}

// Assume's keep->mem has been allocated but doesn't contain valid data.
// Re-allocates keep->mem and moves remove->mem into it.
static void combine_mem_blocks(GList *keepL, GList *removeL)
{
  // Copy data from old section to new section
  Word *mem,*mem2;
  struct _def_mem *keep, *remove;
  int k;

  keep = (struct _def_mem *)keepL->data;
  remove = (struct _def_mem *)removeL->data;

  keep->mem = g_new(Word,keep->end-keep->start+1);
  mem = keep->mem;
  mem = mem + (remove->start - keep->start);
  mem2 = remove->mem;
  for (k=0;k<( remove->end - remove->start + 1);k++)
    {
      *mem++=*mem2++;
    }
  g_free(remove->mem);
  g_free(remove);

  removeL=g_list_remove_link(removeL,removeL);
}

static GList *adjust_mem_list( GList *memlist, WordP start, WordP end, MemType type, int wait_state)
{
  struct _def_mem *def_mem,*def_mem2;
  GList *list,*list2;

  // Just create a new section and insert it in the correct spot.
  list = insert_mem_list( memlist, start, end, type, wait_state);
  def_mem = (struct _def_mem *)list->data;

  // Now check to see if the adjacent entries can be combined.
  // and check for errors in overlapping different type sections.

  // Check the previous section
  if ( list->prev )
    {
      list2 = list->prev;
      def_mem2 = (struct _def_mem *)list2->data;

      // Don't move 0x0 to 0x80 section because that's Memory Mapped
      
      if ( def_mem2->start >= 0x80 )
	{

	  if ( start <= def_mem2->end )
	    {
	      // These sections intersect
	      if ( (type != def_mem2->type ) || 
	       ( wait_state != def_mem2->wait_state ) )
		{
		  // Error, different type
		  printf("Error! mixing different memory section types (start=0x%x)\n",start);
		}
	      // Copy mem from old section to new section
	      def_mem->start = def_mem2->start;
	      combine_mem_blocks(list,list2);
	      
	    }
	  else if ( ( (start-1) == def_mem2->end ) &&
		    (type == def_mem2->type ) &&
		    ( wait_state == def_mem2->wait_state ) )
	    {
	      // These sections are adjacent and same type.
	      def_mem->start = def_mem2->start;
	      combine_mem_blocks(list,list2);
	    }
	}
    }
  // Check the next section
  if ( list->next )
    {
      list2 = list->next;
      def_mem2 = (struct _def_mem *)list2->data;

      if ( end >= def_mem2->start )
	{
	  // These sections intersect
	  if ( (type != def_mem2->type ) || 
	       ( wait_state != def_mem2->wait_state ) )
	    {
	      // Error, different type
	      printf("Error! mixing different memory section types (start=0x%x)\n",start);
	    }
	  // Copy mem from old section to new section
	  def_mem->end = def_mem2->end;
	  combine_mem_blocks(list,list2);

	}
      else if ( ( (end+1) == def_mem2->start ) &&
		(type == def_mem2->type ) &&
		( wait_state == def_mem2->wait_state ) )
	{
	  // These sections are adjacent and same type.
	  def_mem->end = def_mem2->end;
	  combine_mem_blocks(list,list2);
	}
    }

  return list;
}
  
static void fill_mem_with_data(Word *data, WordP start, int size, struct _def_mem *def_mem)
{
  int k;
  Word *mem;

  // Copy new data
  mem = def_mem->mem + start - def_mem->start;
  for (k=0;k<size;k++)
    {
      *mem++=*data++;
    }
}
 
static void fill_mem_with_const(Word fill, WordP start, int size, struct _def_mem *def_mem)
{
  int k;
  Word *mem;

  // Copy new data
  mem = def_mem->mem + start - def_mem->start;
  for (k=0;k<size;k++)
    {
      *mem++=fill;
    }
}
 
void cp_to_mem(Word *data, WordP start, long int size, MemType type)
{
  int wait_state=0;
  GList *list;

  // Setup defined memory list
  if ( type == ( PROGRAM_MEM_TYPE | DATA_MEM_TYPE ) )
    {
      list = adjust_mem_list(DataProgMemList,start,start+size-1,type,
			     wait_state);
      fill_mem_with_data(data,start,size,list->data);

      DataProgMemList = g_list_first(list);
    }
  if ( type == PROGRAM_MEM_TYPE )
    {
      list = adjust_mem_list(ProgMemList,start,start+size-1,type,
			     wait_state);
      fill_mem_with_data(data,start,size,list->data);

      ProgMemList = g_list_first(list);
    }
  if ( type == DATA_MEM_TYPE )
    {
      list = adjust_mem_list(DataMemList,start,start+size-1,type,
			     wait_state);
      fill_mem_with_data(data,start,size,list->data);

      DataMemList = g_list_first(list);
    }
	   
  return;
}

void fill_to_mem(Word fill,  WordP start, long int size, MemType type)
{
  int wait_state=0;
  GList *list;

  // Setup defined memory list
  if ( type == ( PROGRAM_MEM_TYPE | DATA_MEM_TYPE ) )
    {
      list = adjust_mem_list(DataProgMemList,start,start+size-1,type,
			     wait_state);
      fill_mem_with_const(fill,start,size,list->data);

      DataProgMemList = g_list_first(list);
    }
  if ( type == PROGRAM_MEM_TYPE )
    {
      list = adjust_mem_list(ProgMemList,start,start+size-1,type,
			     wait_state);
      fill_mem_with_const(fill,start,size,list->data);

      ProgMemList = g_list_first(list);
    }
  if ( type == DATA_MEM_TYPE )
    {
      list = adjust_mem_list(DataMemList,start,start+size-1,type,
			     wait_state);
      fill_mem_with_const(fill,start,size,list->data);

      DataMemList = g_list_first(list);
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
