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


  printf("Time: %s\n",ctime(&header->f_timdat));

  // get section info
  g_return_val_if_fail(size > header->f_opthdr + sizeof(struct _coff_header)+2,NULL);

  char_header =  buffer + header->f_opthdr + sizeof(struct _coff_header)+2;

  section_header = (struct _section_header *)char_header;

  s_header = section_header;
  for (k=0;k<header->f_nscns;k++)
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
/*
(gdb) p *s_header
$2 = {s_name = ".text\000\000", s_paddr = 128, s_vaddr = 128, s_size = 5519, 
  s_scnptr = 7832, s_relptr = 0, s_lnnoptr = 25158, s_nreloc = 0, s_nlnno = 0, 
  s_flags = 647, bogus = "`\000\000\000\000\000\000"}

$3 = {s_name = {0x2e, 0x74, 0x65, 0x78, 0x74, 0x0, 0x0, 0x0}, s_paddr = 0x80, 
  s_vaddr = 0x80, s_size = 0x158f, s_scnptr = 0x1e98, s_relptr = 0x0, 
  s_lnnoptr = 0x6246, s_nreloc = 0x0, s_nlnno = 0x0, s_flags = 0x287, bogus = {
    0x60, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0}}
*/
