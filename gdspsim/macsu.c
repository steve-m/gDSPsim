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
#include "xymem.h"
#include "multiplier.h"

static GPtrArray *machine_code(gchar *opcode_text);
static void execute(struct _PipeLine *pipeP, struct _Registers *Reg);

static gchar *mask[]=    { "1010011s xxxxyyyy" };
static gchar *opcode[] = { "MACSU x,y,s" };
static gchar *comment[]= { "$(s)=unsigned($(x))*$(y)+$(s); T=unsigned($(x))" };

/* This definition is global because another routine will make have
 * an array that points to all the different instruction classes.
 */
Instruction_Class MACSU_Obj =
{
  "MACSU",
  NULL, // prefetch
  NULL, // fetch
  NULL, // decode
  xymem_read_stg1, // read_stg1 (access)
  xymem_read_stg2, // read_stg2 (read)
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
  int AorB;

  AorB = (pipeP->current_opcode & 0x10000 ) >> 16;

  // X operand is unsigned DB (3)
  // Y operand is signed CB (3)
  // Accumulater using A or B (1 or 2)
  // Store in A or B (0,1)
  multiplier(3,3,AorB+1,AorB,Reg);

  // Move data from X operand to T register.
  MMR->T = Reg->DB;

}


/* Generates an array of Words that this opcode text generates or NULL
 * if it doesn't. There should not be any comments in the string
 */
static GPtrArray *machine_code(gchar *opcode_text)
{
  return NULL;
}


