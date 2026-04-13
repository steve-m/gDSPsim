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
#include "chip_core.h"
#include "register_window.h"
#include <gtk/gtk.h>
#include <string.h>
#include "entryCB.h"
#include "decode.h"

typedef enum { WORD_TYPE = 0, GP_REG_TYPE = 1 } Reg_Type;

struct _entry_nfo {
  GtkWidget *W;
  guint64 mask;
  gpointer reg;
  Reg_Type type;
};

struct _reg_entries {
  struct _entry_nfo PC, AC0, AC1, AC2, AC3, SP, ST0, ST1, T, RC, IMR, IFR, TRN;
  struct _entry_nfo AR0, AR1, AR2, AR3, AR4, AR5, AR6, AR7;
  struct _entry_nfo PAB, IR, DAB, CAB, DB, CB, EAB, BRC, RSA, REA, PMST, XPC;
  struct _entry_nfo C, OVA, OVB, FRCT, SXM, OVM, ARP, TC, DP, BRAF, CPL,
                    XF, HM, INTM, C16, CMPT, ASM;
};

static struct _reg_entries reg_entries;

GtkWidget *registerW = NULL;
struct _Registers *Reg = NULL;

static GtkCssProvider *highlight_provider(void)
{
  static GtkCssProvider *prov = NULL;
  if (prov == NULL)
    {
      prov = gtk_css_provider_new();
      gtk_css_provider_load_from_data(prov,
          "entry.gdsp-changed { color: #ff0000; }", -1, NULL);
    }
  return prov;
}

static void apply_highlight(GtkWidget *w, gboolean changed)
{
  GtkStyleContext *ctx = gtk_widget_get_style_context(w);
  gtk_style_context_add_provider(ctx,
      GTK_STYLE_PROVIDER(highlight_provider()),
      GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
  if (changed)
    gtk_style_context_add_class(ctx, "gdsp-changed");
  else
    gtk_style_context_remove_class(ctx, "gdsp-changed");
}

void entry_regCB(GtkWidget *widget, struct _entry_nfo *nfo)
{
  if (nfo->type == WORD_TYPE)
    entry_word_maskCB(widget, nfo->reg, nfo->mask);
  else if (nfo->type == GP_REG_TYPE)
    entry_gpreg_maskCB(widget, nfo->reg, nfo->mask);
  fill_reg_entries(Reg);
}

static void destroy_window_CB(GtkWidget *W, gpointer data)
{
  registerW = NULL;
}

static void set_reg_table(struct _entry_nfo *entry, const gchar *label_name,
                          int x, int y, GtkGrid *grid, gpointer user_data,
                          Reg_Type type, guint64 mask)
{
  GtkWidget *label;

  label = gtk_label_new(label_name);
  gtk_grid_attach(grid, label, x, y, 1, 1);

  entry->W = gtk_entry_new();
  if (type == WORD_TYPE)
    gtk_entry_set_width_chars(GTK_ENTRY(entry->W), 7);
  else
    gtk_entry_set_width_chars(GTK_ENTRY(entry->W), 13);

  gtk_entry_set_max_length(GTK_ENTRY(entry->W), 13);
  gtk_grid_attach(grid, entry->W, x + 1, y, 1, 1);

  g_signal_connect(entry->W, "activate", G_CALLBACK(entry_regCB), entry);

  entry->type = type;
  entry->mask = mask;
  entry->reg = user_data;
}

void create_register_window(struct _Registers *Registers)
{
  GtkWidget *grid;
  GtkGrid *g;
  extern GtkAccelGroup *gDSP_keyboard_accel;

  if (registerW)
    return;

  Reg = Registers;
  registerW = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  g_signal_connect(registerW, "destroy", G_CALLBACK(destroy_window_CB), NULL);
  gtk_window_add_accel_group(GTK_WINDOW(registerW), gDSP_keyboard_accel);
  gtk_widget_set_name(registerW, "Registers");
  gtk_window_set_title(GTK_WINDOW(registerW), "Registers");
  gtk_container_set_border_width(GTK_CONTAINER(registerW), 0);

  grid = gtk_grid_new();
  g = GTK_GRID(grid);
  gtk_container_add(GTK_CONTAINER(registerW), grid);

  set_reg_table(&reg_entries.PC, "PC", 0, 0, g, &Registers->PC, WORD_TYPE, 0xffff);
  set_reg_table(&reg_entries.AC0, "AC0", 0, 1, g, &MMR->AC0, GP_REG_TYPE, 0xffffffffffULL);
  set_reg_table(&reg_entries.AC1, "AC1", 0, 2, g, &MMR->AC1, GP_REG_TYPE, 0xffffffffffULL);
  set_reg_table(&reg_entries.AC2, "AC2", 0, 3, g, &MMR->AC2, GP_REG_TYPE, 0xffffffffffULL);
  set_reg_table(&reg_entries.AC3, "AC3", 0, 4, g, &MMR->AC3, GP_REG_TYPE, 0xffffffffffULL);
  set_reg_table(&reg_entries.SP, "SP", 0, 5, g, &MMR->SP, WORD_TYPE, 0xffff);
  set_reg_table(&reg_entries.RC, "RC", 0, 7, g, &Registers->RC, WORD_TYPE, 0xffff);

  set_reg_table(&reg_entries.AR0, "AR0", 2, 0, g, &MMR->ar0, WORD_TYPE, 0xffff);
  set_reg_table(&reg_entries.AR1, "AR1", 2, 1, g, &MMR->ar1, WORD_TYPE, 0xffff);
  set_reg_table(&reg_entries.AR2, "AR2", 2, 2, g, &MMR->ar2, WORD_TYPE, 0xffff);
  set_reg_table(&reg_entries.AR3, "AR3", 2, 3, g, &MMR->ar3, WORD_TYPE, 0xffff);
  set_reg_table(&reg_entries.AR4, "AR4", 2, 4, g, &MMR->ar4, WORD_TYPE, 0xffff);
  set_reg_table(&reg_entries.AR5, "AR5", 2, 5, g, &MMR->ar5, WORD_TYPE, 0xffff);
  set_reg_table(&reg_entries.AR6, "AR6", 2, 6, g, &MMR->ar6, WORD_TYPE, 0xffff);
  set_reg_table(&reg_entries.AR7, "AR7", 2, 7, g, &MMR->ar7, WORD_TYPE, 0xffff);

  set_reg_table(&reg_entries.DAB, "DAB", 4, 2, g, &Registers->DAB, WORD_TYPE, 0xffff);
  set_reg_table(&reg_entries.CAB, "CAB", 4, 3, g, &Registers->CAB, WORD_TYPE, 0xffff);
  set_reg_table(&reg_entries.DB, "DB", 4, 4, g, &Registers->DB, WORD_TYPE, 0xffff);
  set_reg_table(&reg_entries.CB, "CB", 4, 5, g, &Registers->CB, WORD_TYPE, 0xffff);
  set_reg_table(&reg_entries.EAB, "EAB", 4, 6, g, &Registers->EAB, WORD_TYPE, 0xffff);
  set_reg_table(&reg_entries.PMST, "PMST", 4, 10, g, &MMR->PMST, WORD_TYPE, 0xffff);
  set_reg_table(&reg_entries.XPC, "XPC", 4, 11, g, &MMR->XPC, WORD_TYPE, 0xffff);

  set_reg_table(&reg_entries.FRCT, "FRCT", 6, 3, g, &MMR->ST1, WORD_TYPE, 0x40);
  set_reg_table(&reg_entries.SXM, "SXM", 6, 4, g, &MMR->ST1, WORD_TYPE, 0x100);
  set_reg_table(&reg_entries.ARP, "ARP", 6, 5, g, &MMR->ST0, WORD_TYPE, 0xe000);
  set_reg_table(&reg_entries.TC, "TC", 6, 6, g, &MMR->ST0, WORD_TYPE, 0x1000);
  set_reg_table(&reg_entries.DP, "DP", 6, 7, g, &MMR->ST0, WORD_TYPE, 0x1ff);
  set_reg_table(&reg_entries.BRAF, "BRAF", 6, 8, g, &MMR->ST1, WORD_TYPE, 0x8000);
  set_reg_table(&reg_entries.CPL, "CPL", 6, 9, g, &MMR->ST1, WORD_TYPE, 0x4000);
  set_reg_table(&reg_entries.XF, "XF", 6, 10, g, &MMR->ST1, WORD_TYPE, 0x2000);
  set_reg_table(&reg_entries.HM, "HM", 6, 11, g, &MMR->ST1, WORD_TYPE, 0x1000);
  set_reg_table(&reg_entries.INTM, "INTM", 6, 12, g, &MMR->ST1, WORD_TYPE, 0x800);
  set_reg_table(&reg_entries.OVM, "OVM", 6, 13, g, &MMR->ST1, WORD_TYPE, 0x200);
  set_reg_table(&reg_entries.C16, "C16", 6, 14, g, &MMR->ST1, WORD_TYPE, 0x80);
  set_reg_table(&reg_entries.CMPT, "CMPT", 6, 15, g, &MMR->ST1, WORD_TYPE, 0x20);
  set_reg_table(&reg_entries.ASM, "ASM", 6, 16, g, &MMR->ST1, WORD_TYPE, 0x1f);

  fill_reg_entries(Registers);

  gtk_widget_show_all(registerW);
}

static void set_gp_reg(GP_Reg gp_reg, GtkWidget *W)
{
  gchar temp_str[24];
  const gchar *current_text;
  g_snprintf(temp_str, 24, "0x%2.2x%2.2x%2.2x%2.2x%2.2x",
             gp_reg.bgp.byte4, gp_reg.bgp.byte3, gp_reg.bgp.byte2,
             gp_reg.bgp.byte1, gp_reg.bgp.byte0);
  current_text = gtk_entry_get_text(GTK_ENTRY(W));
  apply_highlight(W, strcmp(current_text, temp_str) != 0);
  gtk_entry_set_text(GTK_ENTRY(W), temp_str);
}

static void set_word(Word word, GtkWidget *W)
{
  gchar temp_str[24];
  const gchar *current_text;
  g_snprintf(temp_str, 24, "0x%x", word);
  current_text = gtk_entry_get_text(GTK_ENTRY(W));
  apply_highlight(W, strcmp(current_text, temp_str) != 0);
  gtk_entry_set_text(GTK_ENTRY(W), temp_str);
}

void fill_reg_entries(struct _Registers *Registers)
{
  g_return_if_fail(Registers != NULL);

  if (registerW == NULL)
    return;

  set_word(Registers->PC, reg_entries.PC.W);
  set_gp_reg(MMR->AC0, reg_entries.AC0.W);
  set_gp_reg(MMR->AC1, reg_entries.AC1.W);
  set_gp_reg(MMR->AC2, reg_entries.AC2.W);
  set_gp_reg(MMR->AC3, reg_entries.AC3.W);
  set_word(MMR->SP, reg_entries.SP.W);
  set_word(Registers->RC, reg_entries.RC.W);

  set_word(MMR->ar0, reg_entries.AR0.W);
  set_word(MMR->ar1, reg_entries.AR1.W);
  set_word(MMR->ar2, reg_entries.AR2.W);
  set_word(MMR->ar3, reg_entries.AR3.W);
  set_word(MMR->ar4, reg_entries.AR4.W);
  set_word(MMR->ar5, reg_entries.AR5.W);
  set_word(MMR->ar6, reg_entries.AR6.W);
  set_word(MMR->ar7, reg_entries.AR7.W);

  set_word(Registers->DAB, reg_entries.DAB.W);
  set_word(Registers->CAB, reg_entries.CAB.W);
  set_word(Registers->DB, reg_entries.DB.W);
  set_word(Registers->CB, reg_entries.CB.W);
  set_word(Registers->EAB, reg_entries.EAB.W);
}
