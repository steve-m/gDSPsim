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

#include "process_coff.h"
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <glib.h>
#include "memory.h"

struct _section_header *text_sec;
long int text_size;
guint16 *text_data;

struct _file_info *process_coff(char *buffer, int size)
{
  int k;
  struct _coff_header *header;
  struct _section_header *section_header,*s_header;
  char *char_header;
  char *s_name;
  struct _file_info *nfo;

  g_return_val_if_fail(size>sizeof(struct _section_header),NULL);
  header = (struct _coff_header *)buffer;

  nfo = g_new(struct _file_info,1);
  
  nfo->filename = NULL;
  nfo->buffer = buffer;
  nfo->buffer_size = size;


  // printf("Time: %s\n",ctime((time_t *)&CHAR_TO_UINT16(header->time_stamp)));

  // get section info
  g_return_val_if_fail(size > CHAR_TO_UINT16(header->num_bytes_opt) + sizeof(struct _coff_header)+2,NULL);

  char_header =  buffer + CHAR_TO_UINT16(header->num_bytes_opt) + sizeof(struct _coff_header)+2;

  section_header = (struct _section_header *)char_header;

  s_header = section_header;
  for (k=0;k<CHAR_TO_UINT16(header->num_sections);k++)
    {
      s_name = strstr(s_header->s_name,".text");
      if ( s_name )
	{
	  nfo->text_sec = (struct _section_header *)s_name;

	  //	  text_sec = s_name;
	  // text_size = s_header->s_size;
	  // text_data = (guint16 *)(buffer + s_header->s_scnptr);
	  cp_to_mem( (Word *)(buffer + s_header->s_scnptr),
		     s_header->s_vaddr, s_header->s_size, 
		     PROGRAM_MEM_TYPE | DATA_MEM_TYPE);
	}
      s_header++;
    }
  return nfo;
}
