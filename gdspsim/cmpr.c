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

static gchar *mask[]=    { "111101uu 10101eee" };
static gchar *opcode[] = { "CMPR u,e" };
static gchar *comment[]= { "TC = AR0 $(u) $(e)" };

/* This definition is global because another routine will make have
 * an array that points to all the different instruction classes.
 */
Instruction_Class CMPR_Obj =
{
  "CMPR",
  NULL, // prefetch
  NULL, // fetch
  NULL, // decode
  NULL, // read_stg1 (access)
  NULL, // read_stg2 (read)
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
  Word arx;
  int x;

  x = pipeP->current_opcode & 7;

  switch ( x )
    {
    case 0:
      arx = MMR->ar0;
      break;
    case 1:
      arx = MMR->ar1;
      break;
    case 2:
      arx = MMR->ar2;
      break;
    case 3:
      arx = MMR->ar3;
      break;
    case 4:
      arx = MMR->ar4;
      break;
    case 5:
      arx = MMR->ar5;
      break;
    case 6:
      arx = MMR->ar6;
      break;
    case 7:
      arx = MMR->ar7;
      break;
    }
  
  x = (pipeP->current_opcode & 0x300)>>8;

  switch ( x )
    {
    case 0:
      if ( arx == MMR->ar0 )
	set_TC(MMR,1);
      else
	set_TC(MMR,0);
      return;
    case 1:
      if ( arx < MMR->ar0 )
	set_TC(MMR,1);
      else
	set_TC(MMR,0);
      return;
    case 2:
      if ( arx > MMR->ar0 )
	set_TC(MMR,1);
      else
	set_TC(MMR,0);
      return;
    case 3:
      if ( arx != MMR->ar0 )
	set_TC(MMR,1);
      else
	set_TC(MMR,0);
      return;
    }
}

/* Generates an array of Words that this opcode text generates or NULL
 * if it doesn't. There should not be any comments in the string
 */
static GPtrArray *machine_code(gchar *opcode_text)
{
  return NULL;
}

