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
#include <xymem.h>
#include <shifter.h>
#include <memory.h>
#include <chip_help.h>
#include <alu.h>

static gchar *mask[]=
{
  "11011110 ssssssss ttRR1001", // SUBADD Tx, Smem, ACx
  "11101110 ssssssss ttRR111v", // SUBADD Tx, dual(Lmem), ACx
};

static gchar *opcode[] = 
{
  "'SUBADD' t,s,R", // SUBADD Tx, Smem, ACx
  "'SUBADD' t,'dual'(s),R", // SUBADD Tx, dual(Lmem), ACx
};

static void address_stg(struct _PipeLine *pipeP, struct _Registers *Reg);
static void read_stg(struct _PipeLine *pipeP, struct _Registers *Reg);
static void execute(struct _PipeLine *pipeP, struct _Registers *Reg);

Instruction_Class SUBADD_Obj =
{
  "SUBADD",
  NULL, // decode
  address_stg, // address_stg
  NULL, // access_1
  NULL, // access_2
  read_stg, // read_stg
  execute, // execute
  NULL, // write 
  NULL, // write_plus
  2,
  mask,
  opcode,
};

static void address_stg(struct _PipeLine *pipeP, struct _Registers *Reg)
{
  switch ( pipeP->opcode_subType )
    {
    case 0:
      smem_address_stg_b2(pipeP,Reg);
      break;
    case 1:
      lmem_address_stg_b2(pipeP,Reg);
      break;
    }
}

static void read_stg(struct _PipeLine *pipeP, struct _Registers *Reg)
{
  switch ( pipeP->opcode_subType )
    {
    case 0:
      smem_read_stg(pipeP,Reg);
    case 1:
      xymem_read_stg(pipeP,Reg);
      break;
    }
}

static void execute(struct _PipeLine *pipeP, struct _Registers *Reg)
{
  int r,R;
  Opcode opcode;

  opcode = pipeP->decode_nfo.mach_code;

  switch ( pipeP->opcode_subType )
    {
    case 0:
      // SUBADD Tx, Smem, ACx
      R = (opcode.bop[2]>>4)&0x3;
      r = ((opcode.bop[2]>>6)&0x3)+4;
      // ACx_39_16 = Reg->DB + Tx
      // ACx_15_0  = Reg->DB - Tx
      alu(r, ALU_DB, R, ALU_DUAL_SUBADD, Reg);
      return;

    case 1:
      // SUBADD Tx, dual(Lmem), ACx
      R = (opcode.bop[2]>>4)&0x3;
      r = ((opcode.bop[2]>>6)&0x3)+4;
      // ACx_39_16 = Reg->DB + Tx
      // ACx_15_0  = Reg->CB - Tx
      alu(r, ALU_DBL, R, ALU_DUAL_SUBADD, Reg);
      return;
    }
}
