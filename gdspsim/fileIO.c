/*
 * gDSPsim - GNU Digital Signal Processor Simulator
 *
 * Copyright (C) 2002, Kerry Keal, kerry@industrialmusic.com
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

#include "fileIO.h"
#include <stdio.h>
#include "memory.h"
#include "string.h"
#include "gtk_help.h"
#include "entryCB.h"

static GtkWidget *fileIOW=NULL;
GList *fileIOL=NULL;
GtkWidget *vbox_main;
struct _fileIO *io_new;


static GtkWidget *create_io(struct _fileIO *io);
static void fill_io(struct _fileIO *io);

static void destroy_window_CB( GtkWidget *W, gpointer data )
{
  fileIOW=NULL;
}

static void address_reachedCB( GtkWidget *W, struct _fileIO *io)
{
  io->modified |= 1;
  if ( io->modified == 0xf )
    gtk_widget_set_sensitive(io->applyB,TRUE);
}

static void amountCB( GtkWidget *W, struct _fileIO *io)
{
  io->modified |= 2;
  if ( io->modified == 0xf )
    gtk_widget_set_sensitive(io->applyB,TRUE);
}

static void filenameCB( GtkWidget *W, struct _fileIO *io)
{
  io->modified |= 4;
  if ( io->modified == 0xf )
    gtk_widget_set_sensitive(io->applyB,TRUE);
}

static void address_accessCB( GtkWidget *W, struct _fileIO *io)
{
  io->modified |= 8;
  if ( io->modified == 0xf )
    gtk_widget_set_sensitive(io->applyB,TRUE);
}

static void applyCB( GtkWidget *W, struct _fileIO *io)
{
  int mem_type_reached;
  WordA address_reached; // reached
  int reached_how;
  int put_get; // put or get
  int amount;
  gchar *filename;
  int type_access;
  int address_access;
  gchar *str;

  str = gtk_entry_get_text(GTK_ENTRY(GTK_COMBO(io->mem_type_reachedW)->entry));

  if ( strcmp(str,"data") == 0 )
    {
      mem_type_reached = DATA_MEM_TYPE;
      printf("Data picked\n");
    }
  else if ( strcmp(str,"program") == 0 )
    {
      mem_type_reached = PROGRAM_MEM_TYPE;
      printf("Program picked\n");
    }
  else
    {
      printf("Error! %s:%d\n",__FILE__,__LINE__);
      return;
    }

  str = gtk_entry_get_text(GTK_ENTRY(io->address_reachedW));

  if ( !text_to_address(str,&address_reached) )
    return;

  str = gtk_entry_get_text(GTK_ENTRY(GTK_COMBO(io->reached_howW)->entry));

  if ( strcmp(str,"is read") == 0 )
    reached_how = 0;
  else if ( strcmp(str,"is written") == 0 )
    reached_how = 1;
  else if ( strcmp(str,"pipeline is executed") == 0 )
    reached_how = 2;
  else 
    {
      printf("Error! %s:%d\n",__FILE__,__LINE__);
      return;
    }

  str = gtk_entry_get_text(GTK_ENTRY(GTK_COMBO(io->put_getW)->entry));

  if ( strcmp(str,"put") == 0 )
    put_get = 0;
  else if ( strcmp(str,"get") == 0 )
    put_get = 1;
  else
    {
      printf("Error! %s:%d\n",__FILE__,__LINE__);
      return;
    }

  str = gtk_entry_get_text(GTK_ENTRY(io->amountW));
  if ( !text_to_int(str,&amount) )
    return;

  str = gtk_entry_get_text(GTK_ENTRY(io->filenameW));
  filename = str;


  str = gtk_entry_get_text(GTK_ENTRY(GTK_COMBO(io->type_accessW)->entry));

  if ( strcmp(str,"data") == 0 )
    {
      type_access = DATA_MEM_TYPE;
    }
  else if ( strcmp(str,"program") == 0 )
    {
      type_access = PROGRAM_MEM_TYPE;
    }
  else
    {
      printf("Error! %s:%d\n",__FILE__,__LINE__);
      return;
    }


  str = gtk_entry_get_text(GTK_ENTRY(io->address_accessW));

  if ( !text_to_address(str,&address_access) )
    return;

  // OK, everything decoded properly, can apply
  io->mem_type_reached = mem_type_reached;
  io->address_reached = address_reached;
  io->reached_how = reached_how;
  io->put_get = put_get;
  io->amount = amount;
  io->filename = g_strdup(filename);
  io->type_access = type_access;
  io->address_access = address_access;

  if ( io->valid == 0 )
    {
      GtkWidget *connect_box,*separator;

      fileIOL = g_list_append(fileIOL,io);
      io->valid = 1;
  
      separator = gtk_hseparator_new();
      gtk_widget_show(separator);
      gtk_box_pack_start(GTK_BOX(vbox_main),separator,FALSE,FALSE,0);

      io_new = g_new(struct _fileIO,1);
      io_new->valid = 0;
      connect_box = create_io(io_new);
      gtk_box_pack_start(GTK_BOX(vbox_main),connect_box,FALSE,FALSE,0);

      io_new->connect_box = connect_box;

      gtk_widget_set_sensitive(io->applyB,FALSE);
      gtk_widget_set_sensitive(io->removeB,TRUE);

    }

  fill_io(io);
}

static void removeCB( GtkWidget *W, struct _fileIO *io )
{
  gtk_widget_hide(io->connect_box);
}

static void closeCB( GtkWidget *W, struct _fileIO *io )
{
  fclose(io->file);
  io->file = NULL;
  gtk_widget_set_sensitive(io->closeB,FALSE);
}


static void fill_io(struct _fileIO *io)
{
  gchar tmp_str[100];

  if ( io->valid )
    {
      if ( io->mem_type_reached == DATA_MEM_TYPE )
	gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(io->mem_type_reachedW)->entry),
			   "data");
      else if ( io->mem_type_reached == PROGRAM_MEM_TYPE )
	gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(io->mem_type_reachedW)->entry),
			   "program");

      g_snprintf(tmp_str,(size_t)100,"0x%x",io->address_reached);
      gtk_entry_set_text(GTK_ENTRY(io->address_reachedW), tmp_str);

      switch (io->reached_how)
	{
	case 0:
	  gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(io->reached_howW)->entry),
			     "is read");
	  break;
	case 1:
	  gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(io->reached_howW)->entry),
			     "is written");
	  break;
	case 2:
	  gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(io->reached_howW)->entry),
			     "pipeline is executed");
	  break;
	}
      

      switch (io->put_get)
	{
	case 0:
	  gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(io->put_getW)->entry),
			     "put");
	  break;
	case 1:
	  gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(io->put_getW)->entry),
			     "get");
	  break;
	}

      g_snprintf(tmp_str,(size_t)100,"%d",io->amount);
      gtk_entry_set_text(GTK_ENTRY(io->amountW), tmp_str);

      gtk_entry_set_text(GTK_ENTRY(io->filenameW), io->filename);

      if ( io->type_access == DATA_MEM_TYPE )
	gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(io->type_accessW)->entry),
			   "data");
      else if ( io->type_access == PROGRAM_MEM_TYPE )
	gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(io->type_accessW)->entry),
			   "program");

      g_snprintf(tmp_str,(size_t)100,"0x%x",io->address_access);
      gtk_entry_set_text(GTK_ENTRY(io->address_accessW), tmp_str);
    }
}

static GtkWidget *create_io(struct _fileIO *io)
{
  GtkWidget *vbox,*hbox;
  GtkWidget *label, *entry, *combo;
  GList *items;

  vbox = gtk_vbox_new(FALSE,0);
  hbox = gtk_hbox_new(FALSE,0);

  gtk_box_pack_start(GTK_BOX(vbox),hbox,FALSE,FALSE,0);

  // When
  label = gtk_label_new(" When ");
  gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,0);
  gtk_widget_show(label);
  
  // Program/Data
  items = NULL;
  items = g_list_append(items, "data");
  items = g_list_append(items, "program");
  combo = gtk_combo_new();
  gtk_widget_set_usize(GTK_WIDGET(GTK_COMBO(combo)->entry),7*ENTRY_CHAR_WIDTH,ENTRY_CHAR_HEIGHT); 
  gtk_combo_set_popdown_strings (GTK_COMBO (combo), items);
  gtk_box_pack_start(GTK_BOX(hbox),combo,FALSE,FALSE,0);
  gtk_widget_show(combo);
  gtk_editable_set_editable(GTK_EDITABLE(GTK_COMBO(combo)->entry),FALSE);
  io->mem_type_reachedW = combo;

  printf("combo widget 0x%x\n",(unsigned int)combo);
  // address
  label = gtk_label_new(" address ");
  gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,0);
  gtk_widget_show(label);

  // []
  entry = gtk_entry_new();
#ifdef GTK2
  gtk_entry_set_width_chars(GTK_ENTRY(entry),13);
#else
  gtk_widget_set_usize(GTK_WIDGET(entry),13*ENTRY_CHAR_WIDTH,ENTRY_CHAR_HEIGHT); 
#endif
  gtk_widget_show (entry);  
  gtk_entry_set_max_length(GTK_ENTRY(entry),13);
  gtk_box_pack_start(GTK_BOX(hbox),entry,FALSE,FALSE,0);
  gtk_widget_show (entry);  
  io->address_reachedW = entry;
  gtk_signal_connect(GTK_OBJECT(entry), "changed",
		     GTK_SIGNAL_FUNC (address_reachedCB), io);

  // is ___
  items = NULL;
  items = g_list_append(items, "is read");
  items = g_list_append(items, "is written");
  items = g_list_append(items, "pipeline is executed");
  combo = gtk_combo_new();
  gtk_combo_set_popdown_strings (GTK_COMBO (combo), items);
  io->reached_howW = combo;
  gtk_box_pack_start(GTK_BOX(hbox),combo,FALSE,FALSE,0);
  gtk_widget_set_usize(GTK_WIDGET(GTK_COMBO(combo)->entry),20*ENTRY_CHAR_WIDTH,ENTRY_CHAR_HEIGHT); 
  gtk_widget_show(combo);
  gtk_editable_set_editable(GTK_EDITABLE(GTK_COMBO(combo)->entry),FALSE);
  io->reached_howW = combo;

  // New line
  gtk_widget_show(hbox);
  hbox = gtk_hbox_new(FALSE,0);
  gtk_box_pack_start(GTK_BOX(vbox),hbox,FALSE,FALSE,0);

  // put/get
  items = NULL;
  items = g_list_append(items, "put");
  items = g_list_append(items, "get");
  combo = gtk_combo_new();
  gtk_widget_set_usize(GTK_WIDGET(combo),6*ENTRY_CHAR_WIDTH,ENTRY_CHAR_HEIGHT); 
  gtk_combo_set_popdown_strings (GTK_COMBO (combo), items);
  gtk_box_pack_start(GTK_BOX(hbox),combo,FALSE,FALSE,0);
  gtk_widget_show(combo);
  gtk_editable_set_editable(GTK_EDITABLE(GTK_COMBO(combo)->entry),FALSE);
  io->put_getW = combo;
 
  entry = gtk_entry_new();
#ifdef GTK2
  gtk_entry_set_width_chars(GTK_ENTRY(entry),13);
#else
  gtk_widget_set_usize(GTK_WIDGET(entry),13*ENTRY_CHAR_WIDTH,ENTRY_CHAR_HEIGHT); 
#endif
  gtk_widget_show (entry);  
  gtk_entry_set_max_length(GTK_ENTRY(entry),13);
  gtk_box_pack_start(GTK_BOX(hbox),entry,FALSE,FALSE,0);
  gtk_widget_show (entry);  
  io->amountW = entry;
  gtk_signal_connect(GTK_OBJECT(entry), "changed",
		     GTK_SIGNAL_FUNC (amountCB), io);

  // entries of
  label = gtk_label_new(" entries of file ");
  gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,0);
  gtk_widget_show(label);

 
  entry = gtk_entry_new();
#ifdef GTK2
  gtk_entry_set_width_chars(GTK_ENTRY(entry),13);
#else
  gtk_widget_set_usize(GTK_WIDGET(entry),13*ENTRY_CHAR_WIDTH,ENTRY_CHAR_HEIGHT); 
#endif
  gtk_widget_show (entry);  
  gtk_entry_set_max_length(GTK_ENTRY(entry),13);
  gtk_box_pack_start(GTK_BOX(hbox),entry,FALSE,FALSE,0);
  gtk_widget_show (entry);  
  io->filenameW = entry;
  gtk_signal_connect(GTK_OBJECT(entry), "changed",
		     GTK_SIGNAL_FUNC (filenameCB), io);

  // file

  // into
  label = gtk_label_new(" into ");
  gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,0);
  gtk_widget_show(label);


  // program/data
  items = NULL;
  items = g_list_append(items, "data");
  items = g_list_append(items, "program");
  combo = gtk_combo_new();
  gtk_widget_set_usize(GTK_WIDGET(GTK_COMBO(combo)->entry),7*ENTRY_CHAR_WIDTH,ENTRY_CHAR_HEIGHT); 
  gtk_combo_set_popdown_strings (GTK_COMBO (combo), items);
  gtk_box_pack_start(GTK_BOX(hbox),combo,FALSE,FALSE,0);
  gtk_widget_show(combo);
  gtk_editable_set_editable(GTK_EDITABLE(GTK_COMBO(combo)->entry),FALSE);
  io->type_accessW = combo;

  // address
  label = gtk_label_new(" address ");
  gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,0);
  gtk_widget_show(label);

 
 entry = gtk_entry_new();
#ifdef GTK2
  gtk_entry_set_width_chars(GTK_ENTRY(entry),13);
#else
  gtk_widget_set_usize(GTK_WIDGET(entry),13*ENTRY_CHAR_WIDTH,ENTRY_CHAR_HEIGHT); 
#endif
  gtk_widget_show (entry);  
  gtk_entry_set_max_length(GTK_ENTRY(entry),13);
  gtk_box_pack_start(GTK_BOX(hbox),entry,FALSE,FALSE,0);
  gtk_widget_show (entry);  
  io->address_accessW = entry;
  gtk_signal_connect(GTK_OBJECT(entry), "changed",
		     GTK_SIGNAL_FUNC (address_accessCB), io);

  gtk_widget_show(hbox);
  gtk_widget_show(vbox);
  
  hbox = gtk_hbox_new(FALSE,0);
  gtk_box_pack_start(GTK_BOX(vbox),hbox,FALSE,FALSE,0);
  gtk_widget_show(hbox);

  io->applyB = gtk_button_new_with_label("Apply");
  gtk_box_pack_start(GTK_BOX(hbox),io->applyB,FALSE,FALSE,0);
  gtk_signal_connect(GTK_OBJECT(io->applyB), "pressed",
		     GTK_SIGNAL_FUNC (applyCB), io);
  gtk_widget_set_sensitive(io->applyB,FALSE);
  gtk_widget_show(io->applyB);

  io->removeB = gtk_button_new_with_label("Remove");
  gtk_box_pack_start(GTK_BOX(hbox),io->removeB,FALSE,FALSE,0);
  gtk_signal_connect(GTK_OBJECT(io->removeB), "pressed",
		     GTK_SIGNAL_FUNC (removeCB), io);
  gtk_widget_show(io->removeB);

  if (io->valid)
    {
      io->modified = 0xf;
      fill_io(io);
    }
  else
    {
      gtk_widget_set_sensitive(io->removeB,FALSE);
    }

  io->closeB = gtk_button_new_with_label("Close File");
  gtk_box_pack_start(GTK_BOX(hbox),io->closeB,FALSE,FALSE,0);
  gtk_signal_connect(GTK_OBJECT(io->closeB), "pressed",
		     GTK_SIGNAL_FUNC (closeCB), io);
  if ( io->file )
    gtk_widget_set_sensitive(io->closeB,TRUE);
  else
    gtk_widget_set_sensitive(io->closeB,FALSE);
  gtk_widget_show(io->closeB);

  return vbox;
}

void create_fileIO(GtkWidget *widget, gpointer data)
{
  GtkWidget *scrolledW,*connect_box,*separator;
  struct _fileIO *io;
  int num_file=0;
  GList *list;

  if ( fileIOW != NULL )
    return;

  fileIOW = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_widget_set_usize(fileIOW,600,300); 

  gtk_signal_connect(GTK_OBJECT(fileIOW),"destroy",
		     (GtkSignalFunc)destroy_window_CB,NULL);

  gtk_widget_set_name( fileIOW, "Connect File" );
  gtk_window_set_title( GTK_WINDOW(fileIOW), "Connect File" );
  gtk_container_set_border_width ( GTK_CONTAINER(fileIOW), 0);

  scrolledW = gtk_scrolled_window_new(NULL, NULL);
  gtk_widget_show(scrolledW);
  gtk_container_add(GTK_CONTAINER (fileIOW), scrolledW);

  vbox_main = gtk_vbox_new(FALSE,6);
  gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(scrolledW),vbox_main);
  gtk_widget_show(vbox_main);

  // Create already created fileIO
  list = fileIOL;
  while ( list )
    {
      io = list->data;
      io->modified = 0;
      connect_box = create_io((struct _fileIO *)list->data);
      gtk_box_pack_start(GTK_BOX(vbox_main),connect_box,FALSE,FALSE,0);
      io->connect_box = connect_box;
      
      separator = gtk_hseparator_new();
      gtk_widget_show(separator);
      gtk_box_pack_start(GTK_BOX(vbox_main),separator,FALSE,FALSE,0);

      list=list->next;
      num_file++;
    }

  
  io_new = g_new(struct _fileIO,1);
  io_new->valid = 0;
  connect_box = create_io(io_new);
  gtk_box_pack_start(GTK_BOX(vbox_main),connect_box,FALSE,FALSE,0);

  io_new->connect_box = connect_box;
  io_new->modified = 0;

  gtk_widget_show(fileIOW);
  
}

void fileIO_process(struct _fileIO *io)
{
  int k,available,amount_written,wait_state;
  Word wrd;

  if ( io->put_get == 0 )
    {
      // Open file if needed
      if ( io->file == NULL )
	{
	  io->file = fopen(io->filename,"w");
	  if ( io->file == NULL )
	    {
	      printf("Error cannot create file!\n");
	      return;
	    }
	  gtk_widget_set_sensitive(io->closeB,TRUE);
	}

      // putting into a file
      for (k=0;k<io->amount;k++)
	{
	  wrd = read_mem(io->address_access+k,&wait_state,io->type_access,&available);
	  if ( available == 0 )
	    {
	      printf("Warning reading unitialized memory\n");
	    }
	  amount_written = fprintf(io->file,"0x%x",wrd);
	  if ( amount_written <= 0 )
	    {
	      printf("Error cannot write to file!\n");
	      return;
	    }
	
	}
    }
  else
    {
      // Open file if needed
      if ( io->file == NULL )
	{
	  io->file = fopen(io->filename,"r");
	  if ( io->file == NULL )
	    {
	      printf("Error cannot open file!\n");
	      return;
	    }
	  gtk_widget_set_sensitive(io->closeB,TRUE);
	}
      // reading from a file 
      for (k=0;k<io->amount;k++)
	{
	  if ( word_from_file(io->file,&wrd) )
	    {
	      wait_state = write_mem(io->address_access+k,wrd,io->type_access);
	    }
	  
	  else
	    {
		  // end of file or error
	    }
	}
      
    }      
  
}
