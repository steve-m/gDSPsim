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

#include <fileIO.h>
#include <stdio.h>
#include <memory.h>
#include <string.h>
#include <entryCB.h>
#include <pipeline.h>

static GtkWidget *fileOW=NULL;
static GtkWidget *fileIW=NULL;
GList *fileIOL=NULL;
GtkWidget *vbox_main;
struct _fileIO *io_new;


static GtkWidget *create_io(struct _fileIO *io);
static void fill_io(struct _fileIO *io);

static void destroy_window_CB( GtkWidget *W, gpointer data )
{
  if ( (int)data == 1 )
    fileIW = NULL;
  else
    fileOW=NULL;
}

static void destroy_io(struct _fileIO *io)
{
  if ( io )
    {
      // destroys this structure
      if ( io->filename )
	g_free(io->filename);
      if ( io->file )
	fclose(io->file);

      // fixme, destoy widgets
      g_free(io);
    }
}
static void address_reachedCB( GtkWidget *W, struct _fileIO *io)
{
  // fixme. make sure a valid address has been set before setting
  // the modified bit
  io->modified |= ADDRESS_REACHED_SET;
  if ( io->modified == ALL_SET )
    gtk_widget_set_sensitive(io->applyB,TRUE);
}

static void amountCB( GtkWidget *W, struct _fileIO *io)
{
  // fixme. make sure a valid amount has been set before setting
  // the modified bit
  io->modified |= AMOUNT_SET;
  if ( io->modified == ALL_SET )
    gtk_widget_set_sensitive(io->applyB,TRUE);
}

static void filenameCB( GtkWidget *W, struct _fileIO *io)
{
  // fixme. make sure a valid filename has been set before setting
  // the modified bit
  io->modified |= FILENAME_SET;
  if ( io->modified == ALL_SET )
    gtk_widget_set_sensitive(io->applyB,TRUE);
}

static void address_accessCB( GtkWidget *W, struct _fileIO *io)
{
  // fixme. make sure a valid filename has been set before setting
  // the modified bit
  io->modified |= ADDRESS_ACCESS_SET;
  if ( io->modified == ALL_SET )
    gtk_widget_set_sensitive(io->applyB,TRUE);
}

static void applyCB( GtkWidget *W, struct _fileIO *io)
{
  int mem_type_reached;
  WordP address_reached; // reached
  int reached_how;
  int amount;
  gchar *filename;
  int type_access;
  int address_access;
  gchar *str;
  void (*registerF)(struct _fileIO *io);

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
    {
      reached_how = MEMORY_READ;
      registerF = set_fileIO_break_on_memory;
    }
  else if ( strcmp(str,"is written") == 0 )
    {
      reached_how = MEMORY_WRITE;
      registerF = set_fileIO_break_on_memory;
    }
  else if ( strcmp(str,"is executed") == 0 )
    {
      reached_how = PIPELINE_EXECUTED;
      registerF = set_fileIO_break_on_pipeline;
    }
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
  io->amount = amount;
  io->filename = g_strdup(filename);
  io->type_access = type_access;
  io->address_access = address_access;
  io->reached_how = reached_how;

  if ( io->valid == 0 )
    {
      GtkWidget *connect_box,*separator;

      io->registerF = registerF;
      io->registerF(io);

      fileIOL = g_list_append(fileIOL,io);
      io->valid = 1;
  
      separator = gtk_hseparator_new();
      gtk_widget_show(separator);
      gtk_box_pack_start(GTK_BOX(vbox_main),separator,FALSE,FALSE,0);

      io_new = g_new(struct _fileIO,1);
      io_new->valid = 0;
      io_new->input = io->input;
      connect_box = create_io(io_new);
      gtk_box_pack_start(GTK_BOX(vbox_main),connect_box,FALSE,FALSE,0);

      io_new->connect_box = connect_box;

      gtk_widget_set_sensitive(io->removeB,TRUE);

    }
  else
    {
      // Reapply
      if ( io->registerF == registerF )
	{
	  // Update
	  io->updateF(io);
	}
      else
	{
	  // Delete old, register new
	  io->removeF(io);
	  io->registerF=registerF;
	  io->registerF(io);
	}
    

    }

  fill_io(io);
}

static void removeCB( GtkWidget *W, struct _fileIO *io )
{
  gtk_widget_set_sensitive(io->removeB,FALSE);
  // unregister set
  io->removeF(io);
  io->registerF=NULL;
  if ( io->valid == 1 )
    {
      gtk_widget_hide(io->connect_box);
      // Totally delete
      destroy_io(io);
    }
}

static void restartCB( GtkWidget *W, struct _fileIO *io )
{
  fclose(io->file);
  io->file = NULL;
  gtk_widget_set_sensitive(io->restartB,FALSE);
}

static void flushCB( GtkWidget *W, struct _fileIO *io )
{
  fflush(io->file);
}

// Fills up the widget structure based upon io.
static void fill_io(struct _fileIO *io)
{
  gchar tmp_str[100];

  if ( io->mem_type_reached == DATA_MEM_TYPE )
    gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(io->mem_type_reachedW)->entry),
		       "data");
  else if ( io->mem_type_reached == PROGRAM_MEM_TYPE )
    gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(io->mem_type_reachedW)->entry),
		       "program");

  if ( io->modified & ADDRESS_REACHED_SET )
    {
      g_snprintf(tmp_str,(size_t)100,"0x%x",io->address_reached);
      gtk_entry_set_text(GTK_ENTRY(io->address_reachedW), tmp_str);
    }

  switch (io->reached_how)
    {
    case MEMORY_READ:
      gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(io->reached_howW)->entry),
			 "is read");
      break;
    case MEMORY_WRITE:
      gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(io->reached_howW)->entry),
			 "is written");
      break;
    case PIPELINE_EXECUTED:
      gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(io->reached_howW)->entry),
			 "is executed");
      break;
    }
  
  
  if ( io->modified & AMOUNT_SET )
    {
      g_snprintf(tmp_str,(size_t)100,"%d",io->amount);
      gtk_entry_set_text(GTK_ENTRY(io->amountW), tmp_str);
    }
  
  if ( (io->modified & FILENAME_SET) && io->filename )
    {
      gtk_entry_set_text(GTK_ENTRY(io->filenameW), io->filename);
    }

  if ( io->type_access == DATA_MEM_TYPE )
    gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(io->type_accessW)->entry),
		       "data");
  else if ( io->type_access == PROGRAM_MEM_TYPE )
    gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(io->type_accessW)->entry),
		       "program");
  
  if ( io->modified & ADDRESS_ACCESS_SET )
    {
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
  items = g_list_append(items, "is executed");
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

  // read/write
  if ( io->input )
    label = gtk_label_new(" read ");
  else
    label = gtk_label_new(" write ");
  gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,0);
  gtk_widget_show(label);
 
  // Number of data
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
  label = gtk_label_new(" entries of ");
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
  label = gtk_label_new(" memory, address ");
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


  // into/from
  if ( io->input )
    label = gtk_label_new(" from file ");
  else
    label = gtk_label_new(" into file ");
  gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,0);
  gtk_widget_show(label);

 
  // file
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

  if (io->valid==0)
    {
      // Set defaults
      io->mem_type_reached = PROGRAM_MEM_TYPE;
      io->address_reached = 0x0;
      io->reached_how = PIPELINE_EXECUTED;
      io->amount = 0;
      io->filename = NULL;
      io->type_access = DATA_MEM_TYPE;
      io->address_access = 0;
      io->file = NULL;
      io->modified = 0x0;
      io->registerF = NULL;
      io->updateF = NULL;
      io->removeF = NULL;

      gtk_widget_set_sensitive(io->removeB,FALSE);
    }

  fill_io(io);

  // Restart Button
  io->restartB = gtk_button_new_with_label("Restart File");
  gtk_box_pack_start(GTK_BOX(hbox),io->restartB,FALSE,FALSE,0);
  gtk_signal_connect(GTK_OBJECT(io->restartB), "pressed",
		     GTK_SIGNAL_FUNC (restartCB), io);
  if ( io->file )
    gtk_widget_set_sensitive(io->restartB,TRUE);
  else
    gtk_widget_set_sensitive(io->restartB,FALSE);
  gtk_widget_show(io->restartB);

  if ( io->input != 0 )
    {
      io->flushB = gtk_button_new_with_label("Flush File");
      gtk_box_pack_start(GTK_BOX(hbox),io->flushB,FALSE,FALSE,0);
      gtk_signal_connect(GTK_OBJECT(io->flushB), "pressed",
			 GTK_SIGNAL_FUNC (flushCB), io);
      if ( io->file )
	gtk_widget_set_sensitive(io->flushB,TRUE);
      else
	gtk_widget_set_sensitive(io->flushB,FALSE);
      gtk_widget_show(io->flushB);
    }

  return vbox;
}

void create_fileIO(GtkWidget *widget, gpointer data)
{
  GtkWidget *scrolledW,*connect_box,*separator,*mainW;
  struct _fileIO *io;
  int num_file=0;
  GList *list;

  if ( (int)data == 0 )
    {
      if ( fileOW != NULL )
	return;

      fileOW = gtk_window_new (GTK_WINDOW_TOPLEVEL);
      gtk_widget_set_name( fileOW, "Connect Data to an Output File" );
      gtk_window_set_title( GTK_WINDOW(fileOW), "Connect Data to an Output File" );
      mainW = fileOW;
    }
  else
    {
      if ( fileIW != NULL )
	return;
      
      fileIW = gtk_window_new (GTK_WINDOW_TOPLEVEL);
      gtk_widget_set_name( fileIW, "Connect Input File to Data" );
      gtk_window_set_title( GTK_WINDOW(fileIW), "Connect Input File to Data" );
      mainW = fileIW;
    }

  gtk_widget_set_usize(mainW,660,300); 

  gtk_signal_connect(GTK_OBJECT(mainW),"destroy",
		     (GtkSignalFunc)destroy_window_CB,data);

  gtk_container_set_border_width ( GTK_CONTAINER(mainW), 0);

  scrolledW = gtk_scrolled_window_new(NULL, NULL);
  gtk_widget_show(scrolledW);
  gtk_container_add(GTK_CONTAINER (mainW), scrolledW);

  vbox_main = gtk_vbox_new(FALSE,6);
  gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(scrolledW),vbox_main);
  gtk_widget_show(vbox_main);

  // Create already created fileIO
  list = fileIOL;
  while ( list )
    {
      io = list->data;
      if ( io->input == (int)data )
	{
	  connect_box = create_io((struct _fileIO *)list->data);

	  gtk_box_pack_start(GTK_BOX(vbox_main),connect_box,FALSE,FALSE,0);
	  io->connect_box = connect_box;
	  
	  separator = gtk_hseparator_new();
	  gtk_widget_show(separator);
	  gtk_box_pack_start(GTK_BOX(vbox_main),separator,FALSE,FALSE,0);
	  
	  list=list->next;
	  num_file++;
	}
    }

  
  io_new = g_new(struct _fileIO,1);
  io_new->valid = 0;
  io_new->input = (int)data;
  
  connect_box = create_io(io_new);

  gtk_box_pack_start(GTK_BOX(vbox_main),connect_box,FALSE,FALSE,0);

  io_new->connect_box = connect_box;
  io_new->modified = 0;

  gtk_widget_show(mainW);
  
}

void fileIO_process(struct _fileIO *io)
{
  int k,available,amount_written,wait_state;
  Word wrd;

  if ( io->input == 0 )
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
	  gtk_widget_set_sensitive(io->restartB,TRUE);
	}

      // putting into a file
      for (k=0;k<io->amount;k++)
	{
	  wrd = read_mem(io->address_access+k,&wait_state,io->type_access,&available);
	  if ( available == 0 )
	    {
	      printf("Warning reading unitialized memory\n");
	    }
	  amount_written = fprintf(io->file,"0x%x\n",wrd);
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
	  gtk_widget_set_sensitive(io->restartB,TRUE);
       	  gtk_widget_set_sensitive(io->flushB,TRUE);
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
