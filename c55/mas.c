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
#include <cmem.h>
#include <smem.h>
#include <xymem.h>
#include <shifter.h>
#include <memory.h>

static gchar *mask[]=
{
  "0101011p RRrrtt1f", // MAS[R] Tx, [ACx,] ACy
  "11010001 ssssssss 3fRR10cc", // MASM[R] [T3 = ]Smem, Cmem, ACx
  "11010010 ssssssss 3fRR01rr", // MASM[R] [T3 = ]Smem, [ACx,] ACy
  "11010101 ssssssss 3fRRttrr", // MASM[R] [T3 = ]Smem, Tx, [ACx,] ACy
  "10000110 xxxxxxyy yyyyrrRR 0114UH3f", // MASM[R][40] [T3 = ][uns(]Xmem[)], [uns(]Ymem[)],[ACx,] ACy
};

static gchar *opcode[] = 
{ 
  "'MAS'f t,r,R", // MAS[R] Tx, [ACx,] ACy
  "'MASM'f 3s,c,R", // MASM[R] [T3 = ]Smem, Cmem, ACx
  "'MASM'f 3s,r,R", // MASM[R] [T3 = ]Smem, [ACx,] ACy
  "'MASM'f 3s,t,r,R", // MASM[R] [T3 = ]Smem, Tx, [ACx,] ACy
  "'MASM'f4 3UxV,HyI,r,R", // MASM[R][40] [T3 = ][uns(]Xmem[)], [uns(]Ymem[)],[ACx,] ACy
};

static void address_stg(struct _PipeLine *pipeP, struct _Registers *Reg);
static void read_stg(struct _PipeLine *pipeP, struct _Registers *Reg);
static void execute(struct _PipeLine *pipeP, struct _Registers *Reg);

Instruction_Class MAS_Obj =
{
  "MAS",
  NULL, // decode
  address_stg, // address_stg
  NULL, // access_1
  NULL, // access_2
  read_stg, // read_stg
  execute, // execute
  NULL, // write 
  NULL, // write_plus
  5,
  mask,
  opcode,
};

static void address_stg(struct _PipeLine *pipeP, struct _Registers *Reg)
{
  switch ( pipeP->opcode_subType )
    {
    case 1:
      // MASM[R] [T3 = ]Smem, Cmem, ACx
      smem_address_stg_b2(pipeP,Reg);
      cmem_address_stg_b3(pipeP,Reg);
      break;

    case 2:
    case 3:
      // MASM[R] [T3 = ]Smem, [ACx,] ACy
      // MASM[R] [T3 = ]Smem, Tx, [ACx,] ACy
      smem_address_stg_b2(pipeP,Reg);

    case 4:
      // MASM[R][40] [T3 = ][uns(]Xmem[)], [uns(]Ymem[)],[ACx,] ACy
      xymem_address_stg_b2(pipeP,Reg);
      break;

    }
}

static void read_stg(struct _PipeLine *pipeP, struct _Registers *Reg)
{
  switch ( pipeP->opcode_subType )
    {
    case 1:
      // MASM[R] [T3 = ]Smem, Cmem, ACx
      smem_read_stg(pipeP,Reg);
      cmem_read_stg(pipeP,Reg);
      break;

    case 2:
    case 3:
      // MASM[R] [T3 = ]Smem, [ACx,] ACy
      // MASM[R] [T3 = ]Smem, Tx, [ACx,] ACy
      smem_read_stg(pipeP,Reg);
      break;

    case 4:
      // MASM[R][40] [T3 = ][uns(]Xmem[)], [uns(]Ymem[)],[ACx,] ACy
      xymem_read_stg(pipeP,Reg);
      break;
    }
}

static void execute(struct _PipeLine *pipeP, struct _Registers *Reg)
{
  Opcode opcode;
  int r,R,flag,t;

  opcode = pipeP->decode_nfo.mach_code;

  switch ( pipeP->opcode_subType )
    {
    case 0:
      // MAS[R] Tx, [ACx,] ACy
      t=((opcode.bop[1]>>2)&0x3)+4;
      r=(opcode.bop[1]>>4)&0x3;
      R=(opcode.bop[1]>>6)&0x3;
      flag = (opcode.bop[1]&0x1) 
	? (MULT_ROUND | MULT_SUBTRACT): MULT_SUBTRACT;
      multiplier(t,r,R,R,flag,Reg);
      break;

    case 1:
      // MASM[R] [T3 = ]Smem, Cmem, ACx
      R=(opcode.bop[2]>>4)&0x3;
      flag = (opcode.bop[2]&0x40) 
	? (MULT_ROUND | MULT_SUBTRACT): MULT_SUBTRACT;
      multiplier(MULT_DB,MULT_BB,R,R,flag,Reg);
      if ( opcode.bop[2]&0x80 )
	{
	  MMR->T3 = Reg->DB;
	}
      break;

    case 2:
      // MASM[R] [T3 = ]Smem, [ACx,] ACy
      R=(opcode.bop[2]>>4)&0x3;
      r=opcode.bop[2]&0x3;
      flag = (opcode.bop[2]&0x40) 
	? (MULT_ROUND | MULT_SUBTRACT): MULT_SUBTRACT;
      multiplier(MULT_DB,r,R,R,flag,Reg);
      if ( opcode.bop[2]&0x80 )
	{
	  MMR->T3 = Reg->DB;
	}
      break;

    case 3:
      // MASM[R] [T3 = ]Smem, Tx, [ACx,] ACy
      R=(opcode.bop[2]>>4)&0x3;
      t=((opcode.bop[2]>>2)&0x3)+4;
      r=opcode.bop[2]&0x3;
      flag = (opcode.bop[2]&0x40) 
	? (MULT_ROUND | MULT_SUBTRACT): MULT_SUBTRACT;
      multiplier(MULT_DB,t,r,R,flag,Reg);
      if ( opcode.bop[2]&0x80 )
	{
	  MMR->T3 = Reg->DB;
	}
      break;

    case 4:
      // MASM[R][40] [T3 = ][uns(]Xmem[)], [uns(]Ymem[)],[ACx,] ACy
      r=(opcode.bop[2]>>2)&0x3;
      R=opcode.bop[2]&0x3;
      flag = (opcode.bop[3]&0x1) 
	? (MULT_ROUND | MULT_SUBTRACT): MULT_SUBTRACT;
      flag = (opcode.bop[3]&0x8) ? (flag | MULT_UNSIGNED_X) : flag;
      flag = (opcode.bop[3]&0x4) ? (flag | MULT_UNSIGNED_Y) : flag;
      flag = (opcode.bop[3]&0x10) ? (flag | MULT_M40_IS_1) : flag;
      multiplier(MULT_DB,MULT_CB,r,R,flag,Reg);
      if ( opcode.bop[3]&0x2 )
	{
	  MMR->T3 = Reg->DB;
	}
      break;

    }
}
