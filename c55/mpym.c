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
  "11010001 ssssssss 3fRR00cc", // MPYM[R] [T3 = ]Smem, Cmem, ACx
  "11010011 ssssssss 3fRR00rr", // MPYM[R] [T3 = ]Smem, [ACx,] ACy
  "11111000 ssssssss hhhhhhhh vvRRv03f", // MPYMK[R] [T3 = ]Smem, K8, ACx
  "10000110 xxxxxxyy yyyyvvRR 0004UH3f", // MPYM[R][40] [T3 = ][uns(]Xmem[)], [uns(]Ymem[)], ACx
  "11010011 ssssssss 3fRRA1tt", // MPYM[R][U] [T3 = ]Smem, Tx, ACx  
};

static gchar *opcode[] = 
{ 
  "'MPYM'f 3s,c,R", // MPYM[R] [T3 = ]Smem, Cmem, ACx
  "'MPYM'f 3s,r,R", // MPYM[R] [T3 = ]Smem, [ACx,] ACy
  "'MPYMK'f 3s,h,R", // MPYMK[R] [T3 = ]Smem, K8, ACx
  "'MPYM'f4 3UxV,HyI,R", // MPYM[R][40] [T3 = ][uns(]Xmem[)], [uns(]Ymem[)], ACx
  "'MPYM'fA 3s,t,R", // MPYM[R][U] [T3 = ]Smem, Tx, ACx

};

static void address_stg(struct _PipeLine *pipeP, struct _Registers *Reg);
static void read_stg(struct _PipeLine *pipeP, struct _Registers *Reg);
static void execute(struct _PipeLine *pipeP, struct _Registers *Reg);

Instruction_Class MPYM_Obj =
{
  "MPYM",
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
    case 0:
      // MPYM[R] [T3 = ]Smem, Cmem, ACx
      smem_address_stg_b2(pipeP,Reg);
      cmem_address_stg_b3(pipeP,Reg);
      break;
    case 1:
    case 2:
    case 4:
      // MPYM[R] [T3 = ]Smem, [ACx,] ACy
      // MPYMK[R] [T3 = ]Smem, K8, ACx
      // MPYM[R][U] [T3 = ]Smem, Tx, ACx
      smem_address_stg_b2(pipeP,Reg);
      break;
    case 3:
      // MPYM[R][40] [T3 = ][uns(]Xmem[)], [uns(]Ymem[)], ACx
      xymem_address_stg_b2(pipeP,Reg);
      break;
    }
}

static void read_stg(struct _PipeLine *pipeP, struct _Registers *Reg)
{
  switch ( pipeP->opcode_subType )
    {
    case 0:
      // MPYM[R] [T3 = ]Smem, Cmem, ACx
      smem_read_stg(pipeP,Reg);
      cmem_read_stg(pipeP,Reg);
      break;
    case 1:
    case 2:
    case 4:
      // MPYM[R] [T3 = ]Smem, [ACx,] ACy
      // MPYMK[R] [T3 = ]Smem, K8, ACx
      // MPYM[R][U] [T3 = ]Smem, Tx, ACx
      smem_read_stg(pipeP,Reg);
      break;
    case 3:
      // MPYM[R][40] [T3 = ][uns(]Xmem[)], [uns(]Ymem[)], ACx
      xymem_read_stg(pipeP,Reg);
      break;
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
      // MPYM[R] [T3 = ]Smem, Cmem, ACx
      R=(opcode.bop[2]>>4)&0x3;
      flag = (opcode.bop[2]&0x40) ? MULT_ROUND : 0;

      multiplier(MULT_DB,MULT_BB,MULT_NO_MAC,R,flag,Reg);
      if ( opcode.bop[2]&0x80 )
	{
	  MMR->T3 = Reg->DB;
	}
      break;

    case 1:
      // MPYM[R] [T3 = ]Smem, [ACx,] ACy
      r=opcode.bop[2]&0x3;
      R=(opcode.bop[2]>>4)&0x3;
      flag = (opcode.bop[2]&0x40) ? MULT_ROUND : 0;

      multiplier(MULT_DB,r,MULT_NO_MAC,R,flag,Reg);
      if ( opcode.bop[2]&0x80 )
	{
	  MMR->T3 = Reg->DB;
	}
      break;
    case 2:
      // MPYMK[R] [T3 = ]Smem, K8, ACx
      R=(opcode.bop[3]>>4)&0x3;
      Reg->P = (SWord)((char)opcode.bop[2]);
      flag = (opcode.bop[3]&0x1) ? MULT_ROUND : 0;

      multiplier(MULT_DB,MULT_P,MULT_NO_MAC,R,flag,Reg);
      if ( opcode.bop[3]&0x2 )
	{
	  MMR->T3 = Reg->DB;
	}
      break;
     
    case 3:
      // MPYM[R][40] [T3 = ][uns(]Xmem[)], [uns(]Ymem[)], ACx
      flag = (opcode.bop[3]&0x1) ? MULT_ROUND : 0;
      flag = (opcode.bop[3]&0x8) ? (flag | MULT_UNSIGNED_X) : flag;
      flag = (opcode.bop[3]&0x4) ? (flag | MULT_UNSIGNED_Y) : flag;
      flag = (opcode.bop[3]&0x10) ? (flag | MULT_M40_IS_1) : flag;

      multiplier(MULT_DB,MULT_P,MULT_NO_MAC,R,flag,Reg);
      if ( opcode.bop[3]&0x2 )
	{
	  MMR->T3 = Reg->DB;
	}
      break;

    case 4:
      // MPYM[R][U] [T3 = ]Smem, Tx, ACx
      R=(opcode.bop[2]>>4)&0x3;
      r=(opcode.bop[2]&0x3)+4;;
      flag = (opcode.bop[2]&0x8) ? (MULT_UNSIGNED_X | MULT_UNSIGNED_Y) : 0;
      flag = (opcode.bop[2]&0x40) ? (MULT_ROUND | flag) : flag;
      
      multiplier(MULT_DB,r,MULT_NO_MAC,R,flag,Reg);
      if ( opcode.bop[2]&0x80 )
	{
	  MMR->T3 = Reg->DB;
	}
      break;
     
    }
}
