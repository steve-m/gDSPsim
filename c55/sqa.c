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
#include <multiplier.h>
#include <smem.h>

static gchar *mask[]=
{
  "0101010p RRrr001f", // SQA[R] [ACx,] ACy
  "11010010 ssssssss 3fRR10rr", // SQAM[R] [T3 = ]Smem, [ACx,] ACy
};

static gchar *opcode[] = 
{ 
  "'SQA'f r,R", // SQA[R] [ACx,] ACy
  "'SQAM'f 3s,r,R", // SQAM[R] [T3 = ]Smem, [ACx,] ACy
};

static void address_stg(struct _PipeLine *pipeP, struct _Registers *Reg);
static void read_stg(struct _PipeLine *pipeP, struct _Registers *Reg);
static void execute(struct _PipeLine *pipeP, struct _Registers *Reg);

Instruction_Class SQA_Obj =
{
  "SQA",
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
  if ( pipeP->opcode_subType == 1 )
    {
      smem_address_stg_b2(pipeP,Reg);
    }
}

static void read_stg(struct _PipeLine *pipeP, struct _Registers *Reg)
{
  if ( pipeP->opcode_subType == 1 )
    {
      smem_read_stg(pipeP,Reg);
    }
}

static void execute(struct _PipeLine *pipeP, struct _Registers *Reg)
{
  Opcode opcode;
  int r,R,flag;

  opcode = pipeP->decode_nfo.mach_code;

  switch ( pipeP->opcode_subType )
    {
    case 0:
      r=(opcode.bop[1]>>4)&0x3;
      R=(opcode.bop[1]>>6)&0x3;
      flag = (opcode.bop[1]&0x1) ? MULT_ROUND : 0;
      
      multiplier(r,r,R,R,flag,Reg);
      return;
    case 1:
      // SQAM
      r=opcode.bop[2]&0x3;
      R=(opcode.bop[2]>>4)&0x3;
      flag = (opcode.bop[2]&0x40) ? MULT_ROUND : 0;
      multiplier(MULT_DB,MULT_DB,r,R,flag,Reg);
      if ( opcode.bop[2]&0x80 )
	{
	  MMR->T3 = Reg->DB;
	}
    }
}


