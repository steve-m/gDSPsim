/*
 * gDSPsim - GNU Digital Signal Processor Simulator
 *
 * Copyright (C) 2002, Kerry Keal, kerry@industrialmusic.com
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
      
#include <chip_core.h>
#include <memory.h>
#include <chip_help.h>

static gchar *mask[]=
{
  "0100100p vvvvv100", // ret
};

static gchar *opcode[] = 
{ 
  "'RET'",
};

static void read_stg(struct _PipeLine *pipeP, struct _Registers *Reg);

Instruction_Class RET_Obj =
{
  "RET",
  NULL, // decode
  NULL, // address_stg
  NULL, // access_1
  NULL, // access_2
  read_stg, // read_stg
  NULL, // execute
  NULL, // write 
  NULL, // write_plus
  1,
  mask,
  opcode,
};

static void read_stg(struct _PipeLine *pipeP, struct _Registers *Reg)
{
  Word p1,p2;
  WordP addr;
  int wait_state;

  p1 = read_data_mem(MMR->SP,&wait_state);
  p2 = read_data_mem(MMR->SSP,&wait_state);
  MMR->SP++;
  MMR->SSP++;

  addr = (((WordP)p2)<<16) | p1;
  Reg->PC = addr;
  pipeP->flags = pipeP->flags | PIPE_PC_CHANGED;
}
