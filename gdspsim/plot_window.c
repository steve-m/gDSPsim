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
#include <gtk/gtk.h>
#include <gtkextra/gtkextra.h>
#include <entryCB.h>
#include <memory.h>

struct _plot_window_nfo
{
  GtkWidget *window;
  GtkWidget *canvas;
  WordP address;
  Word length;
  GtkPlot *plotW;
  struct _entryCB_nfo addressN;
  struct _entryCB_nfo lengthN;
  GtkPlotData *plotData;
  gdouble *xdata;
  gdouble *ydata;
  int type;
};

// List of all created plot windows. 
// List of pointers to struct _plot_window_nfo
GList *all_plot_windowL=NULL;

static void display_plot(struct _plot_window_nfo *pnfo)
{
  Word wrd,wrd2;
  guint32 wrd32;
  GdkColor color;
  int k,wait_state;

  // remove old data
  if ( pnfo->xdata )
    {
      g_free(pnfo->xdata);
      g_free(pnfo->ydata);
    }

  // Get data for plot
  pnfo->ydata = g_new(gdouble,pnfo->length);
  pnfo->xdata = g_new(gdouble,pnfo->length);
  if ( pnfo->type < 3 )
    {
      for (k=0;k<pnfo->length;k++)
	{
	  wrd = read_data_mem_long(pnfo->address+k,&wait_state);
	  pnfo->xdata[k] = (gdouble)k;
	  switch (pnfo->type)
	    {
	    case 0:
	      pnfo->ydata[k] = (gdouble)((gint16)wrd);
	      break;
	    case 1:
	      pnfo->ydata[k] = (gdouble)wrd;
	      break;
	    case 2:
	      pnfo->ydata[k] = (gdouble)((gint16)wrd)/32768.;
	      break;
	    }
	}
    }
  else
    {
      for (k=0;k<pnfo->length;k++)
	{
	  wrd = read_data_mem_long(pnfo->address+2*k,&wait_state);
	  wrd2 = read_data_mem_long(pnfo->address+2*k+1,&wait_state);
	  wrd32 = (guint32)wrd2 |  ((guint32)wrd)<<16;
	  pnfo->xdata[k] = (gdouble)k;
	  switch (pnfo->type)
	    {
	    case 3:
	      pnfo->ydata[k] = (gdouble)((gint32)wrd32);
	      break;
	    case 4:
	      pnfo->ydata[k] = (gdouble)wrd32;
	      break;
	    case 5:
	      pnfo->ydata[k] = (gdouble)((gint32)wrd32)/2147483648.;
	      break;
	    }
	}
    }


  //gtk_plot_refresh(pnfo->plotW,NULL);
  gtk_plot_data_set_points(pnfo->plotData, pnfo->xdata, pnfo->ydata, NULL, 
			   NULL, pnfo->length);
  
  gdk_color_parse("red", &color);
  gdk_color_alloc(gdk_colormap_get_system(), &color); 
  
  gtk_plot_data_set_line_attributes(pnfo->plotData, GTK_PLOT_LINE_SOLID,
				    1, &color);
  gtk_plot_data_set_connector(pnfo->plotData, GTK_PLOT_CONNECT_STRAIGHT);

  gtk_plot_autoscale(pnfo->plotW);

  gtk_plot_paint(pnfo->plotW);

  gtk_plot_refresh(pnfo->plotW,NULL);
  gtk_plot_refresh(pnfo->plotW,NULL);
 

 }

static void start_addressCB(GtkWidget *entry, guint64 num, 
			    gpointer data)
{
  struct _plot_window_nfo *pwn;

  pwn = data;
  pwn->address = num;
  display_plot(pwn);
}

static void lengthCB(GtkWidget *entry, guint64 num, 
		     gpointer data)
{
  struct _plot_window_nfo *pwn;

  pwn = data;
  pwn->length = num;
  display_plot(pwn);
}

static void updateCB(GtkWidget *entry, gpointer data)
{
  struct _plot_window_nfo *pwn;

  pwn = data;
  display_plot(pwn);
}

static void destroy_window_CB( GtkWidget *W, gpointer data )
{
  struct _plot_window_nfo *nfo;

  nfo=data;

  all_plot_windowL=g_list_remove(all_plot_windowL,nfo);

  g_free(nfo->addressN.text);
  g_free(nfo->lengthN.text);
  g_free(nfo);
}

static void setTypeInt16_CB( GtkWidget *W, struct _plot_window_nfo *pnfo)
{
  int state;

  state = GTK_CHECK_MENU_ITEM(W)->active;
  if ( state )
    {
      pnfo->type = 0;
      display_plot(pnfo);
    }
}

static void setTypeInt32_CB( GtkWidget *W, struct _plot_window_nfo *pnfo)
{
  int state;

  state = GTK_CHECK_MENU_ITEM(W)->active;
  if ( state )
    {
      pnfo->type = 3;
      display_plot(pnfo);
    }
}

static void setTypeUInt16_CB( GtkWidget *W, struct _plot_window_nfo *pnfo)
{
  int state;

  state = GTK_CHECK_MENU_ITEM(W)->active;
  if ( state )
    {
      pnfo->type = 1;
      display_plot(pnfo);
    }
}

static void setTypeUInt32_CB( GtkWidget *W, struct _plot_window_nfo *pnfo)
{
  int state;

  state = GTK_CHECK_MENU_ITEM(W)->active;
  if ( state )
    {
      pnfo->type = 4;
      display_plot(pnfo);
    }
}

static void setTypeQ15_CB( GtkWidget *W, struct _plot_window_nfo *pnfo)
{
  int state;

  state = GTK_CHECK_MENU_ITEM(W)->active;
  if ( state )
    {
      pnfo->type = 2;
      display_plot(pnfo);
    }
}

static void setTypeQ31_CB( GtkWidget *W, struct _plot_window_nfo *pnfo)
{
  int state;

  state = GTK_CHECK_MENU_ITEM(W)->active;
  if ( state )
    {
      pnfo->type = 5;
      display_plot(pnfo);
    }
}

static void set_menu( GtkBox *boxW, struct _plot_window_nfo *pnfo)
{
  GtkWidget *menu;
  GtkWidget *menu_bar;

  GSList *group = NULL;
  GtkWidget *item;


  menu = gtk_menu_new ();    /* Don't need to show menus */

  item = gtk_radio_menu_item_new_with_label (group, "16-bit Int");
  group = gtk_radio_menu_item_group (GTK_RADIO_MENU_ITEM (item));
  gtk_widget_show(item);
  gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (item), TRUE);
  gtk_menu_append (GTK_MENU (menu), item);
  gtk_signal_connect(GTK_OBJECT(item),"toggled",
		     (GtkSignalFunc)setTypeInt16_CB,pnfo);

  item = gtk_radio_menu_item_new_with_label (group, "32-bit Int");
  group = gtk_radio_menu_item_group (GTK_RADIO_MENU_ITEM (item));
  gtk_widget_show(item);
  gtk_menu_append (GTK_MENU (menu), item);
  gtk_signal_connect(GTK_OBJECT(item),"toggled",
		     (GtkSignalFunc)setTypeInt32_CB,pnfo);

  item = gtk_radio_menu_item_new_with_label (group, "16-bit Unsigned Int");
  group = gtk_radio_menu_item_group (GTK_RADIO_MENU_ITEM (item));
  gtk_widget_show(item);
  gtk_menu_append (GTK_MENU (menu), item);
  gtk_signal_connect(GTK_OBJECT(item),"toggled",
		     (GtkSignalFunc)setTypeUInt16_CB,pnfo);

  item = gtk_radio_menu_item_new_with_label (group, "32-bit Unsigned Int");
  group = gtk_radio_menu_item_group (GTK_RADIO_MENU_ITEM (item));
  gtk_widget_show(item);
  gtk_menu_append (GTK_MENU (menu), item);
  gtk_signal_connect(GTK_OBJECT(item),"toggled",
		     (GtkSignalFunc)setTypeUInt32_CB,pnfo);

  item = gtk_radio_menu_item_new_with_label (group, "Q15");
  group = gtk_radio_menu_item_group (GTK_RADIO_MENU_ITEM (item));
  gtk_widget_show(item);
  gtk_menu_append (GTK_MENU (menu), item);
  gtk_signal_connect(GTK_OBJECT(item),"toggled",
		     (GtkSignalFunc)setTypeQ15_CB,pnfo);

  item = gtk_radio_menu_item_new_with_label (group, "Q31");
  group = gtk_radio_menu_item_group (GTK_RADIO_MENU_ITEM (item));
  gtk_widget_show(item);
  gtk_menu_append (GTK_MENU (menu), item);
  gtk_signal_connect(GTK_OBJECT(item),"toggled",
		     (GtkSignalFunc)setTypeQ31_CB,pnfo);

  // create menu bar and add it to widget
  menu_bar = gtk_menu_bar_new();
  gtk_box_pack_start (GTK_BOX (boxW), menu_bar, FALSE, FALSE, 2);
  gtk_widget_show (menu_bar);

  item = gtk_menu_item_new_with_label ("Type");
  gtk_widget_show (item);
  
  gtk_menu_item_set_submenu (GTK_MENU_ITEM (item), menu);
    
  gtk_menu_bar_append (GTK_MENU_BAR (menu_bar), item);

}



void create_plot_window()
{
  GtkWidget *scrollW,*buttonW;
  GtkWidget *active_plot;
  GtkWidget *canvas,*hbox,*vbox,*lab,*entryW;
  gint page_width, page_height;
  gfloat scale = 1.;
  struct _plot_window_nfo *plot_windowN;

  // Create new window and place in list of plot windows
  plot_windowN = g_new(struct _plot_window_nfo, 1);
  plot_windowN->type = 0;

  all_plot_windowL = g_list_prepend(all_plot_windowL,plot_windowN);

  plot_windowN->window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_signal_connect(GTK_OBJECT(plot_windowN->window),"destroy",
		     (GtkSignalFunc)destroy_window_CB,plot_windowN);
  
  gtk_window_set_title(GTK_WINDOW(plot_windowN->window), "Plot");
  gtk_widget_set_usize(plot_windowN->window,550,350);
  gtk_container_border_width(GTK_CONTAINER(plot_windowN->window),0);
  
  // Set up containers for window
  hbox = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (hbox);

  vbox = gtk_vbox_new (FALSE, 0);
  gtk_container_add (GTK_CONTAINER (plot_windowN->window), vbox);
  gtk_widget_show (vbox);

  gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 2);

  // Set up menu bar and pack it into widget
  set_menu(GTK_BOX(hbox),plot_windowN);

  // Place address and length entry widgets besides menu
  lab = gtk_label_new("Start");
  gtk_box_pack_start(GTK_BOX (hbox), lab, FALSE, FALSE, 2);
  gtk_widget_show(lab);

  // Start address entry
  plot_windowN->address = 0x80; // default value
  entryW = gtk_entry_new();
  gtk_widget_show(entryW);
  gtk_box_pack_start(GTK_BOX (hbox), entryW, FALSE, FALSE, 2);
  gtk_entry_set_max_length(GTK_ENTRY(entryW),64);
  gtk_editable_set_editable(GTK_EDITABLE(entryW),TRUE);
  

  gtk_signal_connect(GTK_OBJECT(entryW), "activate",
		     GTK_SIGNAL_FUNC(entry_addressCB),
		     &plot_windowN->addressN);

  plot_windowN->addressN.entry = entryW;
  plot_windowN->addressN.bits = BITS_PER_PROGRAM_ACCESS;
  plot_windowN->addressN.CB_func = start_addressCB;
  plot_windowN->addressN.data = plot_windowN;
  plot_windowN->addressN.text = g_strdup_printf("0x%x",plot_windowN->address);
  gtk_entry_set_text(GTK_ENTRY(entryW),plot_windowN->addressN.text);
  

  // length label
  lab = gtk_label_new("Length");
  gtk_box_pack_start(GTK_BOX (hbox), lab, FALSE, FALSE, 2);
  gtk_widget_show(lab);

  // Length entry
  plot_windowN->length = 0x80; // default value
  entryW = gtk_entry_new();
  gtk_widget_show(entryW);
  gtk_box_pack_start(GTK_BOX (hbox), entryW, FALSE, FALSE, 2);
  gtk_entry_set_max_length(GTK_ENTRY(entryW),7);
  gtk_editable_set_editable(GTK_EDITABLE(entryW),TRUE);

  gtk_signal_connect(GTK_OBJECT(entryW), "activate",
		     GTK_SIGNAL_FUNC(entry_hexCB),
		     &plot_windowN->lengthN);

  plot_windowN->lengthN.entry  = entryW;
  plot_windowN->lengthN.bits = BITS_PER_PROGRAM_ACCESS;
  plot_windowN->lengthN.CB_func = lengthCB;
  plot_windowN->lengthN.data = plot_windowN;
  plot_windowN->lengthN.text = g_strdup_printf("0x%x",plot_windowN->length);
  gtk_entry_set_text(GTK_ENTRY(entryW),plot_windowN->lengthN.text);
 
  // Update Button
  buttonW = gtk_button_new_with_label("Update");
  gtk_widget_show(buttonW);
  gtk_box_pack_start(GTK_BOX (hbox), buttonW, FALSE, FALSE, 2);
  gtk_signal_connect(GTK_OBJECT(buttonW), "pressed",
		     GTK_SIGNAL_FUNC(updateCB),
		     plot_windowN);
  

  scrollW=gtk_scrolled_window_new(NULL, NULL);
  gtk_container_border_width(GTK_CONTAINER(scrollW),0);
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrollW),
				 GTK_POLICY_ALWAYS,GTK_POLICY_ALWAYS);
  gtk_box_pack_start(GTK_BOX(vbox),scrollW, TRUE, TRUE,0);
  gtk_widget_show(scrollW);

  /* Width and Height for 1 page printed */
  page_width = GTK_PLOT_LETTER_W * scale;
  page_height = GTK_PLOT_LETTER_H * scale;
  
  canvas = gtk_plot_canvas_new(page_width, page_height, (gdouble)1.);
  plot_windowN->canvas = canvas;
  GTK_PLOT_CANVAS_SET_FLAGS(GTK_PLOT_CANVAS(canvas), GTK_PLOT_CANVAS_DND_FLAGS);
  gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(scrollW), canvas);
  gtk_widget_show(canvas);
  
  active_plot = gtk_plot_new_with_size(NULL, .5, .25);
  gtk_widget_show(active_plot);
  plot_windowN->xdata = NULL;
  plot_windowN->ydata = NULL;
  plot_windowN->plotW = GTK_PLOT(active_plot);

  gtk_plot_autoscale(GTK_PLOT(active_plot));
  gtk_plot_legends_move(GTK_PLOT(active_plot), .500, .05);
  gtk_plot_set_legends_border(GTK_PLOT(active_plot), 0, 0);
  gtk_plot_axis_hide_title(GTK_PLOT(active_plot), GTK_PLOT_AXIS_TOP);
  
  gtk_plot_axis_show_ticks(GTK_PLOT(active_plot), 3, 0xf, 0x8);
  
  gtk_plot_axis_hide_title(GTK_PLOT(active_plot),GTK_PLOT_AXIS_BOTTOM);
  gtk_plot_axis_hide_title(GTK_PLOT(active_plot),GTK_PLOT_AXIS_RIGHT);
  gtk_plot_axis_hide_title(GTK_PLOT(active_plot),GTK_PLOT_AXIS_LEFT);
  
  
  /* Allows each axis to be set visible */
  gtk_plot_axis_set_visible(GTK_PLOT(active_plot), GTK_PLOT_AXIS_TOP, TRUE);
  gtk_plot_axis_set_visible(GTK_PLOT(active_plot), GTK_PLOT_AXIS_RIGHT, TRUE);
  
  gtk_plot_canvas_add_plot(GTK_PLOT_CANVAS(canvas), GTK_PLOT(active_plot), .15, .06);

  plot_windowN->plotData = GTK_PLOT_DATA(gtk_plot_data_new());
  gtk_widget_show(GTK_WIDGET(plot_windowN->plotData));
  gtk_plot_add_data(plot_windowN->plotW,plot_windowN->plotData);

  display_plot(plot_windowN);


  gtk_widget_show (plot_windowN->window);
}
