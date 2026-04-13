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
 * GTK3 port: the original plot_window relied on gtkextra's GtkPlot/
 * GtkPlotCanvas, neither of which has a GTK 3 port. We draw the memory
 * samples with cairo on a GtkDrawingArea instead.
 */

#include <gtk/gtk.h>
#include <entryCB.h>
#include <memory.h>

struct _plot_window_nfo {
  GtkWidget *window;
  GtkWidget *drawing;
  WordP address;
  Word length;
  struct _entryCB_nfo addressN;
  struct _entryCB_nfo lengthN;
  gdouble *ydata;
  int type;
};

GList *all_plot_windowL = NULL;

static void collect_data(struct _plot_window_nfo *pnfo)
{
  Word wrd, wrd2;
  guint32 wrd32;
  int k, wait_state;

  g_free(pnfo->ydata);
  pnfo->ydata = g_new0(gdouble, pnfo->length > 0 ? pnfo->length : 1);

  if (pnfo->type < 3)
    {
      for (k = 0; k < pnfo->length; k++)
        {
          wrd = read_data_mem_long(pnfo->address + k, &wait_state);
          switch (pnfo->type)
            {
            case 0: pnfo->ydata[k] = (gdouble)((gint16)wrd); break;
            case 1: pnfo->ydata[k] = (gdouble)wrd; break;
            case 2: pnfo->ydata[k] = (gdouble)((gint16)wrd) / 32768.0; break;
            }
        }
    }
  else
    {
      for (k = 0; k < pnfo->length; k++)
        {
          wrd = read_data_mem_long(pnfo->address + 2*k, &wait_state);
          wrd2 = read_data_mem_long(pnfo->address + 2*k + 1, &wait_state);
          wrd32 = (guint32)wrd2 | ((guint32)wrd) << 16;
          switch (pnfo->type)
            {
            case 3: pnfo->ydata[k] = (gdouble)((gint32)wrd32); break;
            case 4: pnfo->ydata[k] = (gdouble)wrd32; break;
            case 5: pnfo->ydata[k] = (gdouble)((gint32)wrd32) / 2147483648.0; break;
            }
        }
    }
}

static gboolean draw_CB(GtkWidget *widget, cairo_t *cr, gpointer data)
{
  struct _plot_window_nfo *pnfo = data;
  GtkAllocation alloc;
  double ymin, ymax, yspan, xstep;
  int k;

  gtk_widget_get_allocation(widget, &alloc);

  cairo_set_source_rgb(cr, 1, 1, 1);
  cairo_paint(cr);

  if (pnfo->length == 0 || pnfo->ydata == NULL)
    return FALSE;

  ymin = ymax = pnfo->ydata[0];
  for (k = 1; k < pnfo->length; k++)
    {
      if (pnfo->ydata[k] < ymin) ymin = pnfo->ydata[k];
      if (pnfo->ydata[k] > ymax) ymax = pnfo->ydata[k];
    }
  yspan = ymax - ymin;
  if (yspan == 0) yspan = 1;

  xstep = (double)alloc.width / (double)pnfo->length;

  cairo_set_source_rgb(cr, 0.8, 0.8, 0.8);
  cairo_move_to(cr, 0, alloc.height / 2);
  cairo_line_to(cr, alloc.width, alloc.height / 2);
  cairo_stroke(cr);

  cairo_set_source_rgb(cr, 0.8, 0, 0);
  cairo_set_line_width(cr, 1);
  for (k = 0; k < pnfo->length; k++)
    {
      double x = k * xstep;
      double y = alloc.height - ((pnfo->ydata[k] - ymin) / yspan)
                                * (alloc.height - 4) - 2;
      if (k == 0)
        cairo_move_to(cr, x, y);
      else
        cairo_line_to(cr, x, y);
    }
  cairo_stroke(cr);
  return FALSE;
}

static void redraw(struct _plot_window_nfo *pnfo)
{
  collect_data(pnfo);
  if (pnfo->drawing)
    gtk_widget_queue_draw(pnfo->drawing);
}

static void start_addressCB(GtkWidget *entry, guint64 num, gpointer data)
{
  struct _plot_window_nfo *pwn = data;
  pwn->address = num;
  redraw(pwn);
}

static void lengthCB(GtkWidget *entry, guint64 num, gpointer data)
{
  struct _plot_window_nfo *pwn = data;
  pwn->length = num;
  redraw(pwn);
}

static void updateCB(GtkWidget *entry, gpointer data)
{
  redraw(data);
}

static void destroy_window_CB(GtkWidget *W, gpointer data)
{
  struct _plot_window_nfo *nfo = data;
  all_plot_windowL = g_list_remove(all_plot_windowL, nfo);
  g_free(nfo->addressN.text);
  g_free(nfo->lengthN.text);
  g_free(nfo->ydata);
  g_free(nfo);
}

static void setType_CB(GtkWidget *W, gpointer data)
{
  struct _plot_window_nfo *pnfo = g_object_get_data(G_OBJECT(W), "pnfo");
  if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(W)))
    {
      pnfo->type = GPOINTER_TO_INT(data);
      redraw(pnfo);
    }
}

static void set_menu(GtkBox *boxW, struct _plot_window_nfo *pnfo)
{
  GtkWidget *menu, *menu_bar, *item;
  GSList *group = NULL;
  const gchar *labels[6] = {
    "16-bit Int", "16-bit Unsigned Int", "Q15",
    "32-bit Int", "32-bit Unsigned Int", "Q31"
  };
  int map[6] = {0, 1, 2, 3, 4, 5};
  int k;

  menu = gtk_menu_new();
  for (k = 0; k < 6; k++)
    {
      item = gtk_radio_menu_item_new_with_label(group, labels[k]);
      group = gtk_radio_menu_item_get_group(GTK_RADIO_MENU_ITEM(item));
      if (k == 0)
        gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(item), TRUE);
      g_object_set_data(G_OBJECT(item), "pnfo", pnfo);
      g_signal_connect(item, "toggled",
                       G_CALLBACK(setType_CB), GINT_TO_POINTER(map[k]));
      gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);
    }

  menu_bar = gtk_menu_bar_new();
  gtk_box_pack_start(boxW, menu_bar, FALSE, FALSE, 2);
  item = gtk_menu_item_new_with_label("Type");
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(item), menu);
  gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), item);
}

void create_plot_window(void)
{
  GtkWidget *buttonW, *hbox, *vbox, *lab, *entryW;
  struct _plot_window_nfo *p;

  p = g_new0(struct _plot_window_nfo, 1);
  p->type = 0;
  p->address = 0x80;
  p->length = 0x80;

  all_plot_windowL = g_list_prepend(all_plot_windowL, p);

  p->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  g_signal_connect(p->window, "destroy",
                   G_CALLBACK(destroy_window_CB), p);
  gtk_window_set_title(GTK_WINDOW(p->window), "Plot");
  gtk_widget_set_size_request(p->window, 550, 350);
  gtk_container_set_border_width(GTK_CONTAINER(p->window), 0);

  vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  gtk_container_add(GTK_CONTAINER(p->window), vbox);

  hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
  gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 2);

  set_menu(GTK_BOX(hbox), p);

  lab = gtk_label_new("Start");
  gtk_box_pack_start(GTK_BOX(hbox), lab, FALSE, FALSE, 2);

  entryW = gtk_entry_new();
  gtk_box_pack_start(GTK_BOX(hbox), entryW, FALSE, FALSE, 2);
  gtk_entry_set_max_length(GTK_ENTRY(entryW), 16);
  p->addressN.entry = entryW;
  p->addressN.bits = BITS_PER_PROGRAM_ACCESS;
  p->addressN.CB_func = start_addressCB;
  p->addressN.data = p;
  p->addressN.text = g_strdup_printf("0x%x", p->address);
  gtk_entry_set_text(GTK_ENTRY(entryW), p->addressN.text);
  g_signal_connect(entryW, "activate",
                   G_CALLBACK(entry_addressCB), &p->addressN);

  lab = gtk_label_new("Length");
  gtk_box_pack_start(GTK_BOX(hbox), lab, FALSE, FALSE, 2);

  entryW = gtk_entry_new();
  gtk_box_pack_start(GTK_BOX(hbox), entryW, FALSE, FALSE, 2);
  gtk_entry_set_max_length(GTK_ENTRY(entryW), 7);
  p->lengthN.entry = entryW;
  p->lengthN.bits = BITS_PER_PROGRAM_ACCESS;
  p->lengthN.CB_func = lengthCB;
  p->lengthN.data = p;
  p->lengthN.text = g_strdup_printf("0x%x", p->length);
  gtk_entry_set_text(GTK_ENTRY(entryW), p->lengthN.text);
  g_signal_connect(entryW, "activate",
                   G_CALLBACK(entry_hexCB), &p->lengthN);

  buttonW = gtk_button_new_with_label("Update");
  gtk_box_pack_start(GTK_BOX(hbox), buttonW, FALSE, FALSE, 2);
  g_signal_connect(buttonW, "clicked", G_CALLBACK(updateCB), p);

  p->drawing = gtk_drawing_area_new();
  gtk_widget_set_hexpand(p->drawing, TRUE);
  gtk_widget_set_vexpand(p->drawing, TRUE);
  g_signal_connect(p->drawing, "draw", G_CALLBACK(draw_CB), p);
  gtk_box_pack_start(GTK_BOX(vbox), p->drawing, TRUE, TRUE, 0);

  collect_data(p);

  gtk_widget_show_all(p->window);
}
