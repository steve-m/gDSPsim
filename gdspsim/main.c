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

#include <stdio.h>
#include <glib.h>
#include <gtk/gtk.h>
#include <decode_window.h>
#include <memory_window.h>
#include <chip_core.h>
#include <find_opcode.h>
#include <readfile.h>
#include <register_window.h>
#include <pipeline.h>
#include <fileIO.h>
#include <preferences.h>
#include <gnome.h>

extern struct _file_info *gdsp_file_nfo;
struct _Registers *Registers;
// Used so other windows can have same keyboard accelerations
GtkAccelGroup *gDSP_keyboard_accel;
int gStopRun;

static WordA program_start=0;

void set_program_start(WordA new_pc)
{
  Registers->PC=new_pc;
  program_start=new_pc;
}

static void preferences_CB( GtkWidget *widget,  gpointer   data )
{
  edit_preferences();
}

static void restart_CB( GtkWidget *widget,  gpointer   data )
{
  reset_view();
  Registers->Special_Flush=1;
  default_registers(Registers);
  Registers->PC=program_start;
  update_view();
}

static void step_CB( GtkWidget *widget,  gpointer   data )
{
  reset_view();
  pipeline(Registers);
  update_view();
}

static void stop_CB( GtkWidget *widget,  gpointer   data )
{
  gStopRun=1;
}

static void helpCB( GtkWidget *widget,  gpointer   data )
{
  gchar *helpfile;

  helpfile = gnome_help_file_find_file("gdspsim","gdspsim.html");
  if (helpfile != NULL)
    {
      gchar *url;

      url = g_strconcat("file:",helpfile,NULL);
      gnome_help_goto(NULL,url);
      g_free(url);
      g_free(helpfile);
    }
  else
    {
      // gnome_error_dialog(_("Counldn't fine the gDSPsim Manual"));
      printf("Counldn't fine the gDSPsim Manual\n");
    }
}

static void run_CB( GtkWidget *widget,  gpointer   data )
{
  reset_view();
  gStopRun = 0;
  while ( pipeline(Registers) == 0 && !gStopRun)
    while (gtk_events_pending())
      gtk_main_iteration();
  update_view();
}

static void animate_CB( GtkWidget *widget,  gpointer   data )
{
  gStopRun = 0;
  while ( pipeline(Registers) == 0 && !gStopRun)
    {
      reset_view();
      update_view();
      while (gtk_events_pending())
	gtk_main_iteration();
    }
}

static void destroy( GtkWidget *widget, gpointer   data )
{
  gtk_widget_destroy(widget);
  gtk_main_quit();
}

static void register_view_CB( GtkWidget *widget,  gpointer   data )
{
  create_register_window(Registers);
  return;
}

void create_plot_window();
static void plot_CB( GtkWidget *widget,  gpointer   data )
{
  create_plot_window();
  return;
}

/*------------------------------------------------------------------------*/
/* Font Selection */
/*------------------------------------------------------------------------*/
static GtkWidget* fontW=NULL;
GdkFont *gdsp_Decode_Font = NULL;

static void font_ok_CB( GtkWidget *W, gpointer data )
{
  gdsp_Decode_Font = gtk_font_selection_dialog_get_font(data);
  
  fontW = NULL;
  gtk_widget_destroy(data);
}
 
static void font_destroy_CB( GtkWidget *W, gpointer data )
{
  fontW = NULL;
  gtk_widget_destroy(W);
}

static void font_CB( GtkWidget *widget,  gpointer   data )
{
  // Only 1 font widget window allowed
  if ( fontW )
    return;  

  fontW = gtk_font_selection_dialog_new("Font Selector");
  
  gtk_signal_connect (GTK_OBJECT ( GTK_FONT_SELECTION_DIALOG (fontW)->ok_button),
		      "clicked", (GtkSignalFunc) font_ok_CB, fontW );
  
  /* Connect the cancel_button to destroy the widget */
  //gtk_signal_connect_object ( GTK_OBJECT ( GTK_FONT_SELECTION_DIALOG (fontW)->cancel_button),
  //		      "clicked", (GtkSignalFunc) font_destroy_CB,
  //		      GTK_OBJECT (fontW));
  gtk_signal_connect (GTK_OBJECT ( GTK_FONT_SELECTION_DIALOG (fontW)->cancel_button),
		      "clicked", (GtkSignalFunc) font_destroy_CB, fontW );
  
  gtk_signal_connect ( GTK_OBJECT (fontW), "destroy",
		       (GtkSignalFunc)font_destroy_CB, fontW);



  gtk_widget_show(fontW);

}

/*------------------------------------------------------------------------*/
/*------------------------------------------------------------------------*/

static void memory_CB( GtkWidget *widget,  gpointer   data )
{
  create_memory_window();
}

static void decode_CB( GtkWidget *widget,  gpointer   data )
{
  create_decode_window();
}

static GtkItemFactoryEntry menu_items[] = 
{
  { "/_File",         NULL,         NULL, 0, "<Branch>" },
  { "/File/_Open",    "<control>O", (GtkItemFactoryCallback)load_file_CB, 0, NULL },
  { "/File/_Quit",    "<control>Q", (GtkItemFactoryCallback)gtk_widget_destroy, 0, NULL },
  { "/File/sep1",     NULL,         NULL, 0, "<Separator>" },
  { "/_Edit",      NULL,        NULL, 0, "<Branch>" },
  { "/Edit/Preferences", NULL, (GtkItemFactoryCallback)preferences_CB , 0, NULL },
  { "/File/Font",    "", (GtkItemFactoryCallback)font_CB, 0, NULL },
  { "/_View",      NULL,        NULL, 0, "<Branch>" },
  { "/View/_Memory",    "<control>M", (GtkItemFactoryCallback)memory_CB, 0, NULL },
  { "/View/_Decode",    "<control>D", (GtkItemFactoryCallback)decode_CB, 0, NULL },
  { "/View/_Registers",    "<control>R", (GtkItemFactoryCallback)register_view_CB, 0, NULL },
  { "/View/Plot", NULL, (GtkItemFactoryCallback)plot_CB, 0, NULL },
  { "/_Simulate",      NULL,        NULL, 0, "<Branch>" },
  { "/Simulate/Step",  "F8",        (GtkItemFactoryCallback)step_CB, 0, NULL },
  { "/Simulate/Run",  "F5",        (GtkItemFactoryCallback)run_CB, 0, NULL },
  { "/Simulate/Stop",  "F6",        (GtkItemFactoryCallback)stop_CB, 0, NULL },
  { "/Simulate/Animate",  "F7",        (GtkItemFactoryCallback)animate_CB, 0, NULL },
  { "/Simulate/Restart",  NULL,        (GtkItemFactoryCallback)restart_CB, 0, NULL },
  { "/Simulate/Connect Input File", NULL, (GtkItemFactoryCallback)create_fileIO, 1, NULL },
  { "/Simulate/Connect Output File", NULL, (GtkItemFactoryCallback)create_fileIO, 0, NULL },
  { "/_Help",         NULL,         NULL, 0, "<LastBranch>" },
  { "/_Help/Documentation",   NULL, (GtkItemFactoryCallback)helpCB , 0, NULL },
};


static void get_main_menu( GtkWidget  *window, GtkWidget **menubar )
{
  GtkItemFactory *item_factory;
  gint nmenu_items = sizeof (menu_items) / sizeof (menu_items[0]);
  
  
  /* Create place to store keyboard accelerations */
  gDSP_keyboard_accel = gtk_accel_group_new ();
  
  /* Create the menu items */
  item_factory = gtk_item_factory_new (GTK_TYPE_MENU_BAR, "<main>", 
				       gDSP_keyboard_accel);
  
  /* This function generates the menu items. Pass the item factory,
     the number of items in the array, the array itself, and any
     callback data for the the menu items. */
  gtk_item_factory_create_items (item_factory, nmenu_items, menu_items, NULL);
  
  /* Attach the new accelerator group to the window. */
  gtk_window_add_accel_group (GTK_WINDOW (window), gDSP_keyboard_accel);
  
  if (menubar)
    /* Finally, return the actual menu bar created by the item factory. */ 
    *menubar = gtk_item_factory_get_widget (item_factory, "<main>");
}



int main(int argc, char *argv[])
{
  GtkWidget *window,*vbox;
  GtkWidget *menubar;
  int k;

  /* Setup main window */
  gtk_init (&argc, &argv);

  // Setup default registers
  Registers = pipe_new();

  /* Process flags */
  for (k=0;k<argc;k++)
    {
      char *group;

      group = argv[k];
      if (*group == '-')
	{
	  group++;
	  switch ( *group )
	    {
	    case 'f':
	      // File
	      printf("file = %s\n",argv[k+1]);
	      open_file(argv[k+1]);
	      break;
	    }
	}
    }

		    
  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);

  gtk_widget_set_name (window, "gDSPsim");
  gtk_widget_set_usize (GTK_WIDGET(window), 300, 200);
  gtk_window_set_title (GTK_WINDOW (window), "gDSPsim (gnu Digital Signal Processor Simulator)");

  gtk_signal_connect (GTK_OBJECT (window), "destroy",
		      GTK_SIGNAL_FUNC(destroy),
		      &window);
    
  gtk_container_set_border_width (GTK_CONTAINER (window), 10);



  /* Sets the border width of the window. */
  gtk_container_set_border_width (GTK_CONTAINER (window), 0);

  /* Add Menu */
  vbox = gtk_vbox_new (FALSE, 0);
  gtk_container_add (GTK_CONTAINER (window), vbox);
  gtk_widget_show (vbox);
  get_main_menu (window, &menubar);
  gtk_box_pack_start (GTK_BOX (vbox), menubar, FALSE, TRUE, 0);
  gtk_widget_show (menubar);

  gtk_widget_show (window);

  gtk_main ();

  return 0;
}
