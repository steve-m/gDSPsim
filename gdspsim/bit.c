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
#include "hardware.h"
#include <stdio.h>
#include "smem.h"
#include "instruct_help.h"
#include "memory.h"
#include "xymem.h"
#include "shifter.h"
#include "decode.h"

static void execute(struct _PipeLine *pipeP, struct _Registers *Reg);
static GPtrArray *machine_code(gchar *opcode_text);

static gchar *mask[]=    { "10010110 xxxxuuuu" };
static gchar *opcode[] = { "BIT x,u" };
static gchar *comment[]= { "TC=BIT $(u) or $(x)" };

/* This definition is global because another routine will make have
 * an array that points to all the different instruction classes.
 */
Instruction_Class BIT_Obj =
{
  "BIT",
  NULL, // prefetch
  NULL, // fetch
  NULL, // decode
  xmem_read_stg1, // read_stg1 (access)
  xmem_read_stg2, // read_stg2 (read)
  execute, // execute
  NULL, // number_words 
  NULL, // set_cycle_number
  1,
  mask,
  opcode,
  comment,
  machine_code
};

static void execute(struct _PipeLine *pipeP, struct _Registers *Reg)
{
  int bit;

  bit = 0xf - (pipeP->current_opcode & 0xf);

  if ( Reg->DB & ( 1 << bit ) )
    MMR->ST0 = MMR->ST0 | 0x1000;
  else
    MMR->ST0 = MMR->ST0 & 0xefff;

}

/* Generates an array of Words that this opcode text generates or NULL
 * if it doesn't. There should not be any comments in the string
 */
static GPtrArray *machine_code(gchar *opcode_text)
{
  return NULL;
}

