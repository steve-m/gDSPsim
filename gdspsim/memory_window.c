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

#include <stdio.h>
#include <decode_window.h>
#include <chip_core.h>
#include <find_opcode.h>
#include <memory.h>
#include <entryCB.h>
#include <gtkbox_add.h>
#include <string.h>
#include <memory_window.h>

extern GdkFont *gdsp_Decode_Font;
GList *all_mem_win_list=NULL;

struct _mem_window_nfo
{
  GtkWidget *memoryW;
  int memory_type;
  GtkCList *clist;  // The vbox that memory display is packed into
  GtkWidget *label_data;
  GtkWidget *label_prog;
  WordP start;
  WordP end;
  struct _entryCB_nfo *start_nfo;
  struct _entryCB_nfo *end_nfo;
};

static GdkColor gdsp_color[4]=
{
  {0,0x8000,0x0,0x0},         // forground color for written
  {0,148*256,0,211*256},      // forground color for not allocated
  {0,0x0,0x8000,0x8000},      // forground color for read
  {0,238*256,232*256,170*256} // background color for SP
};

static GdkColormap *gdsp_colormap=NULL;

static Word SP_last;

#define MEM_CHANGED_BLK 5
struct _mem_changed
{
  int valid;
  WordP changed[MEM_CHANGED_BLK];
  unsigned int accessed[MEM_CHANGED_BLK][2]; // written, read
};

GList *head_mem_changed=NULL;

void entry_data_memCB( GtkWidget *widget, WordP offset )
{
  gchar *entry_text,*textP;
  int value;

  entry_text = gtk_editable_get_chars(GTK_EDITABLE(widget),0,-1);
  // entry_text = gtk_entry_get_text(GTK_ENTRY(widget));
  textP = entry_text;
  if ( *textP++ == '0' )
    {
      if ( *textP == 'X' || *textP == 'x' )
	{
	  //	  // now remove all non hex numbers
	  //	  textP++;
	  //	  while ( *textP )
	    {
	      sscanf(entry_text,"0x%x",&value);
	      write_data_mem(offset,value);
	    }
	}
  
    }
  printf("Entry contents: %s %d\n", entry_text,value);
  g_free(entry_text);

}

void entry_prog_memCB( GtkWidget *widget, WordP offset )
{
  gchar *entry_text,*textP;
  int value;

  entry_text = gtk_editable_get_chars(GTK_EDITABLE(widget),0,-1);
  // entry_text = gtk_entry_get_text(GTK_ENTRY(widget));
  textP = entry_text;
  if ( *textP++ == '0' )
    {
      if ( *textP == 'X' || *textP == 'x' )
	{
	  //	  // now remove all non hex numbers
	  //	  textP++;
	  //	  while ( *textP )
	    {
	      sscanf(entry_text,"0x%x",&value);
	      write_program_mem(offset,value);
	    }
	}
  
    }
  printf("Entry contents: %s %d\n", entry_text,value);
  g_free(entry_text);

}

static void click_CB( GtkWidget *W, gint row, gint column, GdkEventButton *event, struct _mem_window_nfo *mwn )
{
  // Create popup window, with entry widget to change value
  GtkWidget *popupW,*tableW,*label,*entry;
  GtkTable *table;
  gchar temp_str[7];
  WordP address;

  address = row + mwn->start;

  popupW = gtk_window_new (GTK_WINDOW_TOPLEVEL);

  //  gtk_signal_connect(GTK_OBJECT(popupW),"destroy",
  //	     (GtkSignalFunc)destroy_window_CB,mem_window);

  tableW = gtk_table_new(2,2,FALSE);
  gtk_widget_show(tableW);
  table = GTK_TABLE(tableW);
  
  gtk_container_add (GTK_CONTAINER (popupW), tableW);

  label = gtk_label_new("address");
  gtk_table_attach(table, label, 0,1, 0,1, 0,0,0,0);
  gtk_widget_show(label);

  label = gtk_label_new("value");
  gtk_table_attach(table, label, 1,2, 0,1, 0,0,0,0);
  gtk_widget_show(label);

  g_snprintf(temp_str,7,"0x%x",address);
  label = gtk_label_new(temp_str);
  gtk_table_attach(table, label, 0,1, 1,2, 0,0,0,0);
  gtk_widget_show(label);

  entry = gtk_entry_new();
#ifdef GTK2
  gtk_entry_set_width_chars(GTK_ENTRY(entry),13);
#endif
  gtk_entry_set_max_length(GTK_ENTRY(entry),13);
  gtk_table_attach(table, entry, 1,2, 1,2, 0,0,0,0);
  gtk_widget_show (entry);  
  if ( mwn->memory_type == DATA_MEM_TYPE )
    gtk_signal_connect(GTK_OBJECT(entry), "activate",
		       GTK_SIGNAL_FUNC(entry_data_memCB),
		       (gpointer)address);
  else
    gtk_signal_connect(GTK_OBJECT(entry), "activate",
		       GTK_SIGNAL_FUNC(entry_prog_memCB),
		       (gpointer)address);

  gtk_widget_show(popupW);

}

static void destroy_window_CB( GtkWidget *W, gpointer data )
{
  struct _mem_window_nfo *nfo;

  nfo=data;

  all_mem_win_list=g_list_remove(all_mem_win_list,nfo);

  g_free(nfo->end_nfo);
  g_free(nfo->start_nfo);
  g_free(nfo);
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

static void mem_view_append(WordP mem, MemType memtype, GtkCList *clist)
{
  int wait;
  gchar *data[2];
  gchar address[7],value_str[7];
  int row,available;

  g_snprintf(address,7,"0x%x",mem);

  g_snprintf(value_str,7,"0x%x",read_mem(mem,&wait,memtype,&available));
  
  data[0] = address;
  data[1] = value_str;

  row = gtk_clist_append(clist,data);
  if ( !available )
    {
      gtk_clist_set_foreground(clist,row,&gdsp_color[1]);
    }

}

static void mem_view_prepend(WordP mem, MemType memtype, GtkCList *clist)
{
  int wait;
  gchar *data[2];
  gchar address[7],value_str[7];
  int row,available;

  g_snprintf(address,7,"0x%x",mem);

  g_snprintf(value_str,7,"0x%x",read_mem(mem,&wait,memtype,&available));
  
  data[0] = address;
  data[1] = value_str;

  row = gtk_clist_prepend(clist,data);
  if ( !available )
    {
      gtk_clist_set_foreground(clist,row,&gdsp_color[1]);
    }

}

static void update_memory_view(struct _mem_window_nfo *mwn, WordP start_mem, WordP end_mem)
{
  WordP mem;
  gchar temp_str[7];

  gtk_clist_freeze(mwn->clist);

  if ( start_mem > end_mem )
    {
      if ( start_mem == mwn->start )
	{
	  start_mem = end_mem;
	  g_snprintf(temp_str,7,"0x%x",start_mem);
	  gtk_entry_set_text (GTK_ENTRY(mwn->start_nfo->entry),temp_str);
	}
      else
	{
	  end_mem = start_mem;
	  g_snprintf(temp_str,7,"0x%x",end_mem);
	  gtk_entry_set_text (GTK_ENTRY(mwn->end_nfo->entry),temp_str);
	}
    }
  else if ( (end_mem - start_mem) > 0x200 )
    {
      if ( start_mem == mwn->start )
	{
	  start_mem = end_mem - 0x200;
	  g_snprintf(temp_str,7,"0x%x",start_mem);
	  gtk_entry_set_text (GTK_ENTRY(mwn->start_nfo->entry),temp_str);
	}
      else
	{
	  end_mem = start_mem + 0x200;
	  g_snprintf(temp_str,7,"0x%x",end_mem);
	  gtk_entry_set_text (GTK_ENTRY(mwn->end_nfo->entry),temp_str);
 	}
    }

  if ( start_mem < mwn->start )
    {
      if ( end_mem < mwn->start )
	{
	  gtk_clist_clear(mwn->clist);
	  mwn->start = start_mem;
	  mwn->end = end_mem;
	  for (mem=mwn->start;mem<=mwn->end;mem++)
	    {
	      mem_view_append(mem,mwn->memory_type,mwn->clist);
	    }
	  update_all_memory_windows(1);
	  gtk_clist_thaw(mwn->clist);
	  return;
	}

      // add new memory location at the beginning
      for (mem=mwn->start-1;mem>=start_mem && mem<mwn->start;mem--)
	{
	  mem_view_prepend(mem,mwn->memory_type,mwn->clist);
	}
    }
  else if ( start_mem > mwn->end )
    {
      // Clear out everyting
      gtk_clist_clear(mwn->clist);
      mem_view_prepend(start_mem,mwn->memory_type,mwn->clist);
    }
  else if ( start_mem > mwn->start )
    {
      // remove some memory locations at the beginning
      for (mem=mwn->start; mem<start_mem;mem++)
	{
	  gtk_clist_remove(mwn->clist,0);
	}
    }

  mwn->start = start_mem;

  if ( end_mem > mwn->end )
    {
      // add new memory locations at the end
      if ( start_mem <= mwn->end )
	start_mem = mwn->end;
      for (mem=start_mem+1;mem<=end_mem;mem++)
	{
	  mem_view_append(mem,mwn->memory_type,mwn->clist);
	}
    }
  else if ( end_mem < mwn->end )
    {
      int row;
      row = end_mem+1-start_mem;
      for (mem=end_mem; mem<mwn->end;mem++)
	{
	  gtk_clist_remove(mwn->clist,row);
	}
    }
  mwn->end = end_mem;
  gtk_clist_thaw(mwn->clist);

}

static void change_start_address(GtkWidget *entry, guint64 address, 
				 gpointer data)
{
  struct _mem_window_nfo *mwn;
  WordP mem;

  mwn = data;
  mem = address;
  update_memory_view(mwn,mem,mwn->end);
}

static void change_end_address(GtkWidget *entry, guint64 address, 
			       gpointer data)
{
  WordP mem;
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
  GtkWidget *vbox,*hbox,*scrolledW,*clist;
  GtkWidget *entryTop,*entryBottom,*memtype_button;
  WordP mem;
  gchar temp_str[7];
  gchar *titles[]={"address","value"};
  struct _mem_window_nfo *mem_window;
  struct _entryCB_nfo *entry_start_nfo,*entry_end_nfo;
  extern GtkAccelGroup *gDSP_keyboard_accel;

  // Allocate colors
  if ( gdsp_colormap == NULL )
    {
      gdsp_colormap = gdk_colormap_get_system ();
      gdk_color_alloc(gdsp_colormap,&gdsp_color[0]);
      gdk_color_alloc(gdsp_colormap,&gdsp_color[1]);
    }

  mem_window = g_new(struct _mem_window_nfo,1);
  all_mem_win_list = g_list_prepend(all_mem_win_list,mem_window);

  mem_window->memoryW = gtk_window_new (GTK_WINDOW_TOPLEVEL);

  gtk_signal_connect(GTK_OBJECT(mem_window->memoryW),"destroy",
		     (GtkSignalFunc)destroy_window_CB,mem_window);
  // Attach keyboard accelerations from main window
  gtk_window_add_accel_group (GTK_WINDOW (mem_window->memoryW), gDSP_keyboard_accel);

  gtk_widget_set_name (mem_window->memoryW, "Memory Window");
  gtk_widget_set_usize (GTK_WIDGET(mem_window->memoryW), 150, 350);
  gtk_window_set_title (GTK_WINDOW (mem_window->memoryW), "Memory Window");
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
  gtk_box_pack_start (GTK_BOX (hbox), entryTop, TRUE, TRUE, 0);
  gtk_entry_set_max_length(GTK_ENTRY(entryTop),7);
#ifdef GTK2
  gtk_entry_set_width_chars(GTK_ENTRY(entryTop),7);
#else
  gtk_widget_set_usize(GTK_WIDGET(entryTop),50,24); 
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
  gtk_box_pack_start (GTK_BOX (hbox), entryBottom, TRUE, TRUE, 0);
  gtk_entry_set_max_length(GTK_ENTRY(entryBottom),7);
#ifdef GTK2
  gtk_entry_set_width_chars(GTK_ENTRY(entryBottom),7);
#else
  gtk_widget_set_usize(GTK_WIDGET(entryBottom),50,24); 
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

  clist = gtk_clist_new_with_titles(2,titles);
  gtk_clist_set_selection_mode(GTK_CLIST(clist), GTK_SELECTION_SINGLE);
  
  gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(scrolledW),clist);
 
  // Default to data memory
  mem_window->memory_type=DATA_MEM_TYPE;
  mem_window->clist = GTK_CLIST(clist); // the list that all memory view go in to
  gtk_signal_connect( GTK_OBJECT(clist),"select_row", 
		      GTK_SIGNAL_FUNC( click_CB), mem_window );
  
  for (mem=mem_window->start;mem<=mem_window->end;mem++)
    {
      mem_view_append(mem,mem_window->memory_type,mem_window->clist);
    }
  
    gtk_clist_set_column_width(mem_window->clist,0,gtk_clist_optimal_column_width(mem_window->clist,0));


  gtk_widget_show(clist);
  gtk_widget_show(mem_window->memoryW);

}

static void initialize_changed_mem(void)
{
  if (head_mem_changed==NULL)
    {
      struct _mem_changed *mc;

      mc = g_new(struct _mem_changed,1);
      head_mem_changed = g_list_prepend(head_mem_changed,mc);
      mc->valid = 0;
    }
}

// type==0 for written, type==1 for read
int check_if_changed_set(WordP address, int type)
{
  GList *list;
  struct _mem_changed *mc;
  int k;

  list=head_mem_changed;
  while (list != NULL)
    {
      mc=list->data;
      for (k=0; k<mc->valid; k++)
	{
	  if ( address == mc->changed[k] )
	    {
	      mc->accessed[k][type]++;
	      return 1;
	    }
	}
      list=list->next;
    }
  return 0;
}
     

void set_mem_changed(WordP address, int type)
{
  GList *list;
  struct _mem_changed *mc;

  if ( head_mem_changed == NULL )
    initialize_changed_mem();

  if ( check_if_changed_set(address,type) )
    return;

  // Get last in list
  list=head_mem_changed;
  while (list->next != NULL)
    {
      list=list->next;
    }

  mc=list->data;

  if ( mc->valid == MEM_CHANGED_BLK )
    {
      // add new link
      mc = g_new(struct _mem_changed,1);
      mc->valid = 0;
      head_mem_changed = g_list_append(head_mem_changed,mc);
    }

  mc->changed[mc->valid]=address;
  mc->accessed[mc->valid][type]=1;
  mc->accessed[mc->valid][type^1]=0;
    
  mc->valid++;
}

void clear_mem_changed(void)
{
  // free all links accepts first one.
  // Usually only 1 link will every be used. Keeps down on
  // allocatation and deallocation
  struct _mem_changed *mc;
  GList *list;

  if ( head_mem_changed == NULL )
    initialize_changed_mem();

  mc = head_mem_changed->data;
  mc->valid=0;

  list = head_mem_changed->next;
  while (list)
    {
      // free this link
      mc = list->data;
      g_free(mc);
      
      head_mem_changed = g_list_remove_link(head_mem_changed,list);
      g_list_free(list);

      list = head_mem_changed->next;
    }
  head_mem_changed->next=NULL;
}

  
// highlight = 1, to highlight changed memory
// highlight = 0, to unhighlight changed memory
void update_all_memory_windows(int highlight)
{
  GList *list,*list2;
  struct _mem_window_nfo *mem_window;
  gchar text_now[7];
  int wait,k,row,available;
  struct _mem_changed *mc;

  SP_last = MMR->SP;
  
  for (list=all_mem_win_list;list;list=list->next)
    {
      mem_window = list->data;
      
      if ( highlight )
	{
	  if ( (mem_window->start <= MMR->SP) &&
	       (mem_window->end   >= MMR->SP) )
	    {
	      row = MMR->SP-mem_window->start;
	      gtk_clist_set_background(mem_window->clist,row,
				       &gdsp_color[3]);
	      gtk_clist_set_foreground(mem_window->clist,row,
				       NULL);
	    }
	  SP_last = MMR->SP;
	}
      else
	{
	  if ( (mem_window->start <= SP_last) &&
	       (mem_window->end   >= SP_last) )
	    {
	      row = SP_last-mem_window->start;
	      gtk_clist_set_background(mem_window->clist,row,
				       NULL);
	      gtk_clist_set_foreground(mem_window->clist,row,
				       NULL);
	    }
	}

       
      for (list2=head_mem_changed;list2;list2=list2->next)
	{
	  mc = list2->data;
	  for ( k=0;k<mc->valid;k++)
	    {
	      if ( (mem_window->start <= mc->changed[k]) &&
		   (mem_window->end   >= mc->changed[k]) )
		{
		  // highlight this row red, it's changed
		  // and give it new value
		  row = mc->changed[k]-mem_window->start;
		  if ( highlight )
		    {
		      if ( mc->accessed[k][0] >= mc->accessed[k][1] )
			gtk_clist_set_foreground(mem_window->clist,row,
						 &gdsp_color[0]);
		      else
			gtk_clist_set_foreground(mem_window->clist,row,
						 &gdsp_color[2]);
		      g_snprintf(text_now,7,"0x%x",
				 read_mem(mc->changed[k],
					  &wait,mem_window->memory_type,
					  &available));
		      gtk_clist_set_text(mem_window->clist,row,1,text_now);
		    }
		  else
		    {
		      gtk_clist_set_foreground(mem_window->clist,row,
						       NULL);
		    }
		}
	    }
	}
    }
  if ( highlight == 0 )
    clear_mem_changed();
}
