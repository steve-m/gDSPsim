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

#include "chip_core.h"

#ifndef __SYMBOL_H__
#define __SYMBOL_H__

struct _symL
{
  gchar *name;
  gint32 value;
  gint16 type;
  unsigned char numaux;
  unsigned char class;
  gint16 section_num;
};

#ifdef __cplusplus
extern "C" {
#endif

/* Returns a symbol if it exists, MUST be freed by caller */
gchar *get_symbol(WordA address);

gboolean get_address_from_symbol(WordA *address, gchar const *symbol);

#ifdef __cplusplus
}
#endif

#endif // __SYMBOL_H__
