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
#include "c54_core.h"
#include "register_window.h"
#include <gtk/gtk.h>

struct _reg_entries
{
  GtkWidget *PC;
  GtkWidget *A;
  GtkWidget *B;
  GtkWidget *SP;
  GtkWidget *ST0;
  GtkWidget *ST1;
  GtkWidget *T;
  GtkWidget *RC;

  GtkWidget *AR0;
  GtkWidget *AR1;
  GtkWidget *AR2;
  GtkWidget *AR3;
  GtkWidget *AR4;
  GtkWidget *AR5;
  GtkWidget *AR6;
  GtkWidget *AR7;

  GtkWidget *PAB;
  GtkWidget *IR;
  GtkWidget *DAB;
  GtkWidget *CAB;
  GtkWidget *DB;
  GtkWidget *CB;
  GtkWidget *EAB;
};


struct _reg_entries reg_entries;


void entry_hexCB( GtkWidget *widget,  GtkWidget *entryCB_nfo );

void entry_regCB( GtkWidget *widget,  GP_Reg *reg );

void entry_wordCB( GtkWidget *widget, Word *reg );

GtkWidget *registerW=NULL;

void entry_regCB( GtkWidget *widget,  GP_Reg *reg )
{
}

static void destroy_window_CB( GtkWidget *W, gpointer data )
{
  registerW=NULL;
}

static GtkWidget *set_reg_table(const gchar *label_name,int x, int y, GtkTable *table, gpointer user_data)
{
  GtkWidget *entry,*label;

  label = gtk_label_new(label_name);
  gtk_table_attach (table, label, x, x+1, y, y+1,
		    0,0,3,0);
  gtk_widget_show(label);

  entry = gtk_entry_new();
#ifdef GTK2
  gtk_entry_set_width_chars(GTK_ENTRY(entry),13);
#endif
  gtk_entry_set_max_length(GTK_ENTRY(entry),13);
  gtk_table_attach (table, entry, x+1, x+2, y, y+1,
		    0,0,0,0);
  gtk_widget_show (entry);  
  gtk_signal_connect(GTK_OBJECT(entry), "activate",
		     GTK_SIGNAL_FUNC(entry_wordCB),
		     user_data);
  return entry;

}


void create_register_window(struct _Registers *Registers)
{
  GtkWidget *table;
  extern GtkAccelGroup *gDSP_keyboard_accel;

  if ( registerW )
    return;

  // Create window
  registerW = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_signal_connect(GTK_OBJECT(registerW),"destroy",
		     (GtkSignalFunc)destroy_window_CB,NULL);

  // Attach keyboard accelerations from main window
  gtk_window_add_accel_group (GTK_WINDOW (registerW), gDSP_keyboard_accel);
  
  gtk_widget_set_name (registerW, "Registers");
  gtk_window_set_title (GTK_WINDOW (registerW),"Registers");
  gtk_container_set_border_width (GTK_CONTAINER (registerW), 0);

  table = gtk_table_new(8,6,FALSE);
  gtk_widget_show(table);
  gtk_container_add (GTK_CONTAINER (registerW), table);
  

  reg_entries.PC = set_reg_table("PC", 0, 0, GTK_TABLE(table), 
				  &Registers->PC);
  reg_entries.A = set_reg_table("A", 0, 1, GTK_TABLE(table), 
				  &MMR->A);
  reg_entries.B = set_reg_table("B", 0, 2, GTK_TABLE(table), 
				  &MMR->B);
  reg_entries.SP = set_reg_table("SP", 0, 3, GTK_TABLE(table), 
				  &MMR->SP);
  reg_entries.ST0 = set_reg_table("ST0", 0, 4, GTK_TABLE(table), 
				  &MMR->ST0);
  reg_entries.ST1 = set_reg_table("ST1", 0, 5, GTK_TABLE(table), 
				  &MMR->ST1);
  reg_entries.T = set_reg_table("T", 0, 6, GTK_TABLE(table), 
				  &MMR->T);
  reg_entries.RC = set_reg_table("RC", 0, 7, GTK_TABLE(table), 
				  &Registers->RC);


  reg_entries.AR0 = set_reg_table("AR0", 2, 0, GTK_TABLE(table), 
				  &MMR->ar0);
  reg_entries.AR1 = set_reg_table("AR1", 2, 1, GTK_TABLE(table), 
				  &MMR->ar1);
  reg_entries.AR2 = set_reg_table("AR2", 2, 2, GTK_TABLE(table), 
				  &MMR->ar2);
  reg_entries.AR3 = set_reg_table("AR3", 2, 3, GTK_TABLE(table), 
				  &MMR->ar3);
  reg_entries.AR4 = set_reg_table("AR4", 2, 4, GTK_TABLE(table), 
				  &MMR->ar4);
  reg_entries.AR5 = set_reg_table("AR5", 2, 5, GTK_TABLE(table), 
				  &MMR->ar5);
  reg_entries.AR6 = set_reg_table("AR6", 2, 6, GTK_TABLE(table), 
				  &MMR->ar6);
  reg_entries.AR7 = set_reg_table("AR7", 2, 7, GTK_TABLE(table), 
				  &MMR->ar7);

  reg_entries.PAB = set_reg_table("PAB", 4, 0, GTK_TABLE(table), 
				  &Registers->PAB);
  reg_entries.IR  = set_reg_table("IR",  4, 1, GTK_TABLE(table), 
				  &Registers->IR);
  reg_entries.DAB = set_reg_table("DAB", 4, 2, GTK_TABLE(table), 
				  &Registers->DAB);
  reg_entries.CAB = set_reg_table("CAB", 4, 3, GTK_TABLE(table), 
				  &Registers->CAB);
  reg_entries.DB  = set_reg_table("DB",  4, 4, GTK_TABLE(table), 
				  &Registers->DB);
  reg_entries.CB  = set_reg_table("CB",  4, 5, GTK_TABLE(table), 
				  &Registers->CB);
  reg_entries.EAB  = set_reg_table("EAB",  4, 6, GTK_TABLE(table), 
				  &Registers->EAB);


  fill_reg_entries(Registers);

  gtk_widget_show(registerW);
  return;

}

void fill_reg_entries(struct _Registers *Registers)
{
  gchar temp_str[15];

  g_return_if_fail(Registers != NULL);

  // Make sure register window exists
  if ( registerW==NULL )
    return;

  g_snprintf(temp_str,15,"0x%x",Registers->PC);
  gtk_entry_set_text (GTK_ENTRY(reg_entries.PC),temp_str);
 
  g_snprintf(temp_str,15,"0x%2.2x%2.2x%2.2x%2.2x%2.2x",MMR->A.byte4,MMR->A.byte3,MMR->A.byte2,MMR->A.byte1,MMR->A.byte0);
  gtk_entry_set_text (GTK_ENTRY(reg_entries.A),temp_str);
  
  g_snprintf(temp_str,15,"0x%2.2x%2.2x%2.2x%2.2x%2.2x",MMR->B.byte4,MMR->B.byte3,MMR->B.byte2,MMR->B.byte1,MMR->B.byte0);
  gtk_entry_set_text (GTK_ENTRY(reg_entries.B),temp_str);
 
  g_snprintf(temp_str,15,"0x%x",MMR->SP);
  gtk_entry_set_text (GTK_ENTRY(reg_entries.SP),temp_str);
 
  g_snprintf(temp_str,15,"0x%x",MMR->ST0);
  gtk_entry_set_text (GTK_ENTRY(reg_entries.ST0),temp_str);
 
  g_snprintf(temp_str,15,"0x%x",MMR->ST1);
  gtk_entry_set_text (GTK_ENTRY(reg_entries.ST1),temp_str);
 
  g_snprintf(temp_str,15,"0x%x",MMR->T);
  gtk_entry_set_text (GTK_ENTRY(reg_entries.T),temp_str);
 
  g_snprintf(temp_str,15,"0x%x",Registers->RC);
  gtk_entry_set_text (GTK_ENTRY(reg_entries.RC),temp_str);
 
  g_snprintf(temp_str,15,"0x%x",MMR->ar0);
  gtk_entry_set_text (GTK_ENTRY(reg_entries.AR0),temp_str);
 
  g_snprintf(temp_str,15,"0x%x",MMR->ar1);
  gtk_entry_set_text (GTK_ENTRY(reg_entries.AR1),temp_str);
 
  g_snprintf(temp_str,15,"0x%x",MMR->ar2);
  gtk_entry_set_text (GTK_ENTRY(reg_entries.AR2),temp_str);
 
  g_snprintf(temp_str,15,"0x%x",MMR->ar3);
  gtk_entry_set_text (GTK_ENTRY(reg_entries.AR3),temp_str);
 
  g_snprintf(temp_str,15,"0x%x",MMR->ar4);
  gtk_entry_set_text (GTK_ENTRY(reg_entries.AR4),temp_str);
 
  g_snprintf(temp_str,15,"0x%x",MMR->ar5);
  gtk_entry_set_text (GTK_ENTRY(reg_entries.AR5),temp_str);
 
  g_snprintf(temp_str,15,"0x%x",MMR->ar6);
  gtk_entry_set_text (GTK_ENTRY(reg_entries.AR6),temp_str);
 
  g_snprintf(temp_str,15,"0x%x",MMR->ar7);
  gtk_entry_set_text (GTK_ENTRY(reg_entries.AR7),temp_str);
 
  g_snprintf(temp_str,15,"0x%x",Registers->PAB);
  gtk_entry_set_text (GTK_ENTRY(reg_entries.PAB),temp_str);
 
  g_snprintf(temp_str,15,"0x%x",Registers->IR);
  gtk_entry_set_text (GTK_ENTRY(reg_entries.IR),temp_str);
 
  g_snprintf(temp_str,15,"0x%x",Registers->DAB);
  gtk_entry_set_text (GTK_ENTRY(reg_entries.DAB),temp_str);
 
  g_snprintf(temp_str,15,"0x%x",Registers->CAB);
  gtk_entry_set_text (GTK_ENTRY(reg_entries.CAB),temp_str);
 
  g_snprintf(temp_str,15,"0x%x",Registers->DB);
  gtk_entry_set_text (GTK_ENTRY(reg_entries.DB),temp_str);
 
  g_snprintf(temp_str,15,"0x%x",Registers->CB);
  gtk_entry_set_text (GTK_ENTRY(reg_entries.CB),temp_str);
 
  g_snprintf(temp_str,15,"0x%x",Registers->EAB);
  gtk_entry_set_text (GTK_ENTRY(reg_entries.EAB),temp_str);
 
}
