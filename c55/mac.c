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
  // Multiply and Accumulate
  "0101011p RRrrtt0f", // MAC[R] ACx, Tx, ACy[, ACy]
  "0101100p RRrrtt1f", // MAC[R] ACy, Tx, ACx, ACy
  "0001111p nnnnnnnn rrRRtt1f", // MACK[R] Tx, K8, [ACx,] ACy
  "01111001 hhhhhhhh hhhhhhhh rrRRtt1f", // MACK[R] Tx, K16, [ACx,] ACy
  "11010001 ssssssss 3fRR01cc", // MACM[R] [T3 = ]Smem, Cmem, ACx

  "11010000 ssssssss 3fRRvvcc", // MACM[R]Z [T3 = ]Smem, Cmem, ACx
  "11010010 ssssssss 3fRR00rr", // MACM[R] [T3 = ]Smem, [ACx,] ACy
  "11010100 ssssssss 3fRRttrr", // MACM[R] [T3 = ]Smem, Tx, [ACx,] ACy
  "11111000 ssssssss nnnnnnnn rrRRv13f", // MACMK[R] [T3 = ]Smem, K8, [ACx,] ACy
  "10000110 xxxxxxyy yyyyrrRR 0014UH3f", // MACM[R][40] [T3 = ][uns(]Xmem[)], [uns(]Ymem[)],[ACx,] ACy

  "10000110 xxxxxxyy yyyyrrRR 0104UH3f", // MACM[R][40] [T3 = ][uns(]Xmem[)], [uns(]Ymem[)],ACx >> #16[, ACy]  
};

static gchar *opcode[] = 
{ 
  "'MAC'f r,t,R,R", // MAC[R] ACx, Tx, ACy[, ACy]
  "'MAC'f R,t,r,R", // MAC[R] ACy, Tx, ACx, ACy
  "'MACK'f t,n,r,R", // MACK[R] Tx, K8, [ACx,] ACy
  "'MACK'f t,h,r,R", // MACK[R] Tx, K16, [ACx,] ACy
  "'MACM'f 3s,c,R", // MACM[R] [T3 = ]Smem, Cmem, ACx

  "'MACM'f'Z' 3s,c,R", // MACM[R]Z [T3 = ]Smem, Cmem, ACx
  "'MACM'f 3s,r,R", // MACM[R] [T3 = ]Smem, [ACx,] ACy
  "'MACM'f 3s,t,r,R", // MACM[R] [T3 = ]Smem, Tx, [ACx,] ACy
  "'MACMK'f 3s,n,r,R", // MACMK[R] [T3 = ]Smem, K8, [ACx,] ACy
  "'MAC'f4 3UxV,HyI,r,R", // MACM[R][40] [T3 = ][uns(]Xmem[)], [uns(]Ymem[)],[ACx,] ACy

  "'MAC'f4 3UxV,HyI,r'<<#16',R", // MACM[R][40] [T3 = ][uns(]Xmem[)], [uns(]Ymem[)],ACx >> #16[, ACy]
};

static void address_stg(struct _PipeLine *pipeP, struct _Registers *Reg);
static void read_stg(struct _PipeLine *pipeP, struct _Registers *Reg);
static void execute(struct _PipeLine *pipeP, struct _Registers *Reg);
static void write_stg(struct _PipeLine *pipeP, struct _Registers *Reg);

Instruction_Class MAC_Obj =
{
  "MAC",
  NULL, // decode
  address_stg, // address_stg
  NULL, // access_1
  NULL, // access_2
  read_stg, // read_stg
  execute, // execute
  write_stg, // write 
  NULL, // write_plus
  11,
  mask,
  opcode,
};

static void address_stg(struct _PipeLine *pipeP, struct _Registers *Reg)
{
  switch ( pipeP->opcode_subType )
    {
    case 4:
      // MACM[R] [T3 = ]Smem, Cmem, ACx
      smem_address_stg_b2(pipeP,Reg);
      cmem_address_stg_b3(pipeP,Reg);
      break;

    case 5:
      // MACM[R]Z [T3 = ]Smem, Cmem, ACx
      smem_address_stg_b2(pipeP,Reg);
      cmem_address_stg_b3(pipeP,Reg);
      pipeP->storage1 = Reg->DAB;
      break;

    case 6:
    case 7:
    case 8:
      // MACM[R] [T3 = ]Smem, [ACx,] ACy
      // MACM[R] [T3 = ]Smem, [ACx,] ACy
      // MACMK[R] [T3 = ]Smem, K8, [ACx,] ACy
      smem_address_stg_b2(pipeP,Reg);

    case 9:
    case 10:
      // MACM[R][40] [T3 = ][uns(]Xmem[)], [uns(]Ymem[)],[ACx,] ACy
      // MACM[R][40] [T3 = ][uns(]Xmem[)], [uns(]Ymem[)],ACx >> #16[, ACy]
      xymem_address_stg_b2(pipeP,Reg);
      break;
    }
}

static void read_stg(struct _PipeLine *pipeP, struct _Registers *Reg)
{
  switch ( pipeP->opcode_subType )
    {
    case 4:
    case 5:
      // MACM[R] [T3 = ]Smem, Cmem, ACx
      // MACM[R]Z [T3 = ]Smem, Cmem, ACx
      smem_read_stg(pipeP,Reg);
      cmem_read_stg(pipeP,Reg);
      break;

    case 6:
    case 7:
    case 8:
      // MACM[R] [T3 = ]Smem, [ACx,] ACy
      // MACM[R] [T3 = ]Smem, [ACx,] ACy
      // MACMK[R] [T3 = ]Smem, K8, [ACx,] ACy
      smem_read_stg(pipeP,Reg);
      break;

    case 9:
    case 10:
      // MACM[R][40] [T3 = ][uns(]Xmem[)], [uns(]Ymem[)],[ACx,] ACy
      // MACM[R][40] [T3 = ][uns(]Xmem[)], [uns(]Ymem[)],ACx >> #16[, ACy]
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
      // MAC[R] ACx, Tx, ACy[, ACy]
      t=((opcode.bop[1]>>2)&0x3)+4;
      r=(opcode.bop[1]>>4)&0x3;
      R=(opcode.bop[1]>>6)&0x3;
      flag = (opcode.bop[1]&0x1) ? MULT_ROUND : 0;
      multiplier(r,t,R,R,flag,Reg);
      break;

    case 1:
      // MAC[R] ACy, Tx, ACx, ACy
      t=((opcode.bop[1]>>2)&0x3)+4;
      r=(opcode.bop[1]>>4)&0x3;
      R=(opcode.bop[1]>>6)&0x3;
      flag = (opcode.bop[1]&0x1) ? MULT_ROUND : 0;
      multiplier(R,t,r,R,flag,Reg);
      break;

    case 2:
      // MACK[R] Tx, K8, [ACx,] ACy
      t=((opcode.bop[2]>>2)&0x3)+4;
      r=(opcode.bop[2]>>6)&0x3;
      R=(opcode.bop[2]>>4)&0x3;
      Reg->P = (SWord)((char)opcode.bop[1]);
      flag = (opcode.bop[2]&0x1) ? MULT_ROUND : 0;
      multiplier(t,MULT_P,r,R,flag,Reg);
      break;

    case 3:
      // MACK[R] Tx, K16, [ACx,] ACy
      t=((opcode.bop[3]>>2)&0x3)+4;
      r=(opcode.bop[3]>>6)&0x3;
      R=(opcode.bop[3]>>4)&0x3;
      Reg->P = (((Word)opcode.bop[1])<<8) | opcode.bop[2];
      flag = (opcode.bop[2]&0x1) ? MULT_ROUND : 0;
      multiplier(t,MULT_P,r,R,flag,Reg);
      break;

    case 4:
      // MACM[R] [T3 = ]Smem, Cmem, ACx
      R=(opcode.bop[2]>>4)&0x3;
      flag = (opcode.bop[2]&0x40) ? MULT_ROUND : 0;
      multiplier(MULT_DB,MULT_BB,R,R,flag,Reg);
      if ( opcode.bop[2]&0x80 )
	{
	  MMR->T3 = Reg->DB;
	}
      break;

    case 5:
      // MACM[R]Z [T3 = ]Smem, Cmem, ACx
      R=(opcode.bop[2]>>4)&0x3;
      flag = (opcode.bop[2]&0x40) ? MULT_ROUND : 0;
      multiplier(MULT_DB,MULT_BB,R,R,flag,Reg);
      if ( opcode.bop[2]&0x80 )
	{
	  MMR->T3 = Reg->DB;
	}
      Reg->EAB = pipeP->storage1 + 1; // delay
      pipeP->storage1 = Reg->DB;
      break;

    case 6:
      // MACM[R] [T3 = ]Smem, [ACx,] ACy
      R=(opcode.bop[2]>>4)&0x3;
      r=opcode.bop[2]&0x3;
      flag = (opcode.bop[2]&0x40) ? MULT_ROUND : 0;
      multiplier(MULT_DB,r,R,R,flag,Reg);
      if ( opcode.bop[2]&0x80 )
	{
	  MMR->T3 = Reg->DB;
	}
      break;

    case 7:
      // MACM[R] [T3 = ]Smem, Tx, [ACx,] ACy
      R=(opcode.bop[2]>>4)&0x3;
      t=((opcode.bop[2]>>2)&0x3)+4;
      r=opcode.bop[2]&0x3;
      flag = (opcode.bop[2]&0x40) ? MULT_ROUND : 0;
      multiplier(MULT_DB,t,r,R,flag,Reg);
      if ( opcode.bop[2]&0x80 )
	{
	  MMR->T3 = Reg->DB;
	}
      break;

    case 8:
      // MACMK[R] [T3 = ]Smem, K8, [ACx,] ACy
      r=(opcode.bop[3]>>6)&0x3;
      R=(opcode.bop[3]>>4)&0x3;
      Reg->P = (SWord)((char)opcode.bop[2]);
      flag = (opcode.bop[3]&0x1) ? MULT_ROUND : 0;
      multiplier(MULT_DB,MULT_P,r,R,flag,Reg);
      if ( opcode.bop[3]&0x2 )
	{
	  MMR->T3 = Reg->DB;
	}
      break;

    case 9:
      // MACM[R][40] [T3 = ][uns(]Xmem[)], [uns(]Ymem[)],[ACx,] ACy
      r=(opcode.bop[2]>>2)&0x3;
      R=opcode.bop[2]&0x3;
      flag = (opcode.bop[3]&0x1) ? MULT_ROUND : 0;
      flag = (opcode.bop[3]&0x8) ? (flag | MULT_UNSIGNED_X) : flag;
      flag = (opcode.bop[3]&0x4) ? (flag | MULT_UNSIGNED_Y) : flag;
      flag = (opcode.bop[3]&0x10) ? (flag | MULT_M40_IS_1) : flag;
      multiplier(MULT_DB,MULT_CB,r,R,flag,Reg);
      if ( opcode.bop[3]&0x2 )
	{
	  MMR->T3 = Reg->DB;
	}
      break;

    case 10:
      // MACM[R][40] [T3 = ][uns(]Xmem[)], [uns(]Ymem[)],ACx >> #16[, ACy]
      r=(opcode.bop[2]>>2)&0x3;
      R=opcode.bop[2]&0x3;
      flag = (opcode.bop[3]&0x1) 
	? (MULT_ROUND | MULT_RIGHT_SHIFT_MAC) : MULT_RIGHT_SHIFT_MAC;
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

static void write_stg(struct _PipeLine *pipeP, struct _Registers *Reg)
{
  if ( pipeP->opcode_subType == 5 )
    {
      // MACM[R]Z [T3 = ]Smem, Cmem, ACx
      write_data_mem_long(Reg->EAB,pipeP->storage1);
    }
}
