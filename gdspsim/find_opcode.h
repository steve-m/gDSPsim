/*
 * gDSPsim - GNU Digital Signal Processor Simulator
 *
 * Copyright (C) 2001-2002, Kerry Keal, kerry@industrialmusic.com
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

#ifndef __FIND_OPCODE_H__
#define __FIND_OPCODE_H__

#include "chip_core.h"
#include <decode.h>


struct _decode_opcode
{
  gchar *address; 
  gchar *machine_code; // if NULL then opcode_text is a symbol
  gchar *opcode_text;
};

#ifdef __cplusplus
extern "C" {
#endif

// Converts the machine code to it's text representation. Called after
// find_object()
void mach_code_to_text( struct _decoded_opcode *decode_nfo, 
                        struct _decode_opcode *op );


// Sets class,sub_type,length,mach_code1,mach_code2 of decode_nfo. 
// Returns 0 for OK, 1 for Error. Assumes mach_code0 and address have
// been set.
int find_object( struct _decoded_opcode *decode_nfo);

/* Returns an array of strings of decoded opcodes */
void decoded_opcodes(GPtrArray *textA,WordP start,WordP end, GArray *word2line);

#ifdef __cplusplus
}
#endif

#endif // __FIND_OPCODE_H__
