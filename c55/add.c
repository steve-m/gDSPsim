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
#include <smem.h>

static gchar *mask[]=
  {
    "1110 1101 pppp ppp1 qqrr 0000", // ADD DBL(p),ACq,ACr
    "0010 0100 00rr 00qq", // ADD r,q
    "0001 0000 00rr 00qq 0011 0000", // ADD r<<#-16,q
    "0100 0000 nnnn rrrr", // add #n,r
  };
static gchar *opcode[] = 
{ 
  "ADD DBL(p),q,r",
  "ADD r,q",
  "ADD r<<#-16,q",
  "ADD #n,r",
};

static void execute(struct _PipeLine *pipeP, struct _Registers *Reg);

Instruction_Class ADD_Obj =
{
  "ADD",
  NULL, // decode
  NULL, // address_stg
  NULL, // access_1
  NULL, // access_2
  NULL, // read_stg
  execute, // execute
  NULL, // write 
  NULL, // write_plus
  4,
  mask,
  opcode,
};



static void execute(struct _PipeLine *pipeP, struct _Registers *Reg)
{
}


