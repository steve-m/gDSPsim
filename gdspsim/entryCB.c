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
#include <ctype.h>
#include "chip_core.h"
#include "entryCB.h"
#include "symbols.h"

void entry_hexCB(GtkWidget *widget, struct _entryCB_nfo *entryCB_nfo)
{
  gchar *entry_text, *textP;
  guint64 hex_value = 0;

  entry_text = gtk_editable_get_chars(GTK_EDITABLE(entryCB_nfo->entry), 0, -1);
  if (entry_text == NULL)
    return;

  /* lowercase + strip trailing whitespace */
  for (textP = entry_text; *textP; textP++)
    *textP = g_ascii_tolower(*textP);
  g_strchomp(entry_text);

  textP = entry_text;
  if (*textP == '0' && (textP[1] == 'x' || textP[1] == 'X'))
    sscanf(entry_text, "0x%" G_GINT64_MODIFIER "x", &hex_value);
  else
    sscanf(entry_text, "%" G_GINT64_MODIFIER "u", &hex_value);

  if (entryCB_nfo->CB_func)
    entryCB_nfo->CB_func(entryCB_nfo->entry, hex_value, entryCB_nfo->data);

  g_free(entry_text);
}

void entry_addressCB(GtkWidget *W, struct _entryCB_nfo *entryCB_nfo)
{
  gchar *entry_text;
  WordP hex_value;

  entry_text = gtk_editable_get_chars(GTK_EDITABLE(entryCB_nfo->entry), 0, -1);
  if (entry_text == NULL)
    return;

  if (text_to_address(entry_text, &hex_value))
    {
      if (entryCB_nfo->text)
        g_free(entryCB_nfo->text);
      entryCB_nfo->text = entry_text;
      if (entryCB_nfo->CB_func)
        entryCB_nfo->CB_func(entryCB_nfo->entry, hex_value, entryCB_nfo->data);
    }
  else
    {
      gtk_entry_set_text(GTK_ENTRY(W), entryCB_nfo->text ? entryCB_nfo->text : "");
      g_free(entry_text);
    }
}

void entry_wordCB(GtkWidget *widget, Word *reg)
{
  gchar *entry_text;
  int value = 0;

  entry_text = gtk_editable_get_chars(GTK_EDITABLE(widget), 0, -1);
  if (entry_text == NULL)
    return;

  if (entry_text[0] == '0' && (entry_text[1] == 'x' || entry_text[1] == 'X'))
    sscanf(entry_text, "0x%x", &value);
  else
    sscanf(entry_text, "%d", &value);

  *reg = value;
  g_free(entry_text);
}

void entry_word_maskCB(GtkWidget *widget, Word *reg, Word mask)
{
  gchar *entry_text;
  int value = 0;
  Word mask_shift;

  entry_text = gtk_editable_get_chars(GTK_EDITABLE(widget), 0, -1);
  if (entry_text == NULL)
    return;

  if (entry_text[0] == '0' && (entry_text[1] == 'x' || entry_text[1] == 'X'))
    sscanf(entry_text, "0x%x", &value);
  else
    sscanf(entry_text, "%d", &value);

  mask_shift = mask;
  if (mask_shift)
    {
      while ((mask_shift & 1) == 0)
        {
          mask_shift >>= 1;
          value <<= 1;
        }
    }
  value &= mask;
  *reg = (*reg & ~mask) | value;

  g_free(entry_text);
}

void entry_gpreg_maskCB(GtkWidget *widget, GP_Reg *reg, guint64 mask)
{
  gchar *entry_text;
  union _GP_Reg_Union value;
  guint64 mask_shift;

  value.guint64 = 0;
  entry_text = gtk_editable_get_chars(GTK_EDITABLE(widget), 0, -1);
  if (entry_text == NULL)
    return;

  if (entry_text[0] == '0' && (entry_text[1] == 'x' || entry_text[1] == 'X'))
    sscanf(entry_text, "0x%" G_GINT64_MODIFIER "x", &value.guint64);
  else
    sscanf(entry_text, "%" G_GINT64_MODIFIER "d", &value.gint64);

  mask_shift = mask;
  if (mask_shift)
    {
      while ((mask_shift & 1) == 0)
        {
          mask_shift >>= 1;
          value.guint64 <<= 1;
        }
    }
  value.guint64 &= mask;
  *reg = value.gp_reg;

  g_free(entry_text);
}

gboolean text_to_address(const gchar *ch, WordP *address)
{
  const gchar *textP = ch;
  int num;

  while (*textP && isspace((unsigned char)*textP))
    textP++;

  if (*textP == '\0')
    return FALSE;

  if (isdigit((unsigned char)textP[0]))
    {
      if (textP[0] == '0' && (textP[1] == 'x' || textP[1] == 'X'))
        {
          num = sscanf(textP, "0x%x", address);
          return num == 1;
        }
      num = sscanf(textP, "%d", address);
      return num > 0;
    }
  return get_address_from_symbol(address, textP);
}

gboolean text_to_int(const gchar *ch, int *address)
{
  const gchar *textP = ch;
  int num;

  while (*textP && isspace((unsigned char)*textP))
    textP++;

  if (*textP == '\0')
    return FALSE;

  if (isdigit((unsigned char)textP[0]))
    {
      if (textP[0] == '0' && (textP[1] == 'x' || textP[1] == 'X'))
        {
          num = sscanf(textP, "0x%x", address);
          return num == 1;
        }
      num = sscanf(textP, "%d", address);
      return num > 0;
    }
  return FALSE;
}

gboolean word_from_file(FILE *file, Word *value)
{
  int c;
  int type = 0;
  int len = 0;
  unsigned char ch;
  unsigned char str[16];
  unsigned char *strP = str;
  int start_hex = 0, num;

  while (len < 15)
    {
      c = fgetc(file);
      if (c == EOF)
        return FALSE;

      ch = (unsigned char)c;
      if (isdigit(ch))
        {
          *strP++ = ch;
          len++;
          if (start_hex == 1) start_hex = 0;
        }
      else if ((ch >= 'a' && ch <= 'f') || (ch >= 'A' && ch <= 'F'))
        {
          if (type != 1) type = 2;
          *strP++ = ch;
          len++;
          if (start_hex == 1) start_hex = 0;
        }
      else if ((len > 0) && (start_hex == 0))
        {
          if (isspace(ch) || ch == ',' || ch == '\n')
            {
              *strP = '\0';
              if (type == 1)
                num = sscanf((char *)str, "0x%x", value);
              else
                num = sscanf((char *)str, "%d", value);
              return num > 0;
            }
          else if (ch == 'x' || ch == 'X')
            {
              if (len == 1 && str[0] == '0')
                {
                  start_hex = 1;
                  *strP++ = ch;
                  len = 2;
                  type = 1;
                }
              else
                return FALSE;
            }
          else if (ch == 'h' || ch == 'H')
            {
              if (type == 1) return FALSE;
              num = sscanf((char *)str, "%x", value);
              return num > 0;
            }
        }
      else
        {
          if (!isspace(ch) && ch != ',')
            return FALSE;
        }
    }
  return FALSE;
}
