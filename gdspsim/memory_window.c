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

#include <stdio.h>
#include "decode_window.h"
#include "c54_core.h"
#include "process_coff.h"
#include "find_opcode.h"
#include "memory.h"
#include "entryCB.h"
#include "gtkbox_add.h"
#include <string.h>

extern GdkFont *gdsp_Decode_Font;
GList *all_mem_win_list=NULL;

struct _mem_window_nfo
{
  GtkWidget *memoryW;
  int memory_type;
  GtkWidget *mem_box;  // The vbox that memory display is packed into
  GtkWidget *label_data;
  GtkWidget *label_prog;
  WordA start;
  WordA end;
  struct _entryCB_nfo *start_nfo;
  struct _entryCB_nfo *end_nfo;
};
static void destroy_window_CB( GtkWidget *W, gpointer data )
{
  struct _mem_window_nfo *nfo;

  nfo=data;

  all_mem_win_list=g_list_remove(all_mem_win_list,nfo);

  g_free(nfo->end_nfo);
  g_free(nfo->start_nfo);
  g_free(nfo);
}

static void data_mem_changeCB( GtkWidget *W, int address )
{
  g_return_if_fail(W);

  return;
}

static void prog_mem_changeCB( GtkWidget *W, int address )
{
  g_return_if_fail(W);

  return;
}


static void mem_type_changeCB( GtkWidget *W, struct _mem_window_nfo *mwn )
{
  g_return_if_fail(mwn);
  g_return_if_fail(W);

  if ( mwn->memory_type & DATA_MEM_TYPE )
    {
      gtk_container_remove(GTK_CONTAINER(W),mwn->label_data);
      mwn->memory_type = PROGRAM_MEM_TYPE;
      gtk_container_add(GTK_CONTAINER(W),mwn->label_prog);
      gtk_widget_show(mwn->label_prog);
      gtk_widget_show(W);
    }
  else
    {
      gtk_container_remove(GTK_CONTAINER(W),mwn->label_prog);
      mwn->memory_type = DATA_MEM_TYPE;
      gtk_container_add(GTK_CONTAINER(W),mwn->label_data);
      gtk_widget_show(mwn->label_data);
    }
}
static GtkWidget *mem_view_new(WordA mem, MemType memtype)
{
  GtkWidget *hbox;
  GtkWidget *entry,*label;
  gchar text[7];
  int mem_int,wait;
  
  g_snprintf(text,7,"0x%x",mem);

  label = gtk_label_new(text);

  entry = gtk_entry_new();
#ifdef GTK2
  gtk_entry_set_width_chars(GTK_ENTRY(entry),13);
#endif
  gtk_entry_set_max_length(GTK_ENTRY(entry),13);
  
  g_snprintf(text,7,"0x%x",read_mem(mem,&wait,memtype));
  
  if ( memtype == DATA_MEM_TYPE )
    g_snprintf(text,7,"0x%x",read_data_mem(mem,&wait));
  else
    g_snprintf(text,7,"0x%x",read_program_mem(mem,&wait));
  
  gtk_entry_set_text(GTK_ENTRY(entry),text);
  mem_int=mem;

  if ( memtype & DATA_MEM_TYPE )
    gtk_signal_connect(GTK_OBJECT(entry), "activate",
		       GTK_SIGNAL_FUNC(data_mem_changeCB),
		       (gpointer)mem_int);
  else
    gtk_signal_connect(GTK_OBJECT(entry), "activate",
		       GTK_SIGNAL_FUNC(prog_mem_changeCB),
		       (gpointer)mem_int);

  gtk_widget_show (entry);  
  gtk_widget_show (label);  

  hbox = gtk_hbox_new (FALSE, 0);  
  gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX (hbox), entry, FALSE, FALSE, 0);
  gtk_widget_show (hbox);
      

  return hbox;
}

static void update_memory_view(struct _mem_window_nfo *mwn, WordA start_mem, WordA end_mem)
{
  WordA mem;
  GtkWidget *hbox;
  GList *list,*list2;

  if ( start_mem > end_mem )
    return;

  if ( start_mem < mwn->start )
    {
      printf("start_mem=%d mwn->start=%d\n",start_mem,mwn->start);
      // add new memory location at the beginning
      for (mem=mwn->start-1;mem>=start_mem && mem<mwn->start;mem--)
	{
	  printf("mem=0x%x\n",mem);

	  hbox = mem_view_new(mem,mwn->memory_type);
	  gtk_box_pack_start_prepend (GTK_BOX (mwn->mem_box), hbox, FALSE, TRUE, 0);
	  //gtk_widget_realize(hbox);
	}
    }
  else if ( start_mem > mwn->start )
    {
      // remove some memory locations at the beginning
      list = gtk_container_children(GTK_CONTAINER(mwn->mem_box));
      mem = mwn->start;
      while ( list && mem<start_mem )
	{
	  list2 = list->next;
	  gtk_container_remove(GTK_CONTAINER(mwn->mem_box),GTK_WIDGET(list->data));
	  list = list2;
	  mem++;
	}
    }
  mwn->start = start_mem;
  if ( end_mem > mwn->end )
    {
      // add new memory locations at the end
      if ( start_mem <= mwn->end )
	start_mem = mwn->end + 1;
      for (mem=start_mem;mem<=end_mem;mem++)
	{
	  hbox = mem_view_new(mem,mwn->memory_type);
	  printf("Packing mem 0x%x\n",mem);
	  gtk_box_pack_start (GTK_BOX (mwn->mem_box), hbox, FALSE, TRUE, 0);
	  //gtk_widget_realize(hbox);
	}
    }
  else if ( end_mem < mwn->end )
    {
      // remove some memory locations at the beginning
      list = gtk_container_children(GTK_CONTAINER(mwn->mem_box));
      list = g_list_last(list);
      mem = mwn->end;
      while ( list && mem>end_mem )
	{
	  list2 = list->prev;
	  gtk_container_remove(GTK_CONTAINER(mwn->mem_box),GTK_WIDGET(list->data));
	  list = list2;
	  mem--;
	}
    }
  mwn->end = end_mem;
}

static void change_start_address(GtkWidget *entry, guint64 address, 
				 gpointer data)
{
  struct _mem_window_nfo *mwn;
  WordA mem;

  mwn = data;
  mem = address;
  update_memory_view(mwn,mem,mwn->end);
}

static void change_end_address(GtkWidget *entry, guint64 address, 
			       gpointer data)
{
  WordA mem;
  struct _mem_window_nfo *mwn;

  mwn = data;
  mem = address;
  update_memory_view(mwn,mwn->start,mem);
}

/*
 * This will be called by the main routine to create a popup window.
 * At the top there should be 2 spin buttons that controls the range
 * of memory that the scroll bars can access (and labeled as such). One for
 * the top index and one for the bottom index. For 
 * memory efficieny reasons, the user may only want part of the memory visiable.
 * There should also be a step button at the top that will give a call to 
 * decode_step when pressed.
 */

void create_memory_window()
{
  GtkWidget *vbox2,*hbox2,*vbox,*hbox,*scrolledW;
  GtkWidget *entryTop,*entryBottom,*memtype_button;
  WordA mem;
  gchar temp_str[7];
  struct _mem_window_nfo *mem_window;
  struct _entryCB_nfo *entry_start_nfo,*entry_end_nfo;

  mem_window = g_new(struct _mem_window_nfo,1);
  all_mem_win_list = g_list_prepend(all_mem_win_list,mem_window);

  mem_window->memoryW = gtk_window_new (GTK_WINDOW_TOPLEVEL);

  gtk_signal_connect(GTK_OBJECT(mem_window->memoryW),"destroy",
		     (GtkSignalFunc)destroy_window_CB,mem_window);

  gtk_widget_set_name (mem_window->memoryW, "Disassembly");
  // gtk_widget_set_usize (GTK_WIDGET(memoryW), 300, 200);
  gtk_window_set_title (GTK_WINDOW (mem_window->memoryW), "Disassebly");
  gtk_container_set_border_width (GTK_CONTAINER (mem_window->memoryW), 0);

  vbox = gtk_vbox_new (FALSE, 0);
  gtk_container_add (GTK_CONTAINER (mem_window->memoryW), vbox);
  gtk_widget_show (vbox);

  memtype_button = gtk_button_new();
  mem_window->label_data = gtk_label_new("Data");
#ifdef GTK2
  g_object_ref(mem_window->label_data);
#endif
  mem_window->label_prog = gtk_label_new("Prog");
#ifdef GTK2
  g_object_ref(mem_window->label_prog);
#endif
  gtk_container_add (GTK_CONTAINER (memtype_button), mem_window->label_data);
  gtk_widget_show(mem_window->label_data);
  gtk_box_pack_start (GTK_BOX (vbox), memtype_button, FALSE, FALSE, 0);
  gtk_widget_show(memtype_button);
  gtk_signal_connect(GTK_OBJECT(memtype_button), "clicked",
		     GTK_SIGNAL_FUNC(mem_type_changeCB),
		     mem_window);

  hbox = gtk_hbox_new (FALSE, 0);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);
  gtk_widget_show (hbox);

  // start memory
  get_prog_mem_start_end(&mem_window->start,&mem_window->end);

  entry_start_nfo = g_new(struct _entryCB_nfo,1);
  mem_window->start_nfo = entry_start_nfo; // Saving to prevent mem leak

  entryTop = gtk_entry_new();
  entry_start_nfo->entry = entryTop;
  entry_start_nfo->bits = BITS_PER_ADDRESS;
  entry_start_nfo->CB_func = change_start_address;
  entry_start_nfo->data = mem_window;
  gtk_box_pack_start (GTK_BOX (hbox), entryTop, FALSE, FALSE, 0);
  gtk_entry_set_max_length(GTK_ENTRY(entryTop),7);
#ifdef GTK2
  gtk_entry_set_width_chars(GTK_ENTRY(entryTop),7);
#endif
  g_snprintf(temp_str,7,"0x%x",mem_window->start);
  gtk_entry_set_text (GTK_ENTRY(entryTop),temp_str);
  gtk_signal_connect(GTK_OBJECT(entryTop), "activate",
		     GTK_SIGNAL_FUNC(entry_hexCB),
		     entry_start_nfo);
  gtk_widget_show (entryTop);

  entry_end_nfo = g_new(struct _entryCB_nfo,1);
  mem_window->end_nfo = entry_end_nfo; // Saving to prevent mem leak

  entryBottom = gtk_entry_new();
  entry_end_nfo->entry = entryBottom;
  entry_end_nfo->bits = BITS_PER_ADDRESS;
  entry_end_nfo->CB_func = change_end_address;
  entry_end_nfo->data = mem_window;
  gtk_box_pack_start (GTK_BOX (hbox), entryBottom, FALSE, FALSE, 0);
  gtk_entry_set_max_length(GTK_ENTRY(entryBottom),7);
#ifdef GTK2
  gtk_entry_set_width_chars(GTK_ENTRY(entryBottom),7);
#endif
  g_snprintf(temp_str,7,"0x%x",mem_window->end);
  gtk_entry_set_text (GTK_ENTRY(entryBottom),temp_str);
  gtk_signal_connect(GTK_OBJECT(entryBottom), "activate",
		     GTK_SIGNAL_FUNC(entry_hexCB),
		     entry_end_nfo);
  gtk_widget_show (entryBottom);


  scrolledW = gtk_scrolled_window_new(NULL, NULL);
  gtk_box_pack_start (GTK_BOX (vbox), scrolledW, TRUE, TRUE, 0);
  gtk_widget_show(scrolledW);

  vbox2 = gtk_vbox_new (FALSE, 0);
  // gtk_box_pack_start (GTK_BOX (scrolledW), vbox2, FALSE, FALSE, 0);
  // gtk_container_add(GTK_(scrolledW),vbox2);
  gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(scrolledW),vbox2);
  gtk_widget_show (vbox2);
 
  // Default to data memory
  mem_window->memory_type=DATA_MEM_TYPE;
  mem_window->mem_box = vbox2; // the box that all memory view go in to

  for (mem=mem_window->start;mem<=mem_window->end;mem++)
    {
      hbox2 = mem_view_new(mem,mem_window->memory_type);

      gtk_box_pack_start (GTK_BOX (vbox2), hbox2, FALSE, TRUE, 0);
    }


  gtk_widget_show(mem_window->memoryW);

}
#if 0
void update_mem_cell(GtkWidget *hbox, int mem_type)
{
  GList *list;
  GtkEntry *entry;
  gchar *text_now;
  const gchar *text_before;
  int wait;

  // There's a label then and entry packed into hbox.
  // need to update the entry text and change color if it's different.
  
  list = gtk_container_children(hbox);
  entry = list->next->data;
  g_snprintf(text_now,7,"0x%x",read_mem(mem,&wait,mem_type));

  text_before = gtk_entry_get_text(GTK_ENTRY(entry));
  if ( g_strcmp(text_before,text_new) == 0 )
    {
      printf("Mem location 0x%x changed\n",mem);
      
      gtk_entry_set_text(GTK_ENTRY(entry),text);
    }

}
#endif

void update_all_memory_windows()
{
  GList *list,*list_vbox,*list_hbox;
  struct _mem_window_nfo *mem_window;
  GtkEntry *entry;
  GtkBox *vbox,*hbox;
  gchar text_now[7];
  const gchar *text_before;
  WordA mem;
  int wait;

  for (list=all_mem_win_list;list;list=list->next)
    {
      mem_window = list->data;

      //printf("about to extract vbox\n");
      vbox = GTK_BOX(mem_window->mem_box);
      mem = mem_window->start;
      //printf("starting loop\n");
      for ( list_vbox = gtk_container_children(GTK_CONTAINER(vbox));
	    list_vbox; list_vbox = list_vbox->next )
	{
	  //printf("going through hbox\n");
	  hbox = GTK_BOX(list_vbox->data);
	  //printf("extracted hbox\n");
	  list_hbox = gtk_container_children(GTK_CONTAINER(hbox));
	  //printf("got list of hbox\n");
	  entry = GTK_ENTRY(list_hbox->next->data);
	  //printf("cast to entry\n");
	  g_snprintf(text_now,7,"0x%x",read_mem(mem,&wait,mem_window->memory_type));
	  //printf("read memory\n");
	  text_before = gtk_entry_get_text(GTK_ENTRY(entry));
	  if ( strcmp(text_before,text_now) != 0 )
	    {
	      printf("Mem location 0x%x changed from %s to %s\n",mem,text_before,text_now);
      
	      gtk_entry_set_text(GTK_ENTRY(entry),text_now);
	    }
	  mem++;
	  
	}
    }
}
