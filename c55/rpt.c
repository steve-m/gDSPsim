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

#include <chip_core.h>
#include <smem.h>
#include <chip_help.h>

static void address_stg(struct _PipeLine *pipeP, struct _Registers *Reg);
static void execute(struct _PipeLine *pipeP, struct _Registers *Reg);

static gchar *mask[]=
{
  "0100100p vvvvv000", // RPT CSR
  "0100100p rrrrv001", // RPTADD CSR, TAx
  "0100110p uuuuuuuu", // RPT k8
  "0100100p uuuuv010", // RPTADD CSR, k4
  "0100100p uuuuv011", // RPTSUB CSR, k4
  "0000110p uuuuuuuu uuuuuuuu", // RPT k16
  "0000000p vbbbbbbb uuuuuuuu", // RPTCC k8, cond
};

static gchar *opcode[] = 
{ 
  "'RPT CSR'",
  "'RPTADD CSR',r",
  "'RPT' u",
  "'RPTADD CSR',u",
  "'RPTSUB CSR',u",
  "'RPT' u",
  "'RPTCC' u,b",
};

/* This definition is global because another routine will make have
 * an array that points to all the different instruction classes.
 */
Instruction_Class RPT_Obj =
{
  "RPT",
  NULL, // decode
  address_stg, // address_stg
  NULL, // access_1
  NULL, // access_2
  NULL, // read_stg1 (access)
  execute, // execute
  NULL, // write 
  NULL, // write_plus
  7,
  mask,
  opcode,
};


static void address_stg(struct _PipeLine *pipeP, struct _Registers *Reg)
{
  Opcode opcode;

  opcode = pipeP->decode_nfo.mach_code;

  switch ( pipeP->opcode_subType )
    {
    case 0: // RPT CSR
      MMR->RPTC = MMR->CSR;
      break;
    case 1: // RPTADD CSR, TAx
      MMR->RPTC = MMR->CSR;
      break;
    case 2:  // RPT k8
      MMR->RPTC = (Word)opcode.bop[1];
      break;
    case 3: // RPTADD CSR, k4
      MMR->RPTC = MMR->CSR;
      break;
    case 4: // RPTSUB CSR, k4
      MMR->RPTC = MMR->CSR;
      break;
    case 5:  // RPT k16
      MMR->RPTC = (Word)opcode.bop[1]<<8 | opcode.bop[2];
      break;
    case 6: // RPTCC k8, cond
      if ( check_condition(opcode.bop[1]&0x7f,Reg) )
	MMR->RPTC = (Word)opcode.bop[1];
      break;
    default:
      break;
    }
}

static void execute(struct _PipeLine *pipeP, struct _Registers *Reg)
{
  Opcode opcode;
  int r;

  opcode = pipeP->decode_nfo.mach_code;

  switch ( pipeP->opcode_subType )
    {
    case 1: // RPTADD CSR, TAx
      r = (opcode.bop[1]>>4)&0xf;
      MMR->CSR += (SWord)get_k16_reg(r);
      break;
    case 3: // RPTADD CSR, k4
      MMR->CSR += (opcode.bop[1]>>4)&0xf;
      break;
    case 4: // RPTSUB CSR, k4
      MMR->CSR -= (opcode.bop[1]>>4)&0xf;
      break;
    default:
      break;
    }
}
