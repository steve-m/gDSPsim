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
#include "entryCB.h"
#include "decode.h"
#include "string.h"

typedef enum { WORD_TYPE=0, GP_REG_TYPE=1 } Reg_Type;

struct _entry_nfo
{
  GtkWidget *W;
  guint64 mask;
  gpointer reg;  // pointer to Word or GP_Reg
  Reg_Type type;
};

struct _reg_entries
{
  struct _entry_nfo PC;
  struct _entry_nfo A;
  struct _entry_nfo B;
  struct _entry_nfo SP;
  struct _entry_nfo ST0;
  struct _entry_nfo ST1;
  struct _entry_nfo T;
  struct _entry_nfo RC;
  struct _entry_nfo IMR;
  struct _entry_nfo IFR;
  struct _entry_nfo TRN;

  struct _entry_nfo AR0;
  struct _entry_nfo AR1;
  struct _entry_nfo AR2;
  struct _entry_nfo AR3;
  struct _entry_nfo AR4;
  struct _entry_nfo AR5;
  struct _entry_nfo AR6;
  struct _entry_nfo AR7;

  struct _entry_nfo PAB;
  struct _entry_nfo IR;
  struct _entry_nfo DAB;
  struct _entry_nfo CAB;
  struct _entry_nfo DB;
  struct _entry_nfo CB;
  struct _entry_nfo EAB;
  struct _entry_nfo BRC;
  struct _entry_nfo RSA;
  struct _entry_nfo REA;
  struct _entry_nfo PMST;
  struct _entry_nfo XPC;

  // Flag bits that make up ST0 and ST1
  struct _entry_nfo C;
  struct _entry_nfo OVA;
  struct _entry_nfo OVB;
  struct _entry_nfo FRCT;
  struct _entry_nfo SXM;
  struct _entry_nfo OVM;
  struct _entry_nfo ARP;
  struct _entry_nfo TC;
  struct _entry_nfo DP;
  struct _entry_nfo BRAF;
  struct _entry_nfo CPL;
  struct _entry_nfo XF;
  struct _entry_nfo HM;
  struct _entry_nfo INTM;
  struct _entry_nfo C16;
  struct _entry_nfo CMPT;
  struct _entry_nfo ASM;
};


struct _reg_entries reg_entries;


GtkWidget *registerW=NULL;
GtkStyle *styleN=NULL;
GtkStyle *styleH=NULL;


struct _Registers *Reg=NULL;


void entry_regCB( GtkWidget *widget, struct _entry_nfo *nfo )
{
  if ( nfo->type == WORD_TYPE )
    entry_word_maskCB(widget,nfo->reg,nfo->mask);
  else if ( nfo->type == GP_REG_TYPE )
    entry_gpreg_maskCB(widget,nfo->reg,nfo->mask);
  fill_reg_entries(Reg);

}

// this allows a symbol to be entered for the word.
static void entry_reg_PC_CB( GtkWidget *W, struct _entry_nfo *nfo )
{
  WordA address;
  gchar temp_str[15];
  gchar *entry_text;
  struct _Registers *Registers;

  entry_text = gtk_editable_get_chars(GTK_EDITABLE(W),0,-1);

  if ( text_to_address(entry_text,&address) )
    {
      //GdkColor c;
      //GtkStyle *style;
      // valid number
      Registers = nfo->reg;
      Registers->PC = address;
      Registers->Special_Flush = 1;
      // set text style to highlight mode
      gtk_widget_set_style(W, styleH);
      //c.red = 0xffff;
      //c.green = 0x0;
      //c.blue = 0;
      //gtk_widget_modify_fg(W,GTK_STATE_NORMAL,c);
      gtk_entry_set_text (GTK_ENTRY(W),entry_text);
      //style=gtk_widget_get_style (W);
      //style->text[GTK_STATE_NORMAL] = c;
      //gtk_widget_modify_style (W,GTK_RC_STYLE(style));
    }
  else
    {
      // invalid answer, set text color to normal
      gtk_widget_set_style(W, styleN);
      Registers = nfo->reg;
      g_snprintf(temp_str,15,"0x%x",Registers->PC);
      gtk_entry_set_text (GTK_ENTRY(W),temp_str);
    }

  g_free(entry_text);
}

static void destroy_window_CB( GtkWidget *W, gpointer data )
{
  registerW=NULL;
  styleN=NULL;  //memory leak?
  styleH=NULL;  //memory leak?
}

static void set_reg_table(struct _entry_nfo *entry, const gchar *label_name,
			  int x, int y, GtkTable *table, gpointer user_data, 
			  Reg_Type type, guint64 mask)
{
  GtkWidget *label;

  label = gtk_label_new(label_name);
  gtk_table_attach (table, label, x, x+1, y, y+1,
		    0,0,3,0);
  gtk_widget_show(label);

  entry->W = gtk_entry_new();
		     
#ifdef GTK2
  if ( type == WORD_TYPE )
    gtk_entry_set_width_chars(GTK_ENTRY(entry->W),7);
  else
    gtk_entry_set_width_chars(GTK_ENTRY(entry->W),13);
#else
  if ( type == WORD_TYPE )
    gtk_widget_set_usize(GTK_WIDGET(entry->W),50,24);
  else
    gtk_widget_set_usize(GTK_WIDGET(entry->W),100,24);
#endif

  gtk_entry_set_max_length(GTK_ENTRY(entry->W),13);
  gtk_table_attach (table, entry->W, x+1, x+2, y, y+1,
		    0,0,0,0);
  gtk_widget_show (entry->W);

  gtk_signal_connect(GTK_OBJECT(entry->W), "activate",
		     GTK_SIGNAL_FUNC(entry_regCB),
		     entry);

  entry->type = type;
  entry->mask = mask;
  entry->reg = user_data;

  return;

}



static void set_reg_tablePC(struct _entry_nfo *entry, const gchar *label_name,
			    int x, int y, GtkTable *table, 
			    struct _Registers *Registers, 
			    Reg_Type type, guint64 mask)
{
  GtkWidget *label;

  label = gtk_label_new(label_name);
  gtk_table_attach (table, label, x, x+1, y, y+1,
		    0,0,3,0);
  gtk_widget_show(label);

  entry->W = gtk_entry_new();
		     
#ifdef GTK2
  gtk_entry_set_width_chars(GTK_ENTRY(entry->W),13);
#else
  gtk_widget_set_usize(GTK_WIDGET(entry->W),100,24);
#endif

  gtk_entry_set_max_length(GTK_ENTRY(entry->W),25);
  gtk_table_attach (table, entry->W, x+1, x+2, y, y+1,
		    0,0,0,0);
  gtk_widget_show (entry->W);

  gtk_signal_connect(GTK_OBJECT(entry->W), "activate",
		     GTK_SIGNAL_FUNC(entry_reg_PC_CB),
		     entry);

  entry->type = type;
  entry->mask = mask;
  entry->reg = Registers;

  return;

}


void create_register_window(struct _Registers *Registers)
{
  GtkWidget *table;
  extern GtkAccelGroup *gDSP_keyboard_accel;

  if ( registerW )
    return;

  Reg=Registers;
  // Create window
  registerW = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_signal_connect(GTK_OBJECT(registerW),"destroy",
		     (GtkSignalFunc)destroy_window_CB,NULL);

  // Attach keyboard accelerations from main window
  gtk_window_add_accel_group (GTK_WINDOW (registerW), gDSP_keyboard_accel);
  
  gtk_widget_set_name (registerW, "Registers");
  gtk_window_set_title (GTK_WINDOW (registerW),"Registers");
  gtk_container_set_border_width (GTK_CONTAINER (registerW), 0);

  table = gtk_table_new(17,7,FALSE);
  gtk_widget_show(table);
  gtk_container_add (GTK_CONTAINER (registerW), table);
  

  set_reg_tablePC(&reg_entries.PC, "PC", 0, 0, GTK_TABLE(table), 
				  Registers, WORD_TYPE, 0xffff);
  set_reg_table(&reg_entries.A, "A", 0, 1, GTK_TABLE(table), 
				  &MMR->A, GP_REG_TYPE, 0xffffffffff);
  set_reg_table(&reg_entries.B, "B", 0, 2, GTK_TABLE(table), 
				  &MMR->B, GP_REG_TYPE, 0xffffffffff);
  set_reg_table(&reg_entries.SP, "SP", 0, 3, GTK_TABLE(table), 
				  &MMR->SP, WORD_TYPE, 0xffff);
  set_reg_table(&reg_entries.ST0, "ST0", 0, 4, GTK_TABLE(table), 
				  &MMR->ST0, WORD_TYPE, 0xffff);
  set_reg_table(&reg_entries.ST1, "ST1", 0, 5, GTK_TABLE(table), 
				  &MMR->ST1, WORD_TYPE, 0xffff);
  set_reg_table(&reg_entries.T, "T", 0, 6, GTK_TABLE(table), 
				  &MMR->T, WORD_TYPE, 0xffff);
  set_reg_table(&reg_entries.RC, "RC", 0, 7, GTK_TABLE(table), 
				  &Registers->RC, WORD_TYPE, 0xffff);
  set_reg_table(&reg_entries.IMR, "IMR", 0, 8, GTK_TABLE(table), 
				  &MMR->IMR, WORD_TYPE, 0xffff);
  set_reg_table(&reg_entries.IFR, "IFR", 0, 9, GTK_TABLE(table), 
				  &MMR->IFR, WORD_TYPE, 0xffff);
  set_reg_table(&reg_entries.TRN, "TRN", 0, 10, GTK_TABLE(table), 
				  &MMR->TRN, WORD_TYPE, 0xffff);


  set_reg_table(&reg_entries.AR0, "AR0", 2, 0, GTK_TABLE(table), 
				  &MMR->ar0, WORD_TYPE, 0xffff);
  set_reg_table(&reg_entries.AR1, "AR1", 2, 1, GTK_TABLE(table), 
				  &MMR->ar1, WORD_TYPE, 0xffff);
  set_reg_table(&reg_entries.AR2, "AR2", 2, 2, GTK_TABLE(table), 
				  &MMR->ar2, WORD_TYPE, 0xffff);
  set_reg_table(&reg_entries.AR3, "AR3", 2, 3, GTK_TABLE(table), 
				  &MMR->ar3, WORD_TYPE, 0xffff);
  set_reg_table(&reg_entries.AR4, "AR4", 2, 4, GTK_TABLE(table), 
				  &MMR->ar4, WORD_TYPE, 0xffff);
  set_reg_table(&reg_entries.AR5, "AR5", 2, 5, GTK_TABLE(table), 
				  &MMR->ar5, WORD_TYPE, 0xffff);
  set_reg_table(&reg_entries.AR6, "AR6", 2, 6, GTK_TABLE(table), 
				  &MMR->ar6, WORD_TYPE, 0xffff);
  set_reg_table(&reg_entries.AR7, "AR7", 2, 7, GTK_TABLE(table), 
				  &MMR->ar7, WORD_TYPE, 0xffff);

  set_reg_table(&reg_entries.PAB, "PAB", 4, 0, GTK_TABLE(table), 
				  &Registers->PAB, WORD_TYPE, 0xffff);
  set_reg_table(&reg_entries.IR, "IR",  4, 1, GTK_TABLE(table), 
				  &Registers->IR, WORD_TYPE, 0xffff);
  set_reg_table(&reg_entries.DAB, "DAB", 4, 2, GTK_TABLE(table), 
		&Registers->DAB, WORD_TYPE, 0xffff);
  set_reg_table(&reg_entries.CAB, "CAB", 4, 3, GTK_TABLE(table), 
		&Registers->CAB, WORD_TYPE, 0xffff);
  set_reg_table(&reg_entries.DB, "DB",  4, 4, GTK_TABLE(table), 
		&Registers->DB, WORD_TYPE, 0xffff);
  set_reg_table(&reg_entries.CB, "CB",  4, 5, GTK_TABLE(table), 
		&Registers->CB, WORD_TYPE, 0xffff);
  set_reg_table(&reg_entries.EAB, "EAB",  4, 6, GTK_TABLE(table), 
		&Registers->EAB, WORD_TYPE, 0xffff);
  set_reg_table(&reg_entries.BRC, "BRC",  4, 7, GTK_TABLE(table), 
		&MMR->BRC, WORD_TYPE, 0xffff);
  set_reg_table(&reg_entries.RSA, "RSA",  4, 8, GTK_TABLE(table), 
		&MMR->RSA, WORD_TYPE, 0xffff);
  set_reg_table(&reg_entries.REA, "REA",  4, 9, GTK_TABLE(table), 
		&MMR->REA, WORD_TYPE, 0xffff);
  set_reg_table(&reg_entries.PMST, "PMST",  4, 10, GTK_TABLE(table), 
		&MMR->PMST, WORD_TYPE, 0xffff);
  set_reg_table(&reg_entries.XPC, "XPC",  4, 11, GTK_TABLE(table), 
		&MMR->XPC, WORD_TYPE, 0xffff);
  

  set_reg_table(&reg_entries.C, "C",  6, 0, GTK_TABLE(table), 
		&MMR->ST0, WORD_TYPE, 0x0800);
  
  set_reg_table(&reg_entries.OVA, "OVA",  6, 1, GTK_TABLE(table), 
		&MMR->ST0, WORD_TYPE, 0x0400);
  
  set_reg_table(&reg_entries.OVB, "OVB",  6, 2, GTK_TABLE(table), 
		&MMR->ST0, WORD_TYPE, 0x0200);
  
  set_reg_table(&reg_entries.FRCT, "FRCT",  6, 3, GTK_TABLE(table), 
		&MMR->ST1, WORD_TYPE, 0x40);
  
  set_reg_table(&reg_entries.SXM, "SXM",  6, 4, GTK_TABLE(table), 
		&MMR->ST1, WORD_TYPE, 0x100);
  
  set_reg_table(&reg_entries.ARP, "ARP",  6, 5, GTK_TABLE(table), 
		&MMR->ST0, WORD_TYPE, 0xe000);
  
  set_reg_table(&reg_entries.TC, "TC",  6, 6, GTK_TABLE(table), 
		&MMR->ST0, WORD_TYPE, 0x1000);
  
  set_reg_table(&reg_entries.DP, "DP",  6, 7, GTK_TABLE(table), 
		&MMR->ST0, WORD_TYPE, 0x1ff);
  
  set_reg_table(&reg_entries.BRAF, "BRAF",  6, 8, GTK_TABLE(table), 
		&MMR->ST1, WORD_TYPE, 0x8000);
  
  set_reg_table(&reg_entries.CPL, "CPL",  6, 9, GTK_TABLE(table), 
		&MMR->ST1, WORD_TYPE, 0x4000);
  
  set_reg_table(&reg_entries.XF, "XF",  6, 10, GTK_TABLE(table), 
		&MMR->ST1, WORD_TYPE, 0x2000);
  
  set_reg_table(&reg_entries.HM, "HM",  6, 11, GTK_TABLE(table), 
		&MMR->ST1, WORD_TYPE, 0x1000);
  
  set_reg_table(&reg_entries.INTM, "INTM",  6, 12, GTK_TABLE(table), 
		&MMR->ST1, WORD_TYPE, 0x800);
  
  set_reg_table(&reg_entries.OVM, "OVM",  6, 13, GTK_TABLE(table), 
		&MMR->ST1, WORD_TYPE, 0x200);
  
  set_reg_table(&reg_entries.C16, "C16",  6, 14, GTK_TABLE(table), 
		&MMR->ST1, WORD_TYPE, 0x80);
  
  set_reg_table(&reg_entries.CMPT, "CMPT",  6, 15, GTK_TABLE(table), 
		&MMR->ST1, WORD_TYPE, 0x20);
  
  set_reg_table(&reg_entries.ASM, "ASM",  6, 16, GTK_TABLE(table), 
		&MMR->ST1, WORD_TYPE, 0x1f);
  
  fill_reg_entries(Registers);

  gtk_widget_show(registerW);
  return;

}

static void set_gp_reg(GP_Reg gp_reg, GtkWidget *W)
{
  gchar temp_str[15];
  gchar *current_text;

  g_snprintf(temp_str,15,"0x%2.2x%2.2x%2.2x%2.2x%2.2x",gp_reg.byte4,gp_reg.byte3,gp_reg.byte2,gp_reg.byte1,gp_reg.byte0);
  current_text = gtk_entry_get_text(GTK_ENTRY(W));
  if ( strcmp(current_text,temp_str)==0 )
    gtk_widget_set_style(W, styleN);
  else
    gtk_widget_set_style(W, styleH);

  gtk_entry_set_text (GTK_ENTRY(W),temp_str);
}

static void set_word(Word word, GtkWidget *W)
{
  gchar temp_str[15];
  gchar *current_text;

  g_snprintf(temp_str,15,"0x%x",word);
  current_text = gtk_entry_get_text(GTK_ENTRY(W));
  if ( strcmp(current_text,temp_str)==0 )
    gtk_widget_set_style(W, styleN);
  else
    gtk_widget_set_style(W, styleH);

  gtk_entry_set_text (GTK_ENTRY(W),temp_str);
}

static void set_bits(gchar *new_text, GtkWidget *W)
{
  gchar *current_text;

  current_text = gtk_entry_get_text(GTK_ENTRY(W));
  if ( strcmp(current_text,new_text)==0 )
    gtk_widget_set_style(W, styleN);
  else
    gtk_widget_set_style(W, styleH);

  gtk_entry_set_text (GTK_ENTRY(W),new_text);
}



void fill_reg_entries(struct _Registers *Registers)
{
  gchar temp_str[15];

  g_return_if_fail(Registers != NULL);

  // Make sure register window exists
  if ( registerW==NULL )
    return;


  if ( styleN==NULL || styleH==NULL )
    {
      GdkColor c;
    
      styleN = gtk_style_copy (gtk_widget_get_style (reg_entries.PC.W));
      styleH = gtk_style_copy (gtk_widget_get_style (reg_entries.PC.W));

      // Change font, want something that is fixed width
      gdk_font_unref (styleN->font);
      gdk_font_unref (styleH->font);
      //styleN->font = gdk_font_load
      //("-adobe-courier-medium-r-normal-*-*-120-*-*-m-*-*");
      //styleH->font = gdk_font_load
      //("-adobe-courier-medium-r-normal-*-*-120-*-*-m-*-*");
      styleN->font = gdk_font_load
	("-*-courier-medium-r-normal-*-*-120-*-*-m-*-*");
      styleH->font = gdk_font_load
	("-*-courier-medium-r-normal-*-*-120-*-*-m-*-*");

      //styleN->font = NULL;
      //printf("font =0x%x\n",styleN->font);

      c.red = 0xffff;
      c.green = 0x0;
      c.blue = 0;

      gdk_color_alloc (gtk_widget_get_colormap (reg_entries.PC.W), &c);

      styleH->text[GTK_STATE_NORMAL] = c;
      styleH->fg[GTK_STATE_NORMAL] = c;
  }

  set_word(Registers->PC,reg_entries.PC.W);

  set_gp_reg(MMR->A,reg_entries.A.W);

  set_gp_reg(MMR->B,reg_entries.B.W);

  set_word(MMR->SP,reg_entries.SP.W);
 
  set_word(MMR->ST0,reg_entries.ST0.W);
 
  set_word(MMR->ST1,reg_entries.ST1.W);
 
  set_word(MMR->T,reg_entries.T.W);
 
  set_word(Registers->RC,reg_entries.RC.W);
 
  set_word(MMR->IMR,reg_entries.IMR.W);
 
  set_word(MMR->IFR,reg_entries.IFR.W);
 
  set_word(MMR->TRN,reg_entries.TRN.W);
 
  set_word(MMR->ar0,reg_entries.AR0.W);
 
  set_word(MMR->ar1,reg_entries.AR1.W);
 
  set_word(MMR->ar2,reg_entries.AR2.W);
 
  set_word(MMR->ar3,reg_entries.AR3.W);
 
  set_word(MMR->ar4,reg_entries.AR4.W);
 
  set_word(MMR->ar5,reg_entries.AR5.W);
 
  set_word(MMR->ar6,reg_entries.AR6.W);
 
  set_word(MMR->ar7,reg_entries.AR7.W);
 
  set_word(Registers->PAB,reg_entries.PAB.W);
 
  set_word(Registers->IR,reg_entries.IR.W);
 
  set_word(Registers->DAB,reg_entries.DAB.W);
 
  set_word(Registers->CAB,reg_entries.CAB.W);
 
  set_word(Registers->DB,reg_entries.DB.W);
 
  set_word(Registers->CB,reg_entries.CB.W);
 
  set_word(Registers->EAB,reg_entries.EAB.W);
 
  set_word(MMR->BRC,reg_entries.BRC.W);
 
  set_word(MMR->RSA,reg_entries.RSA.W);
 
  set_word(MMR->REA,reg_entries.REA.W);
 
  set_word(MMR->PMST,reg_entries.PMST.W);
 
  set_word(MMR->XPC,reg_entries.XPC.W);

  g_snprintf(temp_str,15,"0x%x",C_bit(MMR));
  set_bits(temp_str,reg_entries.C.W);
 
  g_snprintf(temp_str,15,"0x%x",OVA(MMR));
  set_bits(temp_str,reg_entries.OVA.W);
 
  g_snprintf(temp_str,15,"0x%x",OVB(MMR));
  set_bits(temp_str,reg_entries.OVB.W);
 
  g_snprintf(temp_str,15,"0x%x",FRCT(MMR));
  set_bits(temp_str,reg_entries.FRCT.W);
 
  g_snprintf(temp_str,15,"0x%x",SXM(MMR));
  set_bits(temp_str,reg_entries.SXM.W);
 
  g_snprintf(temp_str,15,"0x%x",ARP(MMR));
  set_bits(temp_str,reg_entries.ARP.W);
  g_snprintf(temp_str,15,"0x%x",TC_bit(MMR));
  set_bits(temp_str,reg_entries.TC.W);
 
  g_snprintf(temp_str,15,"0x%x",DP(MMR));
  set_bits(temp_str,reg_entries.DP.W);
 
  g_snprintf(temp_str,15,"0x%x",BRAF(MMR));
  set_bits(temp_str,reg_entries.BRAF.W);
 
  g_snprintf(temp_str,15,"0x%x",CPL(MMR));
  set_bits(temp_str,reg_entries.CPL.W);
 
  g_snprintf(temp_str,15,"0x%x",XF(MMR));
  set_bits(temp_str,reg_entries.XF.W);
 
  g_snprintf(temp_str,15,"0x%x",HM(MMR));
  set_bits(temp_str,reg_entries.HM.W);
 
  g_snprintf(temp_str,15,"0x%x",INTM(MMR));
  set_bits(temp_str,reg_entries.INTM.W);
 
  g_snprintf(temp_str,15,"0x%x",OVM(MMR));
  set_bits(temp_str,reg_entries.OVM.W);
 
  g_snprintf(temp_str,15,"0x%x",C16(MMR));
  set_bits(temp_str,reg_entries.C16.W);
 
  g_snprintf(temp_str,15,"0x%x",CMPT(MMR));
  set_bits(temp_str,reg_entries.CMPT.W);
 
  g_snprintf(temp_str,15,"0x%x",ASM(MMR));
  set_bits(temp_str,reg_entries.ASM.W);
  
}
