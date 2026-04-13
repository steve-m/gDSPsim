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

extern struct _file_info *gdsp_file_nfo;
struct _Registers *Registers;
GtkAccelGroup *gDSP_keyboard_accel;
int gStopRun;

static WordP program_start=0;

void set_program_start(WordP new_pc)
{
  Registers->PC=new_pc;
  program_start=new_pc;
}

static void preferences_CB(GtkWidget *w, gpointer d)
{
  edit_preferences();
}

static void restart_CB(GtkWidget *w, gpointer d)
{
  reset_view();
  Registers->Special_Flush=1;
  default_registers(Registers);
  Registers->PC=program_start;
  update_view();
}

static void step_CB(GtkWidget *w, gpointer d)
{
  reset_view();
  pipeline(Registers);
  update_view();
}

static void stop_CB(GtkWidget *w, gpointer d)
{
  gStopRun=1;
}

static void helpCB(GtkWidget *w, gpointer d)
{
  printf("gDSPsim Manual: see doc/gdspsim.html\n");
}

static void run_CB(GtkWidget *w, gpointer d)
{
  reset_view();
  gStopRun = 0;
  while (pipeline(Registers) == 0 && !gStopRun)
    while (gtk_events_pending())
      gtk_main_iteration();
  update_view();
}

static void animate_CB(GtkWidget *w, gpointer d)
{
  gStopRun = 0;
  while (pipeline(Registers) == 0 && !gStopRun)
    {
      reset_view();
      update_view();
      while (gtk_events_pending())
        gtk_main_iteration();
    }
}

static void destroy(GtkWidget *w, gpointer d)
{
  gtk_main_quit();
}

static void register_view_CB(GtkWidget *w, gpointer d)
{
  create_register_window(Registers);
}

void create_plot_window(void);
static void plot_CB(GtkWidget *w, gpointer d)
{
  create_plot_window();
}

static void memory_CB(GtkWidget *w, gpointer d)
{
  create_memory_window();
}

static void decode_CB(GtkWidget *w, gpointer d)
{
  create_decode_window();
}

static void input_file_CB(GtkWidget *w, gpointer d)
{
  create_fileIO(w, GINT_TO_POINTER(1));
}

static void output_file_CB(GtkWidget *w, gpointer d)
{
  create_fileIO(w, GINT_TO_POINTER(0));
}

static void add_menu_item(GtkWidget *menu, const gchar *label,
                          const gchar *accel_key, guint key, GdkModifierType mods,
                          GCallback cb)
{
  GtkWidget *item = gtk_menu_item_new_with_mnemonic(label);
  gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);
  g_signal_connect(item, "activate", cb, NULL);
  if (key != 0)
    gtk_widget_add_accelerator(item, "activate", gDSP_keyboard_accel,
                               key, mods, GTK_ACCEL_VISIBLE);
}

static GtkWidget *build_menubar(GtkWidget *window)
{
  GtkWidget *menubar, *menu, *item;

  gDSP_keyboard_accel = gtk_accel_group_new();
  gtk_window_add_accel_group(GTK_WINDOW(window), gDSP_keyboard_accel);

  menubar = gtk_menu_bar_new();

  /* File */
  menu = gtk_menu_new();
  item = gtk_menu_item_new_with_mnemonic("_File");
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(item), menu);
  gtk_menu_shell_append(GTK_MENU_SHELL(menubar), item);
  add_menu_item(menu, "_Open", "<ctrl>O", GDK_KEY_o, GDK_CONTROL_MASK,
                G_CALLBACK(load_file_CB));
  add_menu_item(menu, "_Quit", "<ctrl>Q", GDK_KEY_q, GDK_CONTROL_MASK,
                G_CALLBACK(destroy));

  /* Edit */
  menu = gtk_menu_new();
  item = gtk_menu_item_new_with_mnemonic("_Edit");
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(item), menu);
  gtk_menu_shell_append(GTK_MENU_SHELL(menubar), item);
  add_menu_item(menu, "_Preferences", NULL, 0, 0, G_CALLBACK(preferences_CB));

  /* View */
  menu = gtk_menu_new();
  item = gtk_menu_item_new_with_mnemonic("_View");
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(item), menu);
  gtk_menu_shell_append(GTK_MENU_SHELL(menubar), item);
  add_menu_item(menu, "_Memory", "<ctrl>M", GDK_KEY_m, GDK_CONTROL_MASK,
                G_CALLBACK(memory_CB));
  add_menu_item(menu, "_Decode", "<ctrl>D", GDK_KEY_d, GDK_CONTROL_MASK,
                G_CALLBACK(decode_CB));
  add_menu_item(menu, "_Registers", "<ctrl>R", GDK_KEY_r, GDK_CONTROL_MASK,
                G_CALLBACK(register_view_CB));
  add_menu_item(menu, "_Plot", NULL, 0, 0, G_CALLBACK(plot_CB));

  /* Simulate */
  menu = gtk_menu_new();
  item = gtk_menu_item_new_with_mnemonic("_Simulate");
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(item), menu);
  gtk_menu_shell_append(GTK_MENU_SHELL(menubar), item);
  add_menu_item(menu, "_Step", "F8", GDK_KEY_F8, 0, G_CALLBACK(step_CB));
  add_menu_item(menu, "_Run", "F5", GDK_KEY_F5, 0, G_CALLBACK(run_CB));
  add_menu_item(menu, "S_top", "F6", GDK_KEY_F6, 0, G_CALLBACK(stop_CB));
  add_menu_item(menu, "_Animate", "F7", GDK_KEY_F7, 0, G_CALLBACK(animate_CB));
  add_menu_item(menu, "Re_start", NULL, 0, 0, G_CALLBACK(restart_CB));
  add_menu_item(menu, "Connect _Input File", NULL, 0, 0,
                G_CALLBACK(input_file_CB));
  add_menu_item(menu, "Connect _Output File", NULL, 0, 0,
                G_CALLBACK(output_file_CB));

  /* Help */
  menu = gtk_menu_new();
  item = gtk_menu_item_new_with_mnemonic("_Help");
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(item), menu);
  gtk_menu_shell_append(GTK_MENU_SHELL(menubar), item);
  add_menu_item(menu, "_Documentation", NULL, 0, 0, G_CALLBACK(helpCB));

  return menubar;
}

int main(int argc, char *argv[])
{
  GtkWidget *window, *vbox, *menubar;
  int k;

  gtk_init(&argc, &argv);

  Registers = pipe_new();

  for (k=0;k<argc;k++)
    {
      char *group = argv[k];
      if (*group == '-')
        {
          group++;
          switch (*group)
            {
            case 'f':
              printf("file = %s\n", argv[k+1]);
              open_file(argv[k+1]);
              break;
            }
        }
    }

  window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_widget_set_name(window, "gDSPsim");
  gtk_widget_set_size_request(window, 300, 200);
  gtk_window_set_title(GTK_WINDOW(window),
                       "gDSPsim (gnu Digital Signal Processor Simulator)");
  g_signal_connect(window, "destroy", G_CALLBACK(destroy), NULL);
  gtk_container_set_border_width(GTK_CONTAINER(window), 0);

  vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  gtk_container_add(GTK_CONTAINER(window), vbox);

  menubar = build_menubar(window);
  gtk_box_pack_start(GTK_BOX(vbox), menubar, FALSE, TRUE, 0);

  gtk_widget_show_all(window);

  gtk_main();

  return 0;
}
