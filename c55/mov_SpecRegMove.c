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
  "0101001p rrrr1110", // MOV TAx, BRC0
  "0101001p rrrr1101", // MOV TAx, BRC1
  "0101001p rrrr1010", // MOV TAx, CDP
  "0101001p rrrr1100", // MOV TAx, CSR
  "0101001p rrrr1000", // MOV TAx, SP

  "0101001p rrrr1001", // MOV TAx, SSP
  "0100010p 1100rrrr", // MOV BRC0, TAx
  "0100010p 1101rrrr", // MOV BRC1, TAx
  "0100010p 1010rrrr", // MOV CDP, TAx
  "0100010p 1110rrrr", // MOV RPTC, TAx

  "0100010p 1000rrrr", // MOV SP, TAx
  "0100010p 1001rrrr", // MOV SSP, TAx
};

static gchar *opcode[] = 
{
  "'MOV' r,'BRC0'",
  "'MOV' r,'BRC1'",
  "'MOV' r,'CDP'",
  "'MOV' r,'CSR'",
  "'MOV' r,'SP'",

  "'MOV' r,'SSP'",
  "'MOV' 'BRC0',r",
  "'MOV' 'BRC1',r",
  "'MOV' 'CDP',r",
  "'MOV' 'RPTC',r",

  "'MOV' 'SP',r",
  "'MOV' 'SSP',r",
};

static void execute(struct _PipeLine *pipeP, struct _Registers *Reg);

Instruction_Class MOV_SPEC_REG_MOVE_Obj =
{
  "MOV_SPEC_REG_MOVE",
  NULL, // decode
  NULL, // address_stg
  NULL, // access_1
  NULL, // access_2
  NULL, // read_stg
  execute, // execute
  NULL, // write_stg 
  NULL, // write_plus
  12,
  mask,
  opcode,
};

static void execute(struct _PipeLine *pipeP, struct _Registers *Reg)
{
  Opcode opcode;
  int r;

  opcode = pipeP->decode_nfo.mach_code;

  switch ( pipeP->opcode_subType )
    {
    case 0:
      // MOV TAx, BRC0
      r = (opcode.bop[1]>>4) & 0xf;
      MMR->BRC0=get_k16_reg(r);
      return;
    case 1:
      // MOV TAx, BRC1
      r = (opcode.bop[1]>>4) & 0xf;
      MMR->BRC1=get_k16_reg(r);
      return;
    case 2:
      // MOV TAx, CDP
      r = (opcode.bop[1]>>4) & 0xf;
      MMR->CDP=get_k16_reg(r);
      return;
    case 3:
      // MOV TAx, CSR
      r = (opcode.bop[1]>>4) & 0xf;
      MMR->CSR=get_k16_reg(r);
      return;
    case 4:
      // MOV TAx, SP
      r = (opcode.bop[1]>>4) & 0xf;
      MMR->SP=get_k16_reg(r);
      return;
    case 5:
      // MOV TAx, SSP
      r = (opcode.bop[1]>>4) & 0xf;
      MMR->SSP=get_k16_reg(r);
      return;
    case 6:
      // MOV BRC0, TAx
      r = opcode.bop[1] & 0xf;
      set_k16_reg(r,MMR->BRC0,0);
      return;
    case 7:
      // MOV BRC1, TAx
      r = opcode.bop[1] & 0xf;
      set_k16_reg(r,MMR->BRC1,0);
      return;
    case 8:
      // MOV CDP, TAx
      r = opcode.bop[1] & 0xf;
      set_k16_reg(r,MMR->CDP,0);
      return;
    case 9:
      // MOV RPTC, TAx
      r = opcode.bop[1] & 0xf;
      set_k16_reg(r,MMR->RPTC,0);
      return;
    case 10:
      // MOV SP, TAx
      r = opcode.bop[1] & 0xf;
      set_k16_reg(r,MMR->SP,0);
      return;
    case 11:
      // MOV SSP, TAx
      r = opcode.bop[1] & 0xf;
      set_k16_reg(r,MMR->SSP,0);
      return;
    }
}
