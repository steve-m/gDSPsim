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
#include <chip_help.h>

static gchar *mask[]=
  {
    "0011101p rrrrRRRR", // POP src1,src2
    "0101000p rrrrv010", // POP src
    "11100100 ssssssss rrrrv1vv", // POP src,Smem
    "0101000p vvrrv011", // POP dbl(ACx)
    "10111011 sssssss", // POP Smem
    "10111000 sssssss", // POP dbl(Lmem)


  };

static gchar *opcode[] = 
{ 
  "'POP' r,R",
  "'POP' r",
  "'POP' r,s",
  "'POP' 'dbl'(r)",
  "'POP' s",
  "'POP' 'dbl'(s)",
};

static void write_stg(struct _PipeLine *pipeP, struct _Registers *Reg);
static void execute(struct _PipeLine *pipeP, struct _Registers *Reg);

Instruction_Class POP_Obj =
{
  "POP",
  NULL, // decode
  NULL, // address_stg
  NULL, // access_1
  NULL, // access_2
  NULL, // read_stg
  execute, // execute
  write_stg, // write 
  NULL, // write_plus
  6,
  mask,
  opcode,
};

static void execute(struct _PipeLine *pipeP, struct _Registers *Reg)
{
  Opcode opcode;
  Word value;
  int r,wait_state;

  opcode = pipeP->decode_nfo.mach_code;

  switch ( pipeP->opcode_subType )
    {
    case 0:
      // POP src1,src2
      r = (opcode.bop[1]>>4) & 0xf;
      value = read_data_mem_long((MMR->SPH<<16) | MMR->SP,&wait_state);
      set_k16_regLO(r,value);
      r = opcode.bop[1] & 0xf;
      value = read_data_mem_long((MMR->SPH<<16) | (MMR->SP+1),&wait_state);
      set_k16_regLO(r,value);
      MMR->SP = MMR->SP + 2;
      break;
    case 1:
      // POP src
      r = (opcode.bop[1]>>4) & 0xf;
      value = read_data_mem_long((MMR->SPH<<16) | MMR->SP,&wait_state);
      set_k16_regLO(r,value);
      MMR->SP = MMR->SP + 1;
      break;
    case 2:
      // POP src,Smem
      r = (opcode.bop[2]>>4) & 0xf;
      value = read_data_mem_long((MMR->SPH<<16) | MMR->SP,&wait_state);
      set_k16_regLO(r,value);
      Reg->EB = read_data_mem_long((MMR->SPH<<16) | (MMR->SP+1),&wait_state);
      smem_set_EAB_b2(pipeP,Reg);
      MMR->SP = MMR->SP + 2;
      break;
    case 3:
      // POP dbl(ACx)
      r = (opcode.bop[1]>>4) & 0x3;
      value = read_data_mem_long((MMR->SPH<<16) | MMR->SP,&wait_state);
      set_k16_regLO(r,value);
      r = opcode.bop[1] & 0xf;
      value = read_data_mem_long((MMR->SPH<<16) | (MMR->SP+1),&wait_state);
      set_k16_regHI(r,value);
      MMR->SP = MMR->SP + 2;
      break;
    case 4:
      // POP Smem
      Reg->EB = read_data_mem_long((MMR->SPH<<16) | MMR->SP,&wait_state);
      smem_set_EAB_b2(pipeP,Reg);
      MMR->SP = MMR->SP + 2;
      break;
    case 5:
      // POP dbl(Lmem)
      pipeP->storage1 = read_data_mem_long((MMR->SPH<<16) | MMR->SP,&wait_state);
      pipeP->storage2 = read_data_mem_long((MMR->SPH<<16) | (MMR->SP+1),&wait_state);
      lmem_set_EAB_b2(pipeP,Reg);
      MMR->SP = MMR->SP + 2;
      break;
    }
}


static void write_stg(struct _PipeLine *pipeP, struct _Registers *Reg)
{
  switch ( pipeP->opcode_subType )
    {
    case 2:
    case 4:
      write_data_mem(Reg->EAB,Reg->EB);
      break;
    case 5:
      write_data_mem(Reg->EAB,pipeP->storage1);
      write_data_mem(Reg->EAB^1,pipeP->storage1);
      break;
    }
}

