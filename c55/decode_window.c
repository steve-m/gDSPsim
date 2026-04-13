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

/*
 * C55x disassembly window. Behaviourally identical to the shared
 * version under ../gdspsim/decode_window.c aside from a larger default
 * window size. Only one highlighted row at a time (the PC).
 */

#include <stdio.h>
#include <decode_window.h>
#include <find_opcode.h>
#include <memory.h>
#include <entryCB.h>
#include <pipeline.h>

enum {
  COL_BREAK = 0,
  COL_ADDRESS,
  COL_MACHINE,
  COL_OPCODE,
  COL_ADDR_PTR,
  COL_BG,
  N_COLS
};

int decode_follow_pref = 1;

struct _decode_window_nfo {
  GtkWidget *view;
  GtkListStore *store;
  GtkWidget *decodeW;
  WordP start;
  WordP end;
  GArray *word2line;
};

struct _decode_window_nfo *dwn = NULL;

#define PIPE_BG "#800000"

static void insert_text(WordP start_mem, WordP end_mem, GArray *word2line);

static void change_end_address(GtkWidget *entry, guint64 address, gpointer data)
{
  struct _decode_window_nfo *d = data;
  WordP mem = address;

  unhighlight_pipeline();
  if (mem < d->start) d->start = mem;
  if (mem > (d->start + 0x800)) d->start = mem - 0x800;
  d->end = address;

  while (d->word2line->len > 0)
    g_array_remove_index_fast(d->word2line, 0);

  insert_text(d->start, d->end, d->word2line);
  highlight_pipeline(0x0);
}

static void change_start_address(GtkWidget *entry, guint64 address, gpointer data)
{
  struct _decode_window_nfo *d = data;
  WordP mem = address;

  unhighlight_pipeline();
  if (mem > d->end) d->end = mem;
  if (mem < (d->end - 0x800)) d->end = mem + 0x800;
  d->start = address;

  while (d->word2line->len > 0)
    g_array_remove_index_fast(d->word2line, 0);

  insert_text(d->start, d->end, d->word2line);
  highlight_pipeline(0x0);
}

static void row_activated_CB(GtkTreeView *tv, GtkTreePath *path,
                             GtkTreeViewColumn *col, gpointer data)
{
  GtkTreeModel *model = gtk_tree_view_get_model(tv);
  GtkTreeIter iter;
  gpointer addrp = NULL;

  if (!gtk_tree_model_get_iter(model, &iter, path))
    return;
  gtk_tree_model_get(model, &iter, COL_ADDR_PTR, &addrp, -1);
  if (addrp)
    {
      WordP addr = (WordP)(gsize)addrp;
      gboolean set = toggle_breakpoint(addr) ? TRUE : FALSE;
      gtk_list_store_set(GTK_LIST_STORE(model), &iter,
                         COL_BREAK, set ? "*" : "", -1);
    }
}

static void radio_CB(GtkCheckMenuItem *item, gpointer data)
{
  if (gtk_check_menu_item_get_active(item))
    decode_follow_pref = GPOINTER_TO_INT(data);
}

static int pipe_row = -2;

static void destroy_decode_window_CB(GtkWidget *w, gpointer data)
{
  if (dwn == NULL)
    return;
  /*
   * The GtkListStore is owned by the tree view and was already
   * finalized when gtk tore down the window; dwn->store is now a
   * dangling pointer, so we must not touch it here — just drop our
   * own bookkeeping and NULL dwn before the pipeline fires again.
   */
  if (dwn->word2line)
    g_array_free(dwn->word2line, TRUE);
  g_free(dwn);
  dwn = NULL;
  pipe_row = -2;
}

void create_decode_window(void)
{
  GtkWidget *vbox, *hbox, *scrolledW;
  GtkWidget *entryTop, *entryBottom;
  WordP start_mem, end_mem;
  gchar temp_str[16];
  struct _entryCB_nfo *entry_start_nfo, *entry_end_nfo;
  GtkWidget *menubar, *menu, *item;
  GSList *group = NULL;
  GtkCellRenderer *renderer;
  GtkTreeViewColumn *column;
  extern GtkAccelGroup *gDSP_keyboard_accel;

  if (dwn)
    return;

  dwn = g_new0(struct _decode_window_nfo, 1);

  dwn->decodeW = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_widget_set_name(dwn->decodeW, "Disassembly");
  gtk_widget_set_size_request(dwn->decodeW, 600, 450);
  gtk_window_set_title(GTK_WINDOW(dwn->decodeW), "Disassembly");
  gtk_container_set_border_width(GTK_CONTAINER(dwn->decodeW), 0);
  gtk_window_add_accel_group(GTK_WINDOW(dwn->decodeW), gDSP_keyboard_accel);
  g_signal_connect(dwn->decodeW, "destroy",
                   G_CALLBACK(destroy_decode_window_CB), NULL);

  vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  gtk_container_add(GTK_CONTAINER(dwn->decodeW), vbox);

  menubar = gtk_menu_bar_new();
  menu = gtk_menu_new();
  item = gtk_menu_item_new_with_mnemonic("_Preferences");
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(item), menu);
  gtk_menu_shell_append(GTK_MENU_SHELL(menubar), item);

  item = gtk_radio_menu_item_new_with_label(group, "Don't Follow");
  group = gtk_radio_menu_item_get_group(GTK_RADIO_MENU_ITEM(item));
  gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);
  g_signal_connect(item, "toggled", G_CALLBACK(radio_CB), GINT_TO_POINTER(0));

  item = gtk_radio_menu_item_new_with_label(group, "Follow Execute");
  group = gtk_radio_menu_item_get_group(GTK_RADIO_MENU_ITEM(item));
  gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(item), TRUE);
  gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);
  g_signal_connect(item, "toggled", G_CALLBACK(radio_CB), GINT_TO_POINTER(1));

  item = gtk_radio_menu_item_new_with_label(group, "Follow Decode");
  group = gtk_radio_menu_item_get_group(GTK_RADIO_MENU_ITEM(item));
  gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);
  g_signal_connect(item, "toggled", G_CALLBACK(radio_CB), GINT_TO_POINTER(2));

  item = gtk_radio_menu_item_new_with_label(group, "Follow PC");
  gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);
  g_signal_connect(item, "toggled", G_CALLBACK(radio_CB), GINT_TO_POINTER(3));

  gtk_box_pack_start(GTK_BOX(vbox), menubar, FALSE, TRUE, 0);

  hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
  gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 6);

  get_prog_mem_start_end(&start_mem, &end_mem);

  entryTop = gtk_entry_new();
  gtk_box_pack_start(GTK_BOX(hbox), entryTop, TRUE, TRUE, 0);
  gtk_entry_set_max_length(GTK_ENTRY(entryTop), 10);
  g_snprintf(temp_str, 10, "0x%x", start_mem);
  gtk_entry_set_text(GTK_ENTRY(entryTop), temp_str);

  entry_start_nfo = g_new0(struct _entryCB_nfo, 1);
  entry_start_nfo->entry = entryTop;
  entry_start_nfo->bits = BITS_PER_ADDRESS;
  entry_start_nfo->CB_func = change_start_address;
  entry_start_nfo->data = dwn;
  entry_start_nfo->text = g_strdup(temp_str);
  g_signal_connect(entryTop, "activate", G_CALLBACK(entry_addressCB),
                   entry_start_nfo);

  entryBottom = gtk_entry_new();
  gtk_box_pack_start(GTK_BOX(hbox), entryBottom, TRUE, TRUE, 0);
  gtk_entry_set_max_length(GTK_ENTRY(entryBottom), 10);
  g_snprintf(temp_str, 10, "0x%x", end_mem);
  gtk_entry_set_text(GTK_ENTRY(entryBottom), temp_str);

  entry_end_nfo = g_new0(struct _entryCB_nfo, 1);
  entry_end_nfo->entry = entryBottom;
  entry_end_nfo->bits = BITS_PER_ADDRESS;
  entry_end_nfo->CB_func = change_end_address;
  entry_end_nfo->data = dwn;
  entry_end_nfo->text = g_strdup(temp_str);
  g_signal_connect(entryBottom, "activate", G_CALLBACK(entry_addressCB),
                   entry_end_nfo);

  scrolledW = gtk_scrolled_window_new(NULL, NULL);
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolledW),
                                 GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  gtk_box_pack_start(GTK_BOX(vbox), scrolledW, TRUE, TRUE, 0);

  dwn->store = gtk_list_store_new(N_COLS,
                                  G_TYPE_STRING, G_TYPE_STRING,
                                  G_TYPE_STRING, G_TYPE_STRING,
                                  G_TYPE_POINTER, G_TYPE_STRING);

  dwn->view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(dwn->store));
  gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(dwn->view), TRUE);
  g_object_unref(dwn->store);

  renderer = gtk_cell_renderer_text_new();
  column = gtk_tree_view_column_new_with_attributes("break", renderer,
                                                    "text", COL_BREAK,
                                                    "background", COL_BG,
                                                    NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW(dwn->view), column);

  renderer = gtk_cell_renderer_text_new();
  column = gtk_tree_view_column_new_with_attributes("address", renderer,
                                                    "text", COL_ADDRESS,
                                                    "background", COL_BG,
                                                    NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW(dwn->view), column);

  renderer = gtk_cell_renderer_text_new();
  column = gtk_tree_view_column_new_with_attributes("mach code", renderer,
                                                    "text", COL_MACHINE,
                                                    "background", COL_BG,
                                                    NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW(dwn->view), column);

  renderer = gtk_cell_renderer_text_new();
  column = gtk_tree_view_column_new_with_attributes("opcode", renderer,
                                                    "text", COL_OPCODE,
                                                    "background", COL_BG,
                                                    NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW(dwn->view), column);

  g_signal_connect(dwn->view, "row-activated",
                   G_CALLBACK(row_activated_CB), NULL);

  gtk_container_add(GTK_CONTAINER(scrolledW), dwn->view);

  dwn->start = start_mem;
  dwn->end = end_mem;
  dwn->word2line = g_array_new(FALSE, FALSE, sizeof(int));

  insert_text(start_mem, end_mem, dwn->word2line);

  gtk_widget_show_all(dwn->decodeW);
}

void update_pipeline(WordP prefetch)
{
}

static void set_row_bg(int row, const gchar *bg)
{
  GtkTreeIter iter;
  if (!dwn) return;
  if (row < 0) return;
  if (!gtk_tree_model_iter_nth_child(GTK_TREE_MODEL(dwn->store), &iter,
                                     NULL, row))
    return;
  gtk_list_store_set(dwn->store, &iter, COL_BG, bg, -1);
}

void unhighlight_pipeline(void)
{
  if (!dwn) return;
  if (pipe_row > 0)
    {
      set_row_bg(pipe_row, NULL);
      pipe_row = -1;
    }
}

void highlight_pipeline(WordP follow)
{
  if (dwn == NULL) return;

  if (follow)
    {
      if ((dwn->start <= follow) && (follow <= dwn->end))
        {
          int row = g_array_index(dwn->word2line, int, follow - dwn->start);
          GtkTreePath *path = gtk_tree_path_new_from_indices(row, -1);
          gtk_tree_view_scroll_to_cell(GTK_TREE_VIEW(dwn->view), path,
                                       NULL, TRUE, 0.5, 0.5);
          gtk_tree_path_free(path);
          set_row_bg(row, PIPE_BG);
          pipe_row = row;
        }
    }
}

void set_decode_index_max_range(unsigned long int lo, unsigned long int hi)
{
}

static void insert_text(WordP start_mem, WordP end_mem, GArray *word2line)
{
  GPtrArray *textA;
  int k, offset = 0;
  struct _decode_opcode *op;
  GtkTreeIter iter;

  gtk_list_store_clear(dwn->store);

  textA = g_ptr_array_new();
  decoded_opcodes(textA, start_mem, end_mem, word2line);

  for (k = 0; k < textA->len; k++)
    {
      while (g_array_index(dwn->word2line, int, offset) < k)
        offset++;

      op = g_ptr_array_index(textA, k);
      gtk_list_store_append(dwn->store, &iter);

      if (op->machine_code)
        {
          gtk_list_store_set(dwn->store, &iter,
                             COL_BREAK, "",
                             COL_ADDRESS, op->address,
                             COL_MACHINE, op->machine_code,
                             COL_OPCODE, op->opcode_text,
                             COL_ADDR_PTR,
                             (gpointer)(gsize)(offset + dwn->start),
                             COL_BG, NULL,
                             -1);
          g_free(op->machine_code);
        }
      else
        {
          gtk_list_store_set(dwn->store, &iter,
                             COL_BREAK, "",
                             COL_ADDRESS, op->opcode_text,
                             COL_MACHINE, "",
                             COL_OPCODE, "",
                             COL_ADDR_PTR, NULL,
                             COL_BG, NULL,
                             -1);
        }
      g_free(op->opcode_text);
      g_free(op->address);
      g_free(op);
    }

  g_ptr_array_free(textA, FALSE);
}

WordP line2word(GArray *word2line, int line, WordP start)
{
  WordP guess, guess_hi, guess_low;
  int line_guess;

  guess_hi = word2line->len - 1;
  guess_low = 0;
  guess = guess_hi >> 1;

  while (1)
    {
      line_guess = g_array_index(word2line, int, guess);

      if (line_guess == line || guess == guess_hi)
        {
          while (guess > 0)
            {
              guess--;
              line_guess = g_array_index(word2line, int, guess);
              if (line_guess < line)
                {
                  guess++;
                  return guess + start;
                }
            }
          return start;
        }
      else if (line_guess > line)
        {
          guess_hi = guess;
          guess = guess_low + ((guess_hi - guess_low + 1) >> 1);
        }
      else
        {
          guess_low = guess;
          guess = guess_low + ((guess_hi - guess_low + 1) >> 1);
        }
    }
}
