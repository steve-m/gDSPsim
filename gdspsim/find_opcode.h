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

#ifndef __FIND_OPCODE_H__
#define __FIND_OPCODE_H__

#include "c54_core.h"

// Converts the machine code to it's text representation. Reads extra words
// if needed
struct _decode_opcode *mach_code_to_text(Word mach_code, const Instruction_Class *classP, int subtype, WordA *location);

// Returns pointer to object type given opcode and sets subtype
const Instruction_Class *find_object(Word mach_code, int *subtype);

/* Returns an array of strings of decoded opcodes */
void decoded_opcodes(GPtrArray *textA,WordA start,WordA end, GArray *word2line);


struct _decode_opcode
{
  gchar *address; 
  gchar *machine_code; // if NULL then opcode_text is a symbol
  gchar *opcode_text;
};


#endif // __FIND_OPCODE_H__
