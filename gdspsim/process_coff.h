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

#define CHAR_TO_UINT16(ch)((guint16)*(guint16 *)(ch))
#define CHAR_TO_UINT32(ch)((guint32)*(guint32 *)(ch))

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
  unsigned char version_id[2]; 
  unsigned char num_sections[2];  // Number of sections
  unsigned char time_stamp[4];    // Time (seconds) from Jan 1, 1970
  unsigned char symbolP[4];       // Pointer to symbol table
  unsigned char num_symbols[4];   // Number of symbols in symbol table
  unsigned char num_bytes_opt[2]; // Number of bytes in Optional Header (either 0 or 28)
  unsigned char flags[2];         // Flags
  /*
   * Reference:  C54 Assembly Language Tools Users Guide (Appendix A)
   * F_RELFLG   0001h  Relocation information was stripped from the file.
   * F_EXEC     0002h  The file is relocatable (it contains no unresolved
   *                   external references).
   * F_LNNO     0004h  Line numbers were stripped from the file.
   * F_LSYMS    0010h  Local symbols were stripped from the file.
   * F_LENDIAN  0100h  The file has the byte ordering used by 'C54x de-
   *                   vices (16 bits per word, least significant byte first)
   * F_SYMMERGE 1000h  Duplicate symbols were removed.
   */
  char target_id[2]; // Magic number. 0x108 for SunOS and 0x801 for x86
  // Can be used to see if Big Endian (0x108) or Little Endian (0x801)

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

/* If the symbol name is 8 or less characters, then it goes here.
 * otherwise it is in the string table at location offset, in which
 * case, name_flag will = 0. */
union _symbol_name_or_location
{
  unsigned char name[8]; // if symbol name is 8 or less char, then it goes here
  struct
  {
    unsigned char name_flag[4];
    unsigned char offset[4];
  } _offset;
};


struct _symbol_element
{
  union _symbol_name_or_location norl;
  unsigned char e_value[4];
  unsigned char e_scnum[2];
  unsigned char e_type[2];
  unsigned char e_sclass;
  unsigned char e_numaux;
};

struct _optional_header
{
  unsigned char magic_num[2];  // Magic number. 0x108 for SunOS and 0x801 
  // for x86. Can be used to see if Big Endian (0x108) or Little Endian (0x801)
  
  unsigned char version[2];
  unsigned char size_executable[4]; // size (Words) of executable
  unsigned char size_initialized[4]; // size (Words) of initialized sections
  unsigned char size_uninitialized[4]; // size (Words) of uninitialized sections
  unsigned char start_address[4]; // entry location
  unsigned char start_executable[4]; // start of executable
  unsigned char start_initialized[4]; // start of initialized section
};

#endif //  __PROCESS_COFF_H__
