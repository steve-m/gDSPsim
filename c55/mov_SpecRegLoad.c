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
    "0001011p vvvvvuuu uuuu0000", //    MOV k7, DPH
    "0001011p vvvuuuuu uuuu0011", //    MOV k9, PDP
    "0001011p uuuuuuuu uuuu0100", //    MOV k12, BK03
    "0001011p uuuuuuuu uuuu0101", //    MOV k12, BK47
    "0001011p uuuuuuuu uuuu0110", //    MOV k12, BKC
    "0001011p uuuuuuuu uuuu1000", //    MOV k12, CSR
    "0001011p uuuuuuuu uuuu1001", //    MOV k12, BRC0
    "0001011p uuuuuuuu uuuu1010", //    MOV k12, BRC1
    "01111000 uuuuuuuu uuuuuuuu vvv0000v", //    MOV k16, DP
    "01111000 uuuuuuuu uuuuuuuu vvv0001v", //    MOV k16, SSP
    "01111000 uuuuuuuu uuuuuuuu vvv0010v", //    MOV k16, CDP
    "01111000 uuuuuuuu uuuuuuuu vvv0011v", //    MOV k16, BSA01
    "01111000 uuuuuuuu uuuuuuuu vvv0100v", //    MOV k16, BSA23
    "01111000 uuuuuuuu uuuuuuuu vvv0101v", //    MOV k16, BSA45
    "01111000 uuuuuuuu uuuuuuuu vvv0110v", //    MOV k16, BSA67
    "01111000 uuuuuuuu uuuuuuuu vvv0111v", //    MOV k16, BSAC
    "01111000 uuuuuuuu uuuuuuuu vvv1000v", //    MOV k16, SP

    "11011100 ssssssss 0000vv10", //    MOV Smem, DP
    "11011100 ssssssss 0001vv10", //    MOV Smem, CDP
    "11011100 ssssssss 0010vv10", //    MOV Smem, BSA01
    "11011100 ssssssss 0011vv10", //    MOV Smem, BSA23
    "11011100 ssssssss 0100vv10", //    MOV Smem, BSA45
    "11011100 ssssssss 0101vv10", //    MOV Smem, BSA67
    "11011100 ssssssss 0110vv10", //    MOV Smem, BSAC
    "11011100 ssssssss 0111vv10", //    MOV Smem, SP
    "11011100 ssssssss 1000vv10", //    MOV Smem, SSP
    "11011100 ssssssss 1001vv10", //    MOV Smem, BK03
    "11011100 ssssssss 1010vv10", //    MOV Smem, BK47
    "11011100 ssssssss 1011vv10", //    MOV Smem, BKC
    "11011100 ssssssss 1100vv10", //    MOV Smem, DPH
    "11011100 ssssssss 1111vv10", //    MOV Smem, PDP

    "11011100 ssssssss v000vv11", //    MOV Smem, CSR
    "11011100 ssssssss v001vv11", //    MOV Smem, BRC0
    "11011100 ssssssss v010vv11", //    MOV Smem, BRC1
    "11011100 ssssssss v011vv11", //    MOV Smem, TRN0
    "11011100 ssssssss v100vv11", //    MOV Smem, TRN1
    "11101101 ssssssss vvvv011v", //    MOV dbl(Lmem), RETA
  };

static gchar *opcode[] = 
{ 
  "'MOV' u,DPH",
  "'MOV' u,PDP",
  "'MOV' u,BK03",
  "'MOV' u,BK47",
  "'MOV' u,BKC",
  "'MOV' u,CSR",
  "'MOV' u,BRC0",
  "'MOV' u,BRC1",
  "'MOV' u,DP",
  "'MOV' u,SSP",
  "'MOV' u,CDP",
  "'MOV' u,BSA01",
  "'MOV' u,BSA23",
  "'MOV' u,BSA45",
  "'MOV' u,BSA67",
  "'MOV' u,BSAC",
  "'MOV' u,SP",
  "'MOV' s,DP",
  "'MOV' s,CDP",
  "'MOV' s,BSA01",
  "'MOV' s,BSA23",
  "'MOV' s,BSA45",
  "'MOV' s,BSA67",
  "'MOV' s,BSAC",
  "'MOV' s,SP",
  "'MOV' s,SSP",
  "'MOV' s,BK03",
  "'MOV' s,BK47",
  "'MOV' s,BKC",
  "'MOV' s,DPH",
  "'MOV' s,PDP",
  "'MOV' s,CSR",
  "'MOV' s,BRC0",
  "'MOV' s,BRC1",
  "'MOV' s,TRN0",
  "'MOV' s,TRN1",
  "'MOV' dbl(s),RETA",
};

static void address_stg(struct _PipeLine *pipeP, struct _Registers *Reg);
static void read_stg(struct _PipeLine *pipeP, struct _Registers *Reg);
static void execute(struct _PipeLine *pipeP, struct _Registers *Reg);

Instruction_Class MOV_SPEC_REG_LOAD_Obj =
{
  "MOV_SPEC_REG_LOAD",
  NULL, // decode
  address_stg, // address_stg
  NULL, // access_1
  NULL, // access_2
  read_stg, // read_stg
  execute, // execute
  NULL, // write 
  NULL, // write_plus
  37,
  mask,
  opcode,
};

static void address_stg(struct _PipeLine *pipeP, struct _Registers *Reg)
{
  if ( (pipeP->opcode_subType >= 17) &&
       (pipeP->opcode_subType <= 36) )
    {
      smem_address_stg_b2(pipeP,Reg);
    }
}

static void read_stg(struct _PipeLine *pipeP, struct _Registers *Reg)
{
  if ( (pipeP->opcode_subType >= 17) &&
       (pipeP->opcode_subType <= 35) )
    {
      smem_read_stg(pipeP,Reg);
    }
  else if (pipeP->opcode_subType == 36)
    {
      smem_read_stg_dbl(pipeP,Reg);
    }
}

static void execute(struct _PipeLine *pipeP, struct _Registers *Reg)
{
  Word k;
  Opcode mach_code;

  mach_code = pipeP->decode_nfo.mach_code;

  if ( pipeP->opcode_subType == 0 )
    {
      k = (((Word)mach_code.bop[1] & 0x7) <<4) |  
	(((Word)mach_code.bop[2] & 0xf) >>4);
      MMR->DPH = k;
    }
  else if ( pipeP->opcode_subType == 1 )
    {
      k = (((Word)mach_code.bop[1] & 0x1f) <<4) |  
	(((Word)mach_code.bop[2] & 0xf) >>4);
      MMR->PDP = k;
    }
  else if ( pipeP->opcode_subType < 8 )
    {
      k = (((Word)mach_code.bop[1]) <<4) |  
	(((Word)mach_code.bop[2] & 0xf) >>4);
      switch ( pipeP->opcode_subType )
	{
	case 2:
	  MMR->BK03 = k;
	  return;
	case 3:
	  MMR->BK47 = k;
	  return;
	case 4:
	  MMR->BKC = k;
	  return;
	case 5:
	  MMR->CSR = k;
	  return;
	case 6:
	  MMR->BRC0 = k;
	  return;
	case 7:
	  MMR->BRC1 = k;
	  return;
	}
    }
  else if ( pipeP->opcode_subType < 17 )
    {
      k = (((Word)mach_code.bop[1]) <<8) |  
	(((Word)mach_code.bop[2]));
      switch ( pipeP->opcode_subType )
	{
	case 8:
	  MMR->DP = k;
	  return;
	case 9:
	  MMR->SSP = k;
	  return;
	case 10:
	  MMR->CDP = k;
	  return;
	case 11:
	  MMR->BSA01 = k;
	  return;
	case 12:
	  MMR->BSA23 = k;
	  return;
	case 13:
	  MMR->BSA45 = k;
	  return;
	case 14:
	  MMR->BSA67 = k;
	  return;
	case 15:
	  MMR->BSAC = k;
	  return;
	case 16:
	  MMR->SP = k;
	  return;
	}
    }
  else if ( pipeP->opcode_subType == 36 )
    {
      Reg->RETA = ((DWord)Reg->DB)<<16 | ((DWord)Reg->CB);
    }
  else
    {
      switch ( pipeP->opcode_subType )
	{
	case 17:
	  MMR->DP = Reg->DB;
	  return;
	case 18:
	  MMR->CDP = Reg->DB;
	  return;
	case 19:
	  MMR->BSA01 = Reg->DB;
	  return;
	case 20:
	  MMR->BSA23 = Reg->DB;
	  return;
	case 21:
	  MMR->BSA45 = Reg->DB;
	  return;
	case 22:
	  MMR->BSA67 = Reg->DB;
	  return;
	case 23:
	  MMR->BSAC = Reg->DB;
	  return;
	case 24:
	  MMR->SP = Reg->DB;
	  return;
	case 25:
	  MMR->SSP = Reg->DB;
	  return;
	case 26:
	  MMR->BK03 = Reg->DB;
	  return;
	case 27:
	  MMR->BK47 = Reg->DB;
	  return;
	case 28:
	  MMR->BKC = Reg->DB;
	  return;
	case 29:
	  MMR->DPH = Reg->DB;
	  return;
	case 30:
	  MMR->PDP = Reg->DB;
	  return;
	case 31:
	  MMR->CSR = Reg->DB;
	  return;
	case 32:
	  MMR->BRC0 = Reg->DB;
	  return;
	case 33:
	  MMR->BRC1 = Reg->DB;
	  return;
	case 34:
	  MMR->TRN0 = Reg->DB;
	  return;
	case 35:
	  MMR->TRN1 = Reg->DB;
	  return;
	}
    }
}


