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

#ifndef __PROCESS_COFF_H__
#define __PROCESS_COFF_H__

#include <glib.h>
#include "c54_core.h"

struct _file_info
{
  gchar *filename;
  gchar *buffer;
  int buffer_size;
  struct _section_header *text_sec;
};

struct _file_info *process_coff(char *buffer, int size);

struct _coff_header
{
  unsigned short f_magic;  // Magic Number to specify target machine
  unsigned short f_nscns;  // Number of sections
  long int       f_timdat; // time and date stamp from Jan 1, 1970
  long int       f_symptr; // file pointer to symbol table
  long int       f_nysms;  // Number of entries in symbol table
  unsigned short f_opthdr; // Number of bytes in the optional header
  unsigned short f_flags;  // flags
  char           bogus[2]; // not in the spec, but needed for c54
};

struct _section_header
{
  char           s_name[8];   // 8-character null padded section name
  unsigned long int       s_paddr;   // Physical address of section
  unsigned long int       s_vaddr;   // Virtual address of section.  always same as above?
  unsigned long int s_size;    // Section size in bytes
  unsigned long int       s_scnptr;  // File pointer to raw data. =0 for non-initialized sections ?

  unsigned long int       s_relptr;  // File pointer to relocation entries
  unsigned long int       s_lnnoptr; // File pointer to line number entries
  unsigned short s_nreloc;  // Number of relocation  entries
  unsigned short s_nlnno;   // Number of line number entries
  long int       s_flags;   // Flags. only nonzero for .text section?
  char           bogus[8];    // not in the spec, but needed for c54
};

#endif //  __PROCESS_COFF_H__
