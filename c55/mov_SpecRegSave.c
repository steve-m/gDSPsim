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
#include <memory.h>

static gchar *mask[]=
{
  "11100101 ssssssss 100110vv", // MOV BK03, Smem
  "11100101 ssssssss 101010vv", // MOV BK47, Smem
  "11100101 ssssssss 101110vv", // MOV BKC, Smem
  "11100101 ssssssss 001010vv", // MOV BSA01, Smem
  "11100101 ssssssss 001110vv", // MOV BSA23, Smem

  "11100101 ssssssss 010010vv", // MOV BSA45, Smem
  "11100101 ssssssss 010110vv", // MOV BSA67, Smem
  "11100101 ssssssss 011010vv", // MOV BSAC, Smem
  "11100101 ssssssss v00111vv", // MOV BRC0, Smem
  "11100101 ssssssss v01011vv", // MOV BRC1, Smem

  "11100101 ssssssss 000110vv", // MOV CDP, Smem
  "11100101 ssssssss v00011vv", // MOV CSR, Smem
  "11100101 ssssssss 000010vv", // MOV DP, Smem
  "11100101 ssssssss 110010vv", // MOV DPH, Smem
  "11100101 ssssssss 111110vv", // MOV PDP, Smem

  "11100101 ssssssss 011110vv", // MOV SP, Smem
  "11100101 ssssssss 100010vv", // MOV SSP, Smem
  "11100101 ssssssss v01111vv", // MOV TRN0, Smem
  "11100101 ssssssss v10011vv", // MOV TRN1, Smem
  "11101011 ssssssss vvvv01vv" // MOV RETA, dbl(Lmem)
};

static gchar *opcode[] = 
{
  "'MOV' 'BK03',s",
  "'MOV' 'BK47',s",
  "'MOV' 'BKC',s",
  "'MOV' 'BSA01',s",
  "'MOV' 'BSA23',s",

  "'MOV' 'BSA45',s",
  "'MOV' 'BSA67',s",
  "'MOV' 'BSAC',s",
  "'MOV' 'BRC0',s",
  "'MOV' 'BRC1',s",

  "'MOV' 'CDP',s",
  "'MOV' 'CSR',s",
  "'MOV' 'DP',s",
  "'MOV' 'DPH',s",
  "'MOV' 'PDP',s",

  "'MOV' 'SP',s",
  "'MOV' 'SSP',s",
  "'MOV' 'TRN0',s",
  "'MOV' 'TRN1',s",
  "'MOV' 'RETA','dbl'(s)",
};

static void write_stg(struct _PipeLine *pipeP, struct _Registers *Reg);
static void execute(struct _PipeLine *pipeP, struct _Registers *Reg);

Instruction_Class MOV_SPEC_REG_SAVE_Obj =
{
  "MOV_SPEC_REG_SAVE",
  NULL, // decode
  NULL, // address_stg
  NULL, // access_1
  NULL, // access_2
  NULL, // read_stg
  execute, // execute
  write_stg, // write_stg 
  NULL, // write_plus
  20,
  mask,
  opcode,
};

static void execute(struct _PipeLine *pipeP, struct _Registers *Reg)
{
  smem_set_EAB_b2(pipeP,Reg);

  switch ( pipeP->opcode_subType )
    {
    case 0:
      pipeP->storage1 = MMR->BK03;
      return;
    case 1:
      pipeP->storage1 = MMR->BK47;
      return;
    case 2:
      pipeP->storage1 = MMR->BKC;
      return;
    case 3:
      pipeP->storage1 = MMR->BSA01;
      return;
    case 4:
      pipeP->storage1 = MMR->BSA23;
      return;
    case 5:
      pipeP->storage1 = MMR->BSA45;
      return;
    case 6:
      pipeP->storage1 = MMR->BSA67;
      return;
    case 7:
      pipeP->storage1 = MMR->BSAC;
      return;
    case 8:
      pipeP->storage1 = MMR->BRC0;
      return;
    case 9:
      pipeP->storage1 = MMR->BRC1;
      return;
    case 10:
      pipeP->storage1 = MMR->CDP;
      return;
    case 11:
      pipeP->storage1 = MMR->CSR;
      return;
    case 12:
      pipeP->storage1 = MMR->DP;
      return;
    case 13:
      pipeP->storage1 = MMR->DPH;
      return;
    case 14:
      pipeP->storage1 = MMR->PDP;
      return;
    case 15:
      pipeP->storage1 = MMR->SP;
      return;
    case 16:
      pipeP->storage1 = MMR->SSP;
      return;
    case 17:
      pipeP->storage1 = MMR->TRN0;
      return;
    case 18:
      pipeP->storage1 = MMR->TRN1;
      return;
    case 19:
      {
	WordP reta;
	reta = Reg->RETA;
	pipeP->storage1 = (Word)((reta>>16)&0xffff);
	pipeP->storage2 = (Word)(reta&0xffff);
	return;
      }
    }
}

static void write_stg(struct _PipeLine *pipeP, struct _Registers *Reg)
{
  if ( pipeP->opcode_subType == 19 )
    {
      write_data_mem_long(Reg->EAB,pipeP->storage1);     
      write_data_mem_long(Reg->EAB^1,pipeP->storage2);     
    }
  else
    {
      write_data_mem_long(Reg->EAB,pipeP->storage1);     
    }
}
