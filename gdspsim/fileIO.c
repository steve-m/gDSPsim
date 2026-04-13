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

/*
 * GTK3 port: the original code used deprecated GtkCombo for drop-downs
 * (text+popdown pair). We use GtkComboBoxText instead, which gives a
 * single widget; helper macros below map the old ->entry accesses.
 */

#include <fileIO.h>
#include <stdio.h>
#include <string.h>
#include <memory.h>
#include <entryCB.h>
#include <pipeline.h>

#define COMBO_TEXT(combo) \
  gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(combo))

static GtkWidget *fileOW = NULL;
static GtkWidget *fileIW = NULL;
GList *fileIOL = NULL;
GtkWidget *vbox_main;
struct _fileIO *io_new;

static GtkWidget *create_io(struct _fileIO *io);
static void fill_io(struct _fileIO *io);

static void destroy_window_CB(GtkWidget *W, gpointer data)
{
  if (GPOINTER_TO_INT(data) == 1)
    fileIW = NULL;
  else
    fileOW = NULL;
}

static void destroy_io(struct _fileIO *io)
{
  if (io)
    {
      if (io->filename) g_free(io->filename);
      if (io->file)     fclose(io->file);
      g_free(io);
    }
}

static void address_reachedCB(GtkWidget *W, struct _fileIO *io)
{
  io->modified |= ADDRESS_REACHED_SET;
  if (io->modified == ALL_SET)
    gtk_widget_set_sensitive(io->applyB, TRUE);
}

static void amountCB(GtkWidget *W, struct _fileIO *io)
{
  io->modified |= AMOUNT_SET;
  if (io->modified == ALL_SET)
    gtk_widget_set_sensitive(io->applyB, TRUE);
}

static void filenameCB(GtkWidget *W, struct _fileIO *io)
{
  io->modified |= FILENAME_SET;
  if (io->modified == ALL_SET)
    gtk_widget_set_sensitive(io->applyB, TRUE);
}

static void address_accessCB(GtkWidget *W, struct _fileIO *io)
{
  io->modified |= ADDRESS_ACCESS_SET;
  if (io->modified == ALL_SET)
    gtk_widget_set_sensitive(io->applyB, TRUE);
}

static int combo_picks_mem_type(GtkWidget *combo, int *out)
{
  gchar *str = COMBO_TEXT(combo);
  if (str == NULL)
    return 0;
  if (strcmp(str, "data") == 0)
    *out = DATA_MEM_TYPE;
  else if (strcmp(str, "program") == 0)
    *out = PROGRAM_MEM_TYPE;
  else
    {
      g_free(str);
      return 0;
    }
  g_free(str);
  return 1;
}

static void applyCB(GtkWidget *W, struct _fileIO *io)
{
  int mem_type_reached;
  WordP address_reached;
  int reached_how;
  int amount;
  gchar *filename;
  int type_access;
  int address_access;
  const gchar *str;
  gchar *str_owned;
  void (*registerF)(struct _fileIO *io);

  if (!combo_picks_mem_type(io->mem_type_reachedW, &mem_type_reached))
    {
      printf("Error picking mem type\n");
      return;
    }

  str = gtk_entry_get_text(GTK_ENTRY(io->address_reachedW));
  if (!text_to_address(str, &address_reached))
    return;

  str_owned = COMBO_TEXT(io->reached_howW);
  if (str_owned == NULL) return;
  if (strcmp(str_owned, "is read") == 0)
    {
      reached_how = MEMORY_READ;
      registerF = set_fileIO_break_on_memory;
    }
  else if (strcmp(str_owned, "is written") == 0)
    {
      reached_how = MEMORY_WRITE;
      registerF = set_fileIO_break_on_memory;
    }
  else if (strcmp(str_owned, "is executed") == 0)
    {
      reached_how = PIPELINE_EXECUTED;
      registerF = set_fileIO_break_on_pipeline;
    }
  else
    {
      g_free(str_owned);
      return;
    }
  g_free(str_owned);

  str = gtk_entry_get_text(GTK_ENTRY(io->amountW));
  if (!text_to_int(str, &amount))
    return;

  str = gtk_entry_get_text(GTK_ENTRY(io->filenameW));
  filename = (gchar *)str;

  if (!combo_picks_mem_type(io->type_accessW, &type_access))
    return;

  str = gtk_entry_get_text(GTK_ENTRY(io->address_accessW));
  if (!text_to_address(str, &address_access))
    return;

  io->mem_type_reached = mem_type_reached;
  io->address_reached = address_reached;
  io->amount = amount;
  io->filename = g_strdup(filename);
  io->type_access = type_access;
  io->address_access = address_access;
  io->reached_how = reached_how;

  if (io->valid == 0)
    {
      GtkWidget *connect_box, *separator;

      io->registerF = registerF;
      io->registerF(io);

      fileIOL = g_list_append(fileIOL, io);
      io->valid = 1;

      separator = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
      gtk_box_pack_start(GTK_BOX(vbox_main), separator, FALSE, FALSE, 0);

      io_new = g_new0(struct _fileIO, 1);
      io_new->valid = 0;
      io_new->input = io->input;
      connect_box = create_io(io_new);
      gtk_box_pack_start(GTK_BOX(vbox_main), connect_box, FALSE, FALSE, 0);
      io_new->connect_box = connect_box;

      gtk_widget_set_sensitive(io->removeB, TRUE);
      gtk_widget_show_all(vbox_main);
    }
  else
    {
      if (io->registerF == registerF)
        {
          if (io->updateF) io->updateF(io);
        }
      else
        {
          if (io->removeF) io->removeF(io);
          io->registerF = registerF;
          io->registerF(io);
        }
    }

  fill_io(io);
}

static void removeCB(GtkWidget *W, struct _fileIO *io)
{
  gtk_widget_set_sensitive(io->removeB, FALSE);
  if (io->removeF) io->removeF(io);
  io->registerF = NULL;
  if (io->valid == 1)
    {
      gtk_widget_hide(io->connect_box);
      destroy_io(io);
    }
}

static void restartCB(GtkWidget *W, struct _fileIO *io)
{
  if (io->file) fclose(io->file);
  io->file = NULL;
  gtk_widget_set_sensitive(io->restartB, FALSE);
}

static void flushCB(GtkWidget *W, struct _fileIO *io)
{
  if (io->file) fflush(io->file);
}

static void fill_io(struct _fileIO *io)
{
  gchar tmp_str[100];

  if (io->mem_type_reached == DATA_MEM_TYPE)
    gtk_combo_box_set_active(GTK_COMBO_BOX(io->mem_type_reachedW), 0);
  else if (io->mem_type_reached == PROGRAM_MEM_TYPE)
    gtk_combo_box_set_active(GTK_COMBO_BOX(io->mem_type_reachedW), 1);

  if (io->modified & ADDRESS_REACHED_SET)
    {
      g_snprintf(tmp_str, 100, "0x%x", io->address_reached);
      gtk_entry_set_text(GTK_ENTRY(io->address_reachedW), tmp_str);
    }

  switch (io->reached_how)
    {
    case MEMORY_READ:
      gtk_combo_box_set_active(GTK_COMBO_BOX(io->reached_howW), 0);
      break;
    case MEMORY_WRITE:
      gtk_combo_box_set_active(GTK_COMBO_BOX(io->reached_howW), 1);
      break;
    case PIPELINE_EXECUTED:
      gtk_combo_box_set_active(GTK_COMBO_BOX(io->reached_howW), 2);
      break;
    }

  if (io->modified & AMOUNT_SET)
    {
      g_snprintf(tmp_str, 100, "%d", io->amount);
      gtk_entry_set_text(GTK_ENTRY(io->amountW), tmp_str);
    }

  if ((io->modified & FILENAME_SET) && io->filename)
    gtk_entry_set_text(GTK_ENTRY(io->filenameW), io->filename);

  if (io->type_access == DATA_MEM_TYPE)
    gtk_combo_box_set_active(GTK_COMBO_BOX(io->type_accessW), 0);
  else if (io->type_access == PROGRAM_MEM_TYPE)
    gtk_combo_box_set_active(GTK_COMBO_BOX(io->type_accessW), 1);

  if (io->modified & ADDRESS_ACCESS_SET)
    {
      g_snprintf(tmp_str, 100, "0x%x", io->address_access);
      gtk_entry_set_text(GTK_ENTRY(io->address_accessW), tmp_str);
    }
}

static GtkWidget *mem_type_combo(void)
{
  GtkWidget *c = gtk_combo_box_text_new();
  gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(c), "data");
  gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(c), "program");
  gtk_combo_box_set_active(GTK_COMBO_BOX(c), 0);
  return c;
}

static GtkWidget *reached_how_combo(void)
{
  GtkWidget *c = gtk_combo_box_text_new();
  gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(c), "is read");
  gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(c), "is written");
  gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(c), "is executed");
  gtk_combo_box_set_active(GTK_COMBO_BOX(c), 0);
  return c;
}

static GtkWidget *create_io(struct _fileIO *io)
{
  GtkWidget *vbox, *hbox;
  GtkWidget *label, *entry;

  vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);

  gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);

  label = gtk_label_new(" When ");
  gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);

  io->mem_type_reachedW = mem_type_combo();
  gtk_box_pack_start(GTK_BOX(hbox), io->mem_type_reachedW, FALSE, FALSE, 0);

  label = gtk_label_new(" address ");
  gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);

  entry = gtk_entry_new();
  gtk_entry_set_width_chars(GTK_ENTRY(entry), 13);
  gtk_entry_set_max_length(GTK_ENTRY(entry), 13);
  gtk_box_pack_start(GTK_BOX(hbox), entry, FALSE, FALSE, 0);
  io->address_reachedW = entry;
  g_signal_connect(entry, "changed", G_CALLBACK(address_reachedCB), io);

  io->reached_howW = reached_how_combo();
  gtk_box_pack_start(GTK_BOX(hbox), io->reached_howW, FALSE, FALSE, 0);

  hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
  gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);

  if (io->input)
    label = gtk_label_new(" read ");
  else
    label = gtk_label_new(" write ");
  gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);

  entry = gtk_entry_new();
  gtk_entry_set_width_chars(GTK_ENTRY(entry), 13);
  gtk_entry_set_max_length(GTK_ENTRY(entry), 13);
  gtk_box_pack_start(GTK_BOX(hbox), entry, FALSE, FALSE, 0);
  io->amountW = entry;
  g_signal_connect(entry, "changed", G_CALLBACK(amountCB), io);

  label = gtk_label_new(" entries of ");
  gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);

  io->type_accessW = mem_type_combo();
  gtk_box_pack_start(GTK_BOX(hbox), io->type_accessW, FALSE, FALSE, 0);

  label = gtk_label_new(" memory, address ");
  gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);

  entry = gtk_entry_new();
  gtk_entry_set_width_chars(GTK_ENTRY(entry), 13);
  gtk_entry_set_max_length(GTK_ENTRY(entry), 13);
  gtk_box_pack_start(GTK_BOX(hbox), entry, FALSE, FALSE, 0);
  io->address_accessW = entry;
  g_signal_connect(entry, "changed", G_CALLBACK(address_accessCB), io);

  if (io->input)
    label = gtk_label_new(" from file ");
  else
    label = gtk_label_new(" into file ");
  gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);

  entry = gtk_entry_new();
  gtk_entry_set_width_chars(GTK_ENTRY(entry), 13);
  gtk_entry_set_max_length(GTK_ENTRY(entry), 13);
  gtk_box_pack_start(GTK_BOX(hbox), entry, FALSE, FALSE, 0);
  io->filenameW = entry;
  g_signal_connect(entry, "changed", G_CALLBACK(filenameCB), io);

  hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
  gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);

  io->applyB = gtk_button_new_with_label("Apply");
  gtk_box_pack_start(GTK_BOX(hbox), io->applyB, FALSE, FALSE, 0);
  g_signal_connect(io->applyB, "clicked", G_CALLBACK(applyCB), io);
  gtk_widget_set_sensitive(io->applyB, FALSE);

  io->removeB = gtk_button_new_with_label("Remove");
  gtk_box_pack_start(GTK_BOX(hbox), io->removeB, FALSE, FALSE, 0);
  g_signal_connect(io->removeB, "clicked", G_CALLBACK(removeCB), io);

  if (io->valid == 0)
    {
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

      gtk_widget_set_sensitive(io->removeB, FALSE);
    }

  fill_io(io);

  io->restartB = gtk_button_new_with_label("Restart File");
  gtk_box_pack_start(GTK_BOX(hbox), io->restartB, FALSE, FALSE, 0);
  g_signal_connect(io->restartB, "clicked", G_CALLBACK(restartCB), io);
  gtk_widget_set_sensitive(io->restartB, io->file != NULL);

  if (io->input != 0)
    {
      io->flushB = gtk_button_new_with_label("Flush File");
      gtk_box_pack_start(GTK_BOX(hbox), io->flushB, FALSE, FALSE, 0);
      g_signal_connect(io->flushB, "clicked", G_CALLBACK(flushCB), io);
      gtk_widget_set_sensitive(io->flushB, io->file != NULL);
    }

  return vbox;
}

void create_fileIO(GtkWidget *widget, gpointer data)
{
  GtkWidget *scrolledW, *connect_box, *separator, *mainW;
  struct _fileIO *io;
  GList *list;

  if (GPOINTER_TO_INT(data) == 0)
    {
      if (fileOW != NULL) return;
      fileOW = gtk_window_new(GTK_WINDOW_TOPLEVEL);
      gtk_widget_set_name(fileOW, "Connect Data to an Output File");
      gtk_window_set_title(GTK_WINDOW(fileOW),
                           "Connect Data to an Output File");
      mainW = fileOW;
    }
  else
    {
      if (fileIW != NULL) return;
      fileIW = gtk_window_new(GTK_WINDOW_TOPLEVEL);
      gtk_widget_set_name(fileIW, "Connect Input File to Data");
      gtk_window_set_title(GTK_WINDOW(fileIW),
                           "Connect Input File to Data");
      mainW = fileIW;
    }

  gtk_widget_set_size_request(mainW, 660, 300);
  g_signal_connect(mainW, "destroy", G_CALLBACK(destroy_window_CB), data);
  gtk_container_set_border_width(GTK_CONTAINER(mainW), 0);

  scrolledW = gtk_scrolled_window_new(NULL, NULL);
  gtk_container_add(GTK_CONTAINER(mainW), scrolledW);

  vbox_main = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
  gtk_container_add(GTK_CONTAINER(scrolledW), vbox_main);

  list = fileIOL;
  while (list)
    {
      io = list->data;
      if (io->input == GPOINTER_TO_INT(data))
        {
          connect_box = create_io(io);
          gtk_box_pack_start(GTK_BOX(vbox_main), connect_box, FALSE, FALSE, 0);
          io->connect_box = connect_box;

          separator = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
          gtk_box_pack_start(GTK_BOX(vbox_main), separator, FALSE, FALSE, 0);
        }
      list = list->next;
    }

  io_new = g_new0(struct _fileIO, 1);
  io_new->valid = 0;
  io_new->input = GPOINTER_TO_INT(data);

  connect_box = create_io(io_new);
  gtk_box_pack_start(GTK_BOX(vbox_main), connect_box, FALSE, FALSE, 0);
  io_new->connect_box = connect_box;
  io_new->modified = 0;

  gtk_widget_show_all(mainW);
}

void fileIO_process(struct _fileIO *io)
{
  int k, available, amount_written, wait_state;
  Word wrd;

  if (io->input == 0)
    {
      if (io->file == NULL)
        {
          io->file = fopen(io->filename, "w");
          if (io->file == NULL)
            {
              printf("Error cannot create file!\n");
              return;
            }
          gtk_widget_set_sensitive(io->restartB, TRUE);
        }

      for (k = 0; k < io->amount; k++)
        {
          wrd = read_mem(io->address_access + k, &wait_state,
                         io->type_access, &available);
          if (available == 0)
            printf("Warning reading uninitialised memory\n");
          amount_written = fprintf(io->file, "0x%x\n", wrd);
          if (amount_written <= 0)
            {
              printf("Error cannot write to file!\n");
              return;
            }
        }
    }
  else
    {
      if (io->file == NULL)
        {
          io->file = fopen(io->filename, "r");
          if (io->file == NULL)
            {
              printf("Error cannot open file!\n");
              return;
            }
          gtk_widget_set_sensitive(io->restartB, TRUE);
          gtk_widget_set_sensitive(io->flushB, TRUE);
        }
      for (k = 0; k < io->amount; k++)
        {
          if (word_from_file(io->file, &wrd))
            wait_state = write_mem(io->address_access + k, wrd,
                                   io->type_access);
        }
    }
}
