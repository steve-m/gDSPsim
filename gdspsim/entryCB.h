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

#ifndef __ENTRY_CB_H__
#define __ENTRY_CB_H__

#include <gtk/gtk.h>
#include "chip_core.h"

typedef void (*Entry_Hex_CB_Func)(GtkWidget *entry, guint64 num, gpointer data);

struct _entryCB_nfo
{
  GtkWidget *entry;
  int bits; // maximum allowed bits
  Entry_Hex_CB_Func CB_func;
  gchar *text; // validated text that is inside entry widget
  gpointer data; // pointer passed by to CB_func
};

// turn text into an address. returns TRUE is sucessful
gboolean text_to_address(const gchar *ch, WordA *address);
// turn text into an int. returns TRUE is sucessful
gboolean text_to_int(const gchar *ch, int *address);

gboolean word_from_file(FILE *file, Word *value);

// routine to process an entry callback
void entry_hexCB( GtkWidget *widget, struct _entryCB_nfo *entryCB_nfo );
// Gets address in hex,num,or text form. If text gets numerical address
void entry_addressCB( GtkWidget *widget, struct _entryCB_nfo *entryCB_nfo );
void entry_wordCB( GtkWidget *widget, Word *reg );
// only changes value under mask. mask assumed to have consecutive bits
void entry_word_maskCB( GtkWidget *widget, Word *reg, Word mask );
void entry_gpreg_maskCB( GtkWidget *widget, GP_Reg *reg, guint64 mask );
#endif
