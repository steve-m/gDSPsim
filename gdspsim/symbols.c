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

#include "symbols.h"

GList *symbolL=NULL;
GList *symbol_label=NULL;

/* Returns a symbol if it exists, must NOT be freed by caller */
gchar *get_symbol(WordA address)
{
  GList *list;
  struct _symL *symL;
  gchar *ch;

  for (list=symbol_label;list;list=list->next)
    {
      symL = list->data;

      if ( symL->value == address )
	{
	  ch = g_strdup(symL->name);
	  return ch;
	}
    }
  return NULL;
}

gboolean get_address_from_symbol(WordA *address, gchar const *symbol)
{
  GList *list;
  struct _symL *symL;

  for (list=symbol_label;list;list=list->next)
    {
      symL = list->data;

      if ( strcmp(symL->name,symbol) == 0 )
	{
	  *address = (WordA)symL->value;
	  return TRUE;
	}
    }
  return FALSE;
}
  
