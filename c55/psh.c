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
#include <smem.h>

static gchar *mask[]=
  {
    "0011100p rrrrRRRR", // PSH src1,src2
    "0101000p rrrrv110", // PSH src
    "11100100 ssssssss rrrrv0vv", // PSH src,Smem
    "0101000p vvrrv111", // PSH dbl(ACx)
    "10110101 sssssss", // PSH Smem
    "10110111 sssssss", // PSH dbl(Lmem)


  };

static gchar *opcode[] = 
{ 
  "'PSH' r,R",
  "'PSH' r",
  "'PSH' r,s",
  "'PSH' 'dbl'(r)",
  "'PSH' s",
  "'PSH' 'dbl'(s)",
};

static void address_stg(struct _PipeLine *pipeP, struct _Registers *Reg);
static void read_stg(struct _PipeLine *pipeP, struct _Registers *Reg);
static void execute(struct _PipeLine *pipeP, struct _Registers *Reg);

Instruction_Class PSH_Obj =
{
  "PSH",
  NULL, // decode
  address_stg, // address_stg
  NULL, // access_1
  NULL, // access_2
  read_stg, // read_stg
  execute, // execute
  NULL, // write 
  NULL, // write_plus
  6,
  mask,
  opcode,
};

static void address_stg(struct _PipeLine *pipeP, struct _Registers *Reg)
{
  switch ( pipeP->opcode_subType )
    {
    case 2:
    case 4:
    case 5:
      smem_address_stg_b2(pipeP,Reg);
      break;
    }
}

static void read_stg(struct _PipeLine *pipeP, struct _Registers *Reg)
{
  switch ( pipeP->opcode_subType )
    {
    case 2:
    case 4:
      smem_read_stg(pipeP,Reg);
      break;
    case 5:
      smem_read_stg_dbl(pipeP,Reg);
      break;
    }
}

static void execute(struct _PipeLine *pipeP, struct _Registers *Reg)
{
  Opcode opcode;
  DWord dword;
  Word value;
  int r,wait_state;

  opcode = pipeP->decode_nfo.mach_code;

  switch ( pipeP->opcode_subType )
    {
    case 0:
      // PSH src1,src2
      MMR->SP = MMR->SP - 2;
      r = (opcode.bop[1]>>4) & 0xf;
      value = get_k16_reg(r);
      wait_state = write_data_mem_long(MMR->SP,value);
      r = opcode.bop[1] & 0xf;
      value = get_k16_reg(r);
      wait_state = write_data_mem_long(MMR->SP+1,value);
      break;
    case 1:
      // PSH src
      MMR->SP = MMR->SP - 1;
      r = (opcode.bop[1]>>4) & 0xf;
      value = get_k16_reg(r);
      wait_state = write_data_mem_long(MMR->SP,value);
      break;
    case 2:
      // PSH src,Smem
      MMR->SP = MMR->SP - 2;
      r = (opcode.bop[2]>>4) & 0xf;
      value = get_k16_reg(r);
      wait_state = write_data_mem_long(MMR->SP,value);
      wait_state = write_data_mem_long(MMR->SP+1,Reg->DB);
      break;
    case 3:
      // PSH dbl(ACx)
      MMR->SP = MMR->SP - 2;
      r = (opcode.bop[1]>>4) & 0x3;
      dword = get_k32_reg(r);
      value = ((DWord)dword>>16);
      wait_state = write_data_mem_long(MMR->SP,value);
      value = dword&0xffff;
      wait_state = write_data_mem_long(MMR->SP+1,value);
      break;
    case 4:
      // PSH Smem
      MMR->SP = MMR->SP - 1;
      wait_state = write_data_mem_long(MMR->SP,Reg->DB);
      break;
    case 5:
      // PSH dbl(Lmem)
      MMR->SP = MMR->SP - 2;
      wait_state = write_data_mem_long(MMR->SP,Reg->DB);
      wait_state = write_data_mem_long(MMR->SP+1,Reg->CB);
      break;
    }
}


