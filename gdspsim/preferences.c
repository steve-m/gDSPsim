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

#include "preferences.h"

// globals
gboolean bound_program_mem_read=TRUE;
gboolean bound_data_mem_read=TRUE;
gboolean add_data_mem_on_write=TRUE;
gboolean add_data_mem_on_read=FALSE;

GtkWidget *preferenceW=NULL;



static void toggled(GtkToggleButton *togglebutton, gboolean *value)
{
  *value = gtk_toggle_button_get_active (togglebutton);
}


void edit_preferences(void)
{
  GtkWidget *notebook,*vbox,*label;
  GtkWidget *checkbutton;



  if ( preferenceW )
    return;

  preferenceW = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_widget_set_name (preferenceW, "Preferences");
  gtk_window_set_title (GTK_WINDOW(preferenceW), "Preferences");

  notebook = gtk_notebook_new();
  gtk_widget_show (notebook);
  gtk_container_add (GTK_CONTAINER (preferenceW), notebook);

  //gtk_container_add (GTK_CONTAINER (notebook), vbox);

  label = gtk_label_new("Breakpoints");
  vbox = gtk_vbox_new (FALSE, 0);
  gtk_widget_show (vbox);
  gtk_notebook_append_page(GTK_NOTEBOOK(notebook),vbox,label);

  checkbutton = gtk_check_button_new_with_label ("Break on reading out of bounds Program Memory");
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbutton),
			       bound_program_mem_read);
  gtk_widget_show(checkbutton);
  gtk_signal_connect(GTK_OBJECT(checkbutton),"toggled",(GtkSignalFunc)toggled,
		     &bound_program_mem_read);
  gtk_box_pack_start(GTK_BOX(vbox),checkbutton,TRUE,TRUE,0);

  checkbutton = gtk_check_button_new_with_label ("Break on reading out of bounds Data Memory");
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbutton),
			       bound_data_mem_read);
  gtk_widget_show(checkbutton);
  gtk_signal_connect(GTK_OBJECT(checkbutton),"toggled",(GtkSignalFunc)toggled,
		     &bound_data_mem_read);
  gtk_box_pack_start(GTK_BOX(vbox),checkbutton,TRUE,TRUE,0);

  checkbutton = gtk_check_button_new_with_label ("Add memory when writing");
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbutton),
			       add_data_mem_on_write);
  gtk_widget_show(checkbutton);
  gtk_signal_connect(GTK_OBJECT(checkbutton),"toggled",(GtkSignalFunc)toggled,
		     &add_data_mem_on_write);
  gtk_box_pack_start(GTK_BOX(vbox),checkbutton,TRUE,TRUE,0);

  checkbutton = gtk_check_button_new_with_label ("Add memory when reading");
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbutton),
			       add_data_mem_on_read);
  gtk_widget_show(checkbutton);
  gtk_signal_connect(GTK_OBJECT(checkbutton),"toggled",(GtkSignalFunc)toggled,
		     &add_data_mem_on_read);
  gtk_box_pack_start(GTK_BOX(vbox),checkbutton,TRUE,TRUE,0);


  gtk_widget_show(preferenceW);
}
