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
#include "memory.h"

static void decode(struct _PipeLine *pipeP, struct _Registers *Reg);
static void read_stg1(struct _PipeLine *pipeP, struct _Registers *Reg);
static void read_stg2(struct _PipeLine *pipeP, struct _Registers *Reg);
static void execute(struct _PipeLine *pipeP, struct _Registers *Reg);
static GPtrArray *machine_code(gchar *opcode_text);

static gchar *mask[]=    { "11110100 110uuuuu" };
static gchar *opcode[] = { "TRAP u" };
static gchar *comment[]= { "Interrupt $(u)" };

/* This definition is global because another routine will make have
 * an array that points to all the different instruction classes.
 */
Instruction_Class TRAP_Obj =
{
  "TRAP",
  NULL, // prefetch
  NULL, // fetch
  decode, // decode
  read_stg1, // read_stg1 (access)
  read_stg2, // read_stg2 (read)
  execute, // execute
  NULL, // number_words 
  NULL, // set_cycle_number
  1,
  mask,
  opcode,
  comment,
  machine_code
};

static void decode(struct _PipeLine *pipeP, struct _Registers *Reg)
{
  Reg->RTN = Reg->PC;
}

static void read_stg1(struct _PipeLine *pipeP, struct _Registers *Reg)
{
  Reg->Flush = Reg->Flush + 2;
  Reg->Dont_Decode = 2;
}

static void read_stg2(struct _PipeLine *pipeP, struct _Registers *Reg)
{
  MMR->SP--;
  Reg->EAB = MMR->SP;
}

static void execute(struct _PipeLine *pipeP, struct _Registers *Reg)
{
  int k;
  WordA iptr;

  write_data_mem(Reg->EAB,Reg->RTN);

  k = pipeP->current_opcode & 0x1f;
  iptr = MMR->PMST & 0xff80;

  Reg->PC = k*4 + iptr;
}

/* Generates an array of Words that this opcode text generates or NULL
 * if it doesn't. There should not be any comments in the string
 */
static GPtrArray *machine_code(gchar *opcode_text)
{
  return NULL;
}


