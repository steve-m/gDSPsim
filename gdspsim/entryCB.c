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
#include "chip_core.h"
#include "entryCB.h"
#include <ctype.h>
#include "symbols.h"

// Gets address in hex,num,or text form. If text gets numerical address
void entry_addressCB( GtkWidget *widget, struct _entryCB_nfo *entryCB_nfo )
{
  // FIXME
  entry_hexCB(widget,entryCB_nfo);
}

// routine to process an entry callback
void entry_hexCB( GtkWidget *widget, struct _entryCB_nfo *entryCB_nfo )
{
  gchar *entry_text,*textP;
  guint64 hex_value;

  // entry_text = gtk_entry_get_text(GTK_ENTRY(entryCB_nfo));
  entry_text = gtk_editable_get_chars(GTK_EDITABLE(entryCB_nfo->entry),0,-1);
  g_strdown(entry_text); // convert to lower case
  g_strchomp(entry_text); // remove white space

  textP = entry_text;
  if ( *textP++ == '0' )
    {
      if ( *textP == 'X' || *textP == 'x' )
	{
	  //	  // now remove all non hex numbers
	  //	  textP++;
	  //	  while ( *textP )
	    {
	      sscanf(entry_text,"0x%llx",&hex_value);
	    }
	}
  
    }
  printf("Entry contents: %s %lld 0x%llx\n", entry_text,hex_value,hex_value);
  if ( entryCB_nfo->CB_func )
    entryCB_nfo->CB_func( entryCB_nfo->entry, hex_value, entryCB_nfo->data);

  g_free(entry_text);
}
void entry_wordCB( GtkWidget *widget, Word *reg )
{
  gchar *entry_text,*textP;
  int value;

  entry_text = gtk_editable_get_chars(GTK_EDITABLE(widget),0,-1);
  textP = entry_text;
  if ( *textP++ == '0' )
    {
      if ( *textP == 'X' || *textP == 'x' )
	{
	  sscanf(entry_text,"0x%x",&value);
	}
      else
	{
	  sscanf(entry_text,"%d",&value);
	}
    }
  else
    {
      sscanf(entry_text,"%d",&value);
    }

  *reg=value;

  printf("Entry contents: %s %d\n", entry_text,value);
  g_free(entry_text);

}

void entry_word_maskCB( GtkWidget *widget, Word *reg, Word mask )
{
  gchar *entry_text,*textP;
  int value;
  Word mask_shift;

  entry_text = gtk_editable_get_chars(GTK_EDITABLE(widget),0,-1);
  textP = entry_text;
  if ( *textP++ == '0' )
    {
      if ( *textP == 'X' || *textP == 'x' )
	{
	  sscanf(entry_text,"0x%x",&value);
	}
      else
	{
	  sscanf(entry_text,"%d",&value);
	}
    }
  else
    {
      sscanf(entry_text,"%d",&value);
    }

  mask_shift = mask;
  if ( mask_shift )
    {
      while ( (mask_shift & 1) == 0 )
	{
	  mask_shift=mask_shift>>1;
	  value=value<<1;
	}
    }
  value = value & mask;
  *reg=(*reg & ~mask) | value;

  g_free(entry_text);
}

void entry_gpreg_maskCB( GtkWidget *widget, GP_Reg *reg, guint64 mask )
{
  gchar *entry_text,*textP;
  union _GP_Reg_Union value;
  guint64 mask_shift;

  entry_text = gtk_editable_get_chars(GTK_EDITABLE(widget),0,-1);
  textP = entry_text;
  if ( *textP++ == '0' )
    {
      if ( *textP == 'X' || *textP == 'x' )
	{
	  sscanf(entry_text,"0x%llx",&value.guint64);
	}
      else
	{
	  sscanf(entry_text,"%lld",&value.guint64);
	}
    }
  else
    {
      sscanf(entry_text,"%lld",&value.gint64);
    }

  mask_shift = mask;
  if ( mask_shift )
    {
      while ( (mask_shift & 1) == 0 )
	{
	  mask_shift=mask_shift>>1;
	  value.guint64=value.guint64<<1;
	}
    }
  value.guint64 = value.guint64 & mask;
  *reg=value.gp_reg;

  g_free(entry_text);
}

gboolean text_to_address(const gchar *ch, WordA *address)
{
  gchar const *textP;
  int num;

  textP = ch;

  // remove blank space in front
  while ( *textP && isspace(*textP) )
    textP++;

  if ( *textP == '\0' )
    return FALSE;

  if ( isdigit(textP[0]) )
    {
      // It's a number
      if ( (textP[0] == '0') && 
	   ( (textP[1] == 'x') || (textP[1] == 'X')) )
	{
	  num = sscanf(textP,"0x%x",address);
	  if (num == 1)
	    return TRUE;
	  else
	    {
	      printf("bad data %s:%d\n",__FILE__,__LINE__);
	      return FALSE;
	    }
	}
      else
	{
	  num = sscanf(textP,"%d",address);
	  if (num>0)
	    return TRUE;
	  else
	    {
	      printf("bad data %s:%d\n",__FILE__,__LINE__);
	      return FALSE;
	    }
	}
    }
  else
    {
      // It's a string
      return get_address_from_symbol(address,textP);
    }
  //return FALSE;
}

gboolean text_to_int(const gchar *ch, int *address)
{
  gchar const *textP;
  int num;

  textP = ch;

  // remove blank space in front
  while ( *textP && isspace(*textP) )
    textP++;

  if ( *textP == '\0' )
    return FALSE;

  if ( isdigit(textP[0]) )
    {
      // It's a number
      if ( (textP[0] == '0') && 
	   ( (textP[1] == 'x') || (textP[1] == 'X')) )
	{
	  num = sscanf(textP,"0x%x",address);
	  if (num == 1)
	    return TRUE;
	  else
	    {
	      printf("bad data %s:%d\n",__FILE__,__LINE__);
	      return FALSE;
	    }
	}
      else
	{
	  num = sscanf(textP,"%d",address);
	  if (num>0)
	    return TRUE;
	  else
	    {
	      printf("bad data %s:%d\n",__FILE__,__LINE__);
	      return FALSE;
	    }
	}
    }
  else
    {
      return FALSE;
    }
}

gboolean word_from_file(FILE *file, Word *value)
{
  int c;
  int type=0; // 0=int 1=0x0 2=0h
  int len=0;
  unsigned char ch;
  unsigned char str[16];
  unsigned char *strP;
  int start_hex,num;

  while ( len < 15 )
    {
      c=fgetc(file);
      if ( c == EOF )
	{
	  return FALSE;
	}
      else
	{
	  ch = (unsigned char)c;
	  if ( isdigit(ch) )
	    {
	      *strP++=ch;
	      len++;
	      if ( start_hex==1 )
		start_hex=0;
	    }
	  else if ( ( (ch >= 'a') && (ch <= 'f') ) ||
		    ( (ch >= 'A') && (ch <= 'F') ) )
	    {
	      // posible hexidecimal number
	      if ( type != 1 )
		type=2;
	      *strP++=ch;
	      len++;
	      if ( start_hex==1 )
		start_hex=0;
	    }
	  else if ( (len > 0) && (start_hex==0) )
	    {
	      if ( (isspace(ch)) || (ch==',') || (ch=='\n') )
		{
		  *strP='\0';
		  if ( type == 1 )
		    num = sscanf(str,"0x%x",value);
		  else
		num = sscanf(str,"%d",value);
		  if (num>0)
		    return TRUE;
		  else
		    return FALSE;
		}
	      else if ( (ch=='x') || (ch=='X') )
		{
		  if ( (*strP == '0' ) && (len==1) )
		    {
		      start_hex=1;
		      *strP++ = ch;
		      len=2;
		      type=1;
		    }
		  else
		    {
		      return FALSE;
		    }
		}
	      else if ( (ch=='h') || (ch=='H') )
		{
		  if ( type == 1 )
		    return FALSE;
		  num = sscanf(str,"%x",value);
		  if (num>0)
		    return TRUE;
		  else
		    return FALSE;
		}
	    }
	  else
	    {
	      if ( (!isspace(ch)) && (ch!=',') )
		{
		  return FALSE;
		}
	    }
	}
    }
  return FALSE;
}



