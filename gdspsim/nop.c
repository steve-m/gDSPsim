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

#include "c54_core.h"
#include <stdio.h>


static GPtrArray *machine_code(gchar *opcode_text);

static gchar *mask[]= {"11110100 10010101"};
static gchar *opcode[] = {"NOP"};
static gchar *comment[]=  {""};

/* This definition is global because another routine will make have
 * an array that points to all the different instruction classes.
 */
Instruction_Class NOP_Obj =
{
  "NOP",
  NULL, // prefetch
  NULL, // fetch
  NULL, // decode
  NULL, // read_stg1 (access)
  NULL, // read_stg2 (read)
  NULL, // execute
  NULL, // number_words 
  NULL, // set_cycle_number
  1,
  mask,
  opcode,
  comment,
  machine_code
};


 // Given some opcode text, this returns an array
  // of words that it codes into.
Word NOP_opcode = 0xf495;

static GPtrArray *machine_code(gchar *opcode_text)
{
  GPtrArray *opcode;
  Word *nop;

  nop = g_new(Word,1);

  *nop = NOP_opcode;

  opcode = g_ptr_array_new();
  g_ptr_array_add(opcode,nop);

  return opcode;
}
