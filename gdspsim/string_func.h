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

#include <glib.h>

// gchar equivalent of strncat
gchar *g_strncat(gchar *dest, const gchar *src, size_t n);

/* Like the strstr function except this looks for a needle of type char
 * instead of type char * inside of string haystack. */
gchar *g_strchar(gchar *haystack, const char needle);

// looked like sometimes g_strconcat() didn't NULL terminate the concation when it moved.
gchar *g_strcat(gchar *dest, gchar *src);

