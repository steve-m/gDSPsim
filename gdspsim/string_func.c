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

#include <string.h>
#include "string_func.h"

// looked like sometimes g_strconcat() didn't NULL terminate the concation when it moved.
gchar *g_strcat(gchar *dest, gchar *src)
{
  gchar *new_string,*str_cntr;
  int dlen,slen;
  
  if ( src )
    slen = strlen(src);
  else
    return dest; // nothing to do.

  if ( dest )
    {
      dlen = strlen(dest);

      new_string = g_realloc(dest,dlen+slen+1);
      str_cntr = new_string + dlen;
    }
  else
    {
      //  len = 0;
      new_string = g_new(gchar,slen+1);
      str_cntr = new_string;
    }

  // Copy src string.
  while ( *src )
    {
      *str_cntr++ = *src++;
    }
  *str_cntr = '\0';

  return new_string;
}


// gchar equivalent of strncat, the glib g_strlcat might do it.
gchar *g_strncat(gchar *dest, const gchar *src, size_t n)
{
  gchar *new_string,*str_cntr;
  int k,len;

  if ( dest )
    {
      len = strlen(dest);
      new_string = g_realloc(dest,len+n+1);
      str_cntr = new_string + len;
    }
  else
    {
      len = 0;
      new_string = g_new(gchar,n+1);
      str_cntr = new_string;
    }

  for (k=len;k<len+n;k++)
    {
      *str_cntr++ = *src++;
    }
  *str_cntr = '\0';
  return new_string;
}

/* Like the strstr function except this looks for a needle of type char
 * instead of type char * inside of string haystack. */
gchar *g_strchar(gchar *haystack, const char needle)
{
  while ( *haystack )
    {
      if ( *haystack == needle )
	return haystack;
     haystack++;
    }
  return NULL;
}
