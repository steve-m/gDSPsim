#include "fileIO.h"
#include <stdio.h>
#include "memory.h"
#include "string.h"

static GtkWidget *fileIOW=NULL;

static void destroy_window_CB( GtkWidget *W, gpointer data )
{
  fileIOW=NULL;
}

static void set_mem_typeCB(GtkWidget *W, GtkCombo *combo)
{
  printf("entry_change_id=%d\n",combo->entry_change_id);
  printf("list_change_id=%d\n",combo->list_change_id);
  printf("current_button=%d\n",combo->current_button);
  printf("activate_id=%d\n",combo->activate_id);
}
static void set_mem_typeCB2(GtkWidget *W, int *mem_type)
{
  gchar *ch;

  printf("prg_data\n");
  printf("%s\n",gtk_editable_get_chars(GTK_EDITABLE(W),0,-1));
  ch = gtk_editable_get_chars(GTK_EDITABLE(W),0,-1);
  if ( strcmp(ch,"data") == 0 )
    *mem_type = DATA_MEM_TYPE;
  else if ( strcmp(ch,"program") == 0 )
    *mem_type = PROGRAM_MEM_TYPE;
  else
    *mem_type = DATA_MEM_TYPE | PROGRAM_MEM_TYPE;

  {
    GtkCombo *combo;
    combo = GTK_COMBO(W);
  printf("entry_change_id=%d\n",combo->entry_change_id);
  printf("list_change_id=%d\n",combo->list_change_id);
  printf("current_button=%d\n",combo->current_button);
  printf("activate_id=%d\n",combo->activate_id);
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
  label = gtk_label_new("When");
  gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,0);
  gtk_widget_show(label);
  
  // Program/Data
  items = NULL;
  items = g_list_append(items, "data/program");
  items = g_list_append(items, "data");
  items = g_list_append(items, "program");
  combo = gtk_combo_new();
  gtk_combo_set_popdown_strings (GTK_COMBO (combo), items);
  gtk_box_pack_start(GTK_BOX(hbox),combo,FALSE,FALSE,0);
  gtk_widget_show(combo);
  gtk_editable_set_editable(GTK_EDITABLE(GTK_COMBO(combo)->entry),FALSE);
  //gtk_signal_connect(GTK_OBJECT(GTK_COMBO(combo)->entry), "changed",
  //	     GTK_SIGNAL_FUNC (set_mem_typeCB), &io->access_mem_type);
  gtk_signal_connect(GTK_OBJECT(GTK_COMBO(combo)->entry), "changed",
		     GTK_SIGNAL_FUNC (set_mem_typeCB), combo);

  // address
  label = gtk_label_new("address");
  gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,0);
  gtk_widget_show(label);

  // []
  entry = gtk_entry_new();
#ifdef GTK2
  gtk_entry_set_width_chars(GTK_ENTRY(entry),13);
#endif
  gtk_widget_show (entry);  
  gtk_entry_set_max_length(GTK_ENTRY(entry),13);
  gtk_box_pack_start(GTK_BOX(hbox),entry,FALSE,FALSE,0);
  gtk_widget_show (entry);  
  
  // is ___
  items = NULL;
  items = g_list_append(items, "is read");
  items = g_list_append(items, "is written");
  items = g_list_append(items, "pipeline is prefetched");
  items = g_list_append(items, "pipeline is fetched");
  items = g_list_append(items, "pipeline is decoded");
  items = g_list_append(items, "pipeline is read");
  items = g_list_append(items, "pipeline is accessed");
  items = g_list_append(items, "pipeline is executed");
  combo = gtk_combo_new();
  gtk_combo_set_popdown_strings (GTK_COMBO (combo), items);
  gtk_box_pack_start(GTK_BOX(hbox),combo,FALSE,FALSE,0);
  gtk_widget_show(combo);
  gtk_editable_set_editable(GTK_EDITABLE(GTK_COMBO(combo)->entry),FALSE);

  // New line
  gtk_widget_show(hbox);
  hbox = gtk_hbox_new(FALSE,0);
  gtk_box_pack_start(GTK_BOX(vbox),hbox,FALSE,FALSE,0);

  // put/get
  items = NULL;
  items = g_list_append(items, "put");
  items = g_list_append(items, "get");
  combo = gtk_combo_new();
  gtk_combo_set_popdown_strings (GTK_COMBO (combo), items);
  gtk_box_pack_start(GTK_BOX(hbox),combo,FALSE,FALSE,0);
  gtk_widget_show(combo);
  gtk_editable_set_editable(GTK_EDITABLE(GTK_COMBO(combo)->entry),FALSE);

 
  entry = gtk_entry_new();
#ifdef GTK2
  gtk_entry_set_width_chars(GTK_ENTRY(entry),13);
#endif
  gtk_widget_show (entry);  
  gtk_entry_set_max_length(GTK_ENTRY(entry),13);
  gtk_box_pack_start(GTK_BOX(hbox),entry,FALSE,FALSE,0);
  gtk_widget_show (entry);  

  // entries of
  label = gtk_label_new("entries of file");
  gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,0);
  gtk_widget_show(label);

 
  entry = gtk_entry_new();
#ifdef GTK2
  gtk_entry_set_width_chars(GTK_ENTRY(entry),13);
#endif
  gtk_widget_show (entry);  
  gtk_entry_set_max_length(GTK_ENTRY(entry),13);
  gtk_box_pack_start(GTK_BOX(hbox),entry,FALSE,FALSE,0);
  gtk_widget_show (entry);  
 
  // file

  // into
  label = gtk_label_new("into");
  gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,0);
  gtk_widget_show(label);


  // program/data
  items = NULL;
  items = g_list_append(items, "data");
  items = g_list_append(items, "program");
  combo = gtk_combo_new();
  gtk_combo_set_popdown_strings (GTK_COMBO (combo), items);
  gtk_box_pack_start(GTK_BOX(hbox),combo,FALSE,FALSE,0);
  gtk_widget_show(combo);
  gtk_editable_set_editable(GTK_EDITABLE(GTK_COMBO(combo)->entry),FALSE);
  gtk_signal_connect(GTK_OBJECT(GTK_COMBO(combo)->entry), "changed",
		     GTK_SIGNAL_FUNC (set_mem_typeCB), &io->place_mem_type);

  // address
  label = gtk_label_new("address");
  gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,0);
  gtk_widget_show(label);

 
 entry = gtk_entry_new();
#ifdef GTK2
  gtk_entry_set_width_chars(GTK_ENTRY(entry),13);
#endif
  gtk_widget_show (entry);  
  gtk_entry_set_max_length(GTK_ENTRY(entry),13);
  gtk_box_pack_start(GTK_BOX(hbox),entry,FALSE,FALSE,0);
  gtk_widget_show (entry);  

  gtk_widget_show(hbox);
  gtk_widget_show(vbox);
  
  return vbox;
}

void create_fileIO(GtkWidget *widget, gpointer data)
{
  GtkWidget *vbox;
  struct _fileIO *io;

  if ( fileIOW != NULL )
    return;

  io = g_new(struct _fileIO,1);

  printf("fileIO\n");
  fileIOW = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_signal_connect(GTK_OBJECT(fileIOW),"destroy",
		     (GtkSignalFunc)destroy_window_CB,NULL);

  gtk_widget_set_name( fileIOW, "Connect File" );
  gtk_window_set_title( GTK_WINDOW(fileIOW), "Connect File" );
  gtk_container_set_border_width ( GTK_CONTAINER(fileIOW), 0);

  vbox = create_io(io);
  gtk_container_add (GTK_CONTAINER (fileIOW), vbox);

  gtk_widget_show(fileIOW);


  
}
