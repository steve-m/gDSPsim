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

/*
 * Memory window. GTK3 port: list is a GtkTreeView backed by GtkListStore.
 * Columns: address, value, plus hidden fg/bg strings to colour changed rows.
 */

#include <stdio.h>
#include <string.h>
#include <decode_window.h>
#include <chip_core.h>
#include <find_opcode.h>
#include <memory.h>
#include <entryCB.h>
#include <memory_window.h>

enum {
  COL_ADDRESS = 0,
  COL_VALUE,
  COL_FG,
  COL_BG,
  N_COLS
};

/* Foreground colours: 0=written, 1=unavailable, 2=read; BG: SP row */
#define FG_WRITTEN      "#800000"
#define FG_UNAVAILABLE  "#9400d3"
#define FG_READ         "#008080"
#define BG_SP           "#eee8aa"

GList *all_mem_win_list = NULL;

struct _mem_window_nfo {
  GtkWidget *memoryW;
  int memory_type;
  GtkTreeView *view;
  GtkListStore *store;
  GtkWidget *label_data;
  GtkWidget *label_prog;
  WordP start;
  WordP end;
  struct _entryCB_nfo *start_nfo;
  struct _entryCB_nfo *end_nfo;
};

static Word SP_last;

#define MEM_CHANGED_BLK 5
struct _mem_changed {
  int valid;
  WordP changed[MEM_CHANGED_BLK];
  unsigned int accessed[MEM_CHANGED_BLK][2];
};

GList *head_mem_changed = NULL;

/* ------------------------------------------------------------------ */
/* Helpers to access rows by numeric index                            */
/* ------------------------------------------------------------------ */
static gboolean get_row_iter(struct _mem_window_nfo *mwn, int row,
                             GtkTreeIter *iter)
{
  return gtk_tree_model_iter_nth_child(GTK_TREE_MODEL(mwn->store),
                                       iter, NULL, row);
}

static void set_row_colour(struct _mem_window_nfo *mwn, int row,
                           const gchar *fg, const gchar *bg)
{
  GtkTreeIter iter;
  if (!get_row_iter(mwn, row, &iter))
    return;
  gtk_list_store_set(mwn->store, &iter, COL_FG, fg, COL_BG, bg, -1);
}

static void set_row_value(struct _mem_window_nfo *mwn, int row,
                          const gchar *v)
{
  GtkTreeIter iter;
  if (!get_row_iter(mwn, row, &iter))
    return;
  gtk_list_store_set(mwn->store, &iter, COL_VALUE, v, -1);
}

/* ------------------------------------------------------------------ */
/* Value edit popup                                                   */
/* ------------------------------------------------------------------ */
void entry_data_memCB(GtkWidget *widget, gpointer user_data)
{
  WordP offset = (WordP)(gsize)user_data;
  const gchar *entry_text = gtk_entry_get_text(GTK_ENTRY(widget));
  int value = 0;
  if (entry_text && entry_text[0] == '0' &&
      (entry_text[1] == 'x' || entry_text[1] == 'X'))
    sscanf(entry_text, "0x%x", &value);
  else
    sscanf(entry_text, "%d", &value);
  write_data_mem(offset, value);
}

void entry_prog_memCB(GtkWidget *widget, gpointer user_data)
{
  WordP offset = (WordP)(gsize)user_data;
  const gchar *entry_text = gtk_entry_get_text(GTK_ENTRY(widget));
  int value = 0;
  if (entry_text && entry_text[0] == '0' &&
      (entry_text[1] == 'x' || entry_text[1] == 'X'))
    sscanf(entry_text, "0x%x", &value);
  else
    sscanf(entry_text, "%d", &value);
  write_program_mem(offset, value);
}

static void row_activated_CB(GtkTreeView *tv, GtkTreePath *path,
                             GtkTreeViewColumn *col,
                             struct _mem_window_nfo *mwn)
{
  GtkWidget *popupW, *grid, *label, *entry;
  gchar temp_str[16];
  WordP address;
  gint *indices;

  indices = gtk_tree_path_get_indices(path);
  if (!indices)
    return;
  address = indices[0] + mwn->start;

  popupW = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title(GTK_WINDOW(popupW), "Edit memory");
  grid = gtk_grid_new();
  gtk_container_add(GTK_CONTAINER(popupW), grid);

  label = gtk_label_new("address");
  gtk_grid_attach(GTK_GRID(grid), label, 0, 0, 1, 1);

  label = gtk_label_new("value");
  gtk_grid_attach(GTK_GRID(grid), label, 1, 0, 1, 1);

  g_snprintf(temp_str, 16, "0x%x", address);
  label = gtk_label_new(temp_str);
  gtk_grid_attach(GTK_GRID(grid), label, 0, 1, 1, 1);

  entry = gtk_entry_new();
  gtk_entry_set_width_chars(GTK_ENTRY(entry), 13);
  gtk_entry_set_max_length(GTK_ENTRY(entry), 13);
  gtk_grid_attach(GTK_GRID(grid), entry, 1, 1, 1, 1);

  if (mwn->memory_type == DATA_MEM_TYPE)
    g_signal_connect(entry, "activate", G_CALLBACK(entry_data_memCB),
                     (gpointer)(gsize)address);
  else
    g_signal_connect(entry, "activate", G_CALLBACK(entry_prog_memCB),
                     (gpointer)(gsize)address);

  gtk_widget_show_all(popupW);
}

/* ------------------------------------------------------------------ */
static void destroy_window_CB(GtkWidget *W, gpointer data)
{
  struct _mem_window_nfo *nfo = data;
  all_mem_win_list = g_list_remove(all_mem_win_list, nfo);
  g_free(nfo->end_nfo);
  g_free(nfo->start_nfo);
  g_free(nfo);
}

static void mem_type_changeCB(GtkWidget *W, struct _mem_window_nfo *mwn)
{
  g_return_if_fail(mwn);
  g_return_if_fail(W);

  if (mwn->memory_type & DATA_MEM_TYPE)
    {
      gtk_container_remove(GTK_CONTAINER(W), mwn->label_data);
      mwn->memory_type = PROGRAM_MEM_TYPE;
      gtk_container_add(GTK_CONTAINER(W), mwn->label_prog);
      gtk_widget_show(mwn->label_prog);
    }
  else
    {
      gtk_container_remove(GTK_CONTAINER(W), mwn->label_prog);
      mwn->memory_type = DATA_MEM_TYPE;
      gtk_container_add(GTK_CONTAINER(W), mwn->label_data);
      gtk_widget_show(mwn->label_data);
    }
}

static void mem_view_append(WordP mem, MemType memtype,
                            struct _mem_window_nfo *mwn)
{
  int wait, available;
  GtkTreeIter iter;
  gchar address[16], value_str[16];
  const gchar *fg = NULL;

  g_snprintf(address, 16, "0x%x", mem);
  g_snprintf(value_str, 16, "0x%x",
             read_mem(mem, &wait, memtype, &available));

  if (!available)
    fg = FG_UNAVAILABLE;

  gtk_list_store_append(mwn->store, &iter);
  gtk_list_store_set(mwn->store, &iter,
                     COL_ADDRESS, address,
                     COL_VALUE, value_str,
                     COL_FG, fg,
                     COL_BG, NULL,
                     -1);
}

static void mem_view_prepend(WordP mem, MemType memtype,
                             struct _mem_window_nfo *mwn)
{
  int wait, available;
  GtkTreeIter iter;
  gchar address[16], value_str[16];
  const gchar *fg = NULL;

  g_snprintf(address, 16, "0x%x", mem);
  g_snprintf(value_str, 16, "0x%x",
             read_mem(mem, &wait, memtype, &available));

  if (!available)
    fg = FG_UNAVAILABLE;

  gtk_list_store_insert(mwn->store, &iter, 0);
  gtk_list_store_set(mwn->store, &iter,
                     COL_ADDRESS, address,
                     COL_VALUE, value_str,
                     COL_FG, fg,
                     COL_BG, NULL,
                     -1);
}

static void remove_first_rows(struct _mem_window_nfo *mwn, int count)
{
  GtkTreeIter iter;
  int k;
  for (k=0;k<count;k++)
    {
      if (!gtk_tree_model_get_iter_first(GTK_TREE_MODEL(mwn->store), &iter))
        return;
      gtk_list_store_remove(mwn->store, &iter);
    }
}

static void remove_row_at(struct _mem_window_nfo *mwn, int row)
{
  GtkTreeIter iter;
  if (get_row_iter(mwn, row, &iter))
    gtk_list_store_remove(mwn->store, &iter);
}

static void clear_store(struct _mem_window_nfo *mwn)
{
  gtk_list_store_clear(mwn->store);
}

static void update_memory_view(struct _mem_window_nfo *mwn,
                               WordP start_mem, WordP end_mem)
{
  WordP mem;
  gchar temp_str[16];

  if (start_mem > end_mem)
    {
      if (start_mem == mwn->start)
        {
          start_mem = end_mem;
          g_snprintf(temp_str, 16, "0x%x", start_mem);
          gtk_entry_set_text(GTK_ENTRY(mwn->start_nfo->entry), temp_str);
        }
      else
        {
          end_mem = start_mem;
          g_snprintf(temp_str, 16, "0x%x", end_mem);
          gtk_entry_set_text(GTK_ENTRY(mwn->end_nfo->entry), temp_str);
        }
    }
  else if ((end_mem - start_mem) > 0x200)
    {
      if (start_mem == mwn->start)
        {
          start_mem = end_mem - 0x200;
          g_snprintf(temp_str, 16, "0x%x", start_mem);
          gtk_entry_set_text(GTK_ENTRY(mwn->start_nfo->entry), temp_str);
        }
      else
        {
          end_mem = start_mem + 0x200;
          g_snprintf(temp_str, 16, "0x%x", end_mem);
          gtk_entry_set_text(GTK_ENTRY(mwn->end_nfo->entry), temp_str);
        }
    }

  if (start_mem < mwn->start)
    {
      if (end_mem < mwn->start)
        {
          clear_store(mwn);
          mwn->start = start_mem;
          mwn->end = end_mem;
          for (mem=mwn->start;mem<=mwn->end;mem++)
            mem_view_append(mem, mwn->memory_type, mwn);
          update_all_memory_windows(1);
          return;
        }

      for (mem = mwn->start - 1; mem >= start_mem && mem < mwn->start; mem--)
        mem_view_prepend(mem, mwn->memory_type, mwn);
    }
  else if (start_mem > mwn->end)
    {
      clear_store(mwn);
      mem_view_append(start_mem, mwn->memory_type, mwn);
    }
  else if (start_mem > mwn->start)
    {
      remove_first_rows(mwn, start_mem - mwn->start);
    }

  mwn->start = start_mem;

  if (end_mem > mwn->end)
    {
      if (start_mem <= mwn->end)
        start_mem = mwn->end;
      for (mem = start_mem + 1; mem <= end_mem; mem++)
        mem_view_append(mem, mwn->memory_type, mwn);
    }
  else if (end_mem < mwn->end)
    {
      int row = end_mem + 1 - start_mem;
      for (mem = end_mem; mem < mwn->end; mem++)
        remove_row_at(mwn, row);
    }
  mwn->end = end_mem;
}

static void change_start_address(GtkWidget *entry, guint64 address, gpointer data)
{
  struct _mem_window_nfo *mwn = data;
  update_memory_view(mwn, (WordP)address, mwn->end);
}

static void change_end_address(GtkWidget *entry, guint64 address, gpointer data)
{
  struct _mem_window_nfo *mwn = data;
  update_memory_view(mwn, mwn->start, (WordP)address);
}

/* ------------------------------------------------------------------ */
void create_memory_window(void)
{
  GtkWidget *vbox, *hbox, *scrolledW;
  GtkWidget *entryTop, *entryBottom, *memtype_button;
  WordP mem;
  gchar temp_str[16];
  struct _mem_window_nfo *mem_window;
  struct _entryCB_nfo *entry_start_nfo, *entry_end_nfo;
  GtkCellRenderer *renderer;
  GtkTreeViewColumn *column;
  extern GtkAccelGroup *gDSP_keyboard_accel;

  mem_window = g_new0(struct _mem_window_nfo, 1);
  all_mem_win_list = g_list_prepend(all_mem_win_list, mem_window);

  mem_window->memoryW = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  g_signal_connect(mem_window->memoryW, "destroy",
                   G_CALLBACK(destroy_window_CB), mem_window);
  gtk_window_add_accel_group(GTK_WINDOW(mem_window->memoryW),
                             gDSP_keyboard_accel);
  gtk_widget_set_name(mem_window->memoryW, "Memory Window");
  gtk_widget_set_size_request(mem_window->memoryW, 200, 350);
  gtk_window_set_title(GTK_WINDOW(mem_window->memoryW), "Memory Window");
  gtk_container_set_border_width(GTK_CONTAINER(mem_window->memoryW), 0);

  vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  gtk_container_add(GTK_CONTAINER(mem_window->memoryW), vbox);

  memtype_button = gtk_button_new();
  mem_window->label_data = gtk_label_new("Data");
  g_object_ref(mem_window->label_data);
  mem_window->label_prog = gtk_label_new("Prog");
  g_object_ref(mem_window->label_prog);
  gtk_container_add(GTK_CONTAINER(memtype_button), mem_window->label_data);
  gtk_box_pack_start(GTK_BOX(vbox), memtype_button, FALSE, FALSE, 0);
  g_signal_connect(memtype_button, "clicked",
                   G_CALLBACK(mem_type_changeCB), mem_window);

  hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
  gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);

  get_prog_mem_start_end(&mem_window->start, &mem_window->end);

  entry_start_nfo = g_new0(struct _entryCB_nfo, 1);
  mem_window->start_nfo = entry_start_nfo;

  entryTop = gtk_entry_new();
  entry_start_nfo->entry = entryTop;
  entry_start_nfo->bits = BITS_PER_ADDRESS;
  entry_start_nfo->CB_func = change_start_address;
  entry_start_nfo->data = mem_window;
  gtk_box_pack_start(GTK_BOX(hbox), entryTop, TRUE, TRUE, 0);
  gtk_entry_set_max_length(GTK_ENTRY(entryTop), 16);
  gtk_entry_set_width_chars(GTK_ENTRY(entryTop), 7);
  g_snprintf(temp_str, 16, "0x%x", mem_window->start);
  gtk_entry_set_text(GTK_ENTRY(entryTop), temp_str);
  g_signal_connect(entryTop, "activate", G_CALLBACK(entry_hexCB),
                   entry_start_nfo);

  entry_end_nfo = g_new0(struct _entryCB_nfo, 1);
  mem_window->end_nfo = entry_end_nfo;

  entryBottom = gtk_entry_new();
  entry_end_nfo->entry = entryBottom;
  entry_end_nfo->bits = BITS_PER_ADDRESS;
  entry_end_nfo->CB_func = change_end_address;
  entry_end_nfo->data = mem_window;
  gtk_box_pack_start(GTK_BOX(hbox), entryBottom, TRUE, TRUE, 0);
  gtk_entry_set_max_length(GTK_ENTRY(entryBottom), 16);
  gtk_entry_set_width_chars(GTK_ENTRY(entryBottom), 7);
  g_snprintf(temp_str, 16, "0x%x", mem_window->end);
  gtk_entry_set_text(GTK_ENTRY(entryBottom), temp_str);
  g_signal_connect(entryBottom, "activate", G_CALLBACK(entry_hexCB),
                   entry_end_nfo);

  /* Tree view */
  scrolledW = gtk_scrolled_window_new(NULL, NULL);
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolledW),
                                 GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  gtk_box_pack_start(GTK_BOX(vbox), scrolledW, TRUE, TRUE, 0);

  mem_window->store = gtk_list_store_new(N_COLS,
                                         G_TYPE_STRING, G_TYPE_STRING,
                                         G_TYPE_STRING, G_TYPE_STRING);
  mem_window->view = GTK_TREE_VIEW(
      gtk_tree_view_new_with_model(GTK_TREE_MODEL(mem_window->store)));
  g_object_unref(mem_window->store);

  renderer = gtk_cell_renderer_text_new();
  column = gtk_tree_view_column_new_with_attributes("address", renderer,
                                                    "text", COL_ADDRESS,
                                                    "foreground", COL_FG,
                                                    "background", COL_BG,
                                                    NULL);
  gtk_tree_view_append_column(mem_window->view, column);

  renderer = gtk_cell_renderer_text_new();
  column = gtk_tree_view_column_new_with_attributes("value", renderer,
                                                    "text", COL_VALUE,
                                                    "foreground", COL_FG,
                                                    "background", COL_BG,
                                                    NULL);
  gtk_tree_view_append_column(mem_window->view, column);

  mem_window->memory_type = DATA_MEM_TYPE;

  g_signal_connect(mem_window->view, "row-activated",
                   G_CALLBACK(row_activated_CB), mem_window);

  gtk_container_add(GTK_CONTAINER(scrolledW), GTK_WIDGET(mem_window->view));

  for (mem = mem_window->start; mem <= mem_window->end; mem++)
    mem_view_append(mem, mem_window->memory_type, mem_window);

  gtk_widget_show_all(mem_window->memoryW);
  gtk_widget_hide(mem_window->label_prog);
}

/* ------------------------------------------------------------------ */
static void initialize_changed_mem(void)
{
  if (head_mem_changed == NULL)
    {
      struct _mem_changed *mc = g_new0(struct _mem_changed, 1);
      head_mem_changed = g_list_prepend(head_mem_changed, mc);
    }
}

int check_if_changed_set(WordP address, int type)
{
  GList *list;
  struct _mem_changed *mc;
  int k;

  list = head_mem_changed;
  while (list != NULL)
    {
      mc = list->data;
      for (k = 0; k < mc->valid; k++)
        {
          if (address == mc->changed[k])
            {
              mc->accessed[k][type]++;
              return 1;
            }
        }
      list = list->next;
    }
  return 0;
}

void set_mem_changed(WordP address, int type)
{
  GList *list;
  struct _mem_changed *mc;

  if (head_mem_changed == NULL)
    initialize_changed_mem();

  if (check_if_changed_set(address, type))
    return;

  list = head_mem_changed;
  while (list->next != NULL)
    list = list->next;

  mc = list->data;

  if (mc->valid == MEM_CHANGED_BLK)
    {
      mc = g_new0(struct _mem_changed, 1);
      head_mem_changed = g_list_append(head_mem_changed, mc);
    }

  mc->changed[mc->valid] = address;
  mc->accessed[mc->valid][type] = 1;
  mc->accessed[mc->valid][type ^ 1] = 0;
  mc->valid++;
}

void clear_mem_changed(void)
{
  struct _mem_changed *mc;
  GList *list;

  if (head_mem_changed == NULL)
    initialize_changed_mem();

  mc = head_mem_changed->data;
  mc->valid = 0;

  list = head_mem_changed->next;
  while (list)
    {
      mc = list->data;
      g_free(mc);
      head_mem_changed = g_list_remove_link(head_mem_changed, list);
      g_list_free(list);
      list = head_mem_changed->next;
    }
}

void update_all_memory_windows(int highlight)
{
  GList *list, *list2;
  struct _mem_window_nfo *mw;
  gchar text_now[16];
  int wait, k, row, available;
  struct _mem_changed *mc;

  SP_last = MMR->SP;

  for (list = all_mem_win_list; list; list = list->next)
    {
      mw = list->data;

      if (highlight)
        {
          if ((mw->start <= MMR->SP) && (mw->end >= MMR->SP))
            {
              row = MMR->SP - mw->start;
              set_row_colour(mw, row, NULL, BG_SP);
            }
          SP_last = MMR->SP;
        }
      else
        {
          if ((mw->start <= SP_last) && (mw->end >= SP_last))
            {
              row = SP_last - mw->start;
              set_row_colour(mw, row, NULL, NULL);
            }
        }

      for (list2 = head_mem_changed; list2; list2 = list2->next)
        {
          mc = list2->data;
          for (k = 0; k < mc->valid; k++)
            {
              if ((mw->start <= mc->changed[k]) &&
                  (mw->end >= mc->changed[k]))
                {
                  row = mc->changed[k] - mw->start;
                  if (highlight)
                    {
                      const gchar *fg =
                          (mc->accessed[k][0] >= mc->accessed[k][1])
                          ? FG_WRITTEN : FG_READ;
                      set_row_colour(mw, row, fg, NULL);
                      g_snprintf(text_now, 16, "0x%x",
                                 read_mem(mc->changed[k], &wait,
                                          mw->memory_type, &available));
                      set_row_value(mw, row, text_now);
                    }
                  else
                    {
                      set_row_colour(mw, row, NULL, NULL);
                    }
                }
            }
        }
    }
  if (highlight == 0)
    clear_mem_changed();
}
