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
#include <xymem.h>
#include <memory.h>
#include <shifter.h>
#include <multiplier.h>
#include <alu.h>

static gchar *mask[]=
{
  "10000111 xxxxxxyy yyyyrrRR 000vtt3f", // MPYM[R] [T3 = ]Xmem, Tx, ACy :: MOV HI(ACx << T2), Ymem
  "10000111 xxxxxxyy yyyyrrRR 001vtt3f", // MACM[R] [T3 = ]Xmem, Tx, ACy :: MOV HI(ACx << T2), Ymem
  "10000111 xxxxxxyy yyyyrrRR 010vtt3f", // MASM[R] [T3 = ]Xmem, Tx, ACy :: MOV HI(ACx << T2), Ymem
  "10000111 xxxxxxyy yyyyrrRR 100vvvvv", // ADD Xmem << #16, ACx, ACy :: MOV HI(ACy << T2), Ymem
  "10000111 xxxxxxyy yyyyrrRR 101vvvvv", // SUB Xmem << #16, ACx, ACy :: MOV HI(ACy << T2), Ymem

  "10000111 xxxxxxyy yyyyrrRR 110vvvvv", // MOV Xmem << #16, ACy :: MOV HI(ACx << T2), Ymem
  "10000110 xxxxxxyy yyyyrrRR 101vtt3f", // MACM[R] [T3 = ]Xmem, Tx, ACx :: MOV Ymem << #16, ACy
  "10000110 xxxxxxyy yyyyrrRR 100vtt3f", // MASM[R] [T3 = ]Xmem, Tx, ACx :: MOV Ymem << #16, ACy

};

static gchar *opcode[] = 
{
  "'MPYM'f 3x,t,R || 'MOV' 'HI'(r'<<T2'),y", // MPYM[R] [T3 = ]Xmem, Tx, ACy :: MOV HI(ACx << T2), Ymem
  "'MACM'f 3x,t,R || 'MOV' 'HI'(r'<<T2'),y", // MACM[R] [T3 = ]Xmem, Tx, ACy :: MOV HI(ACx << T2), Ymem
  "'MASM'f 3x,t,R || 'MOV' 'HI'(r'<<T2'),y", // MASM[R] [T3 = ]Xmem, Tx, ACy :: MOV HI(ACx << T2), Ymem
  "'ADD' x '<< #16,' r,R '|| MOV HI'(R '<< T2'),y", // ADD Xmem << #16, ACx, ACy :: MOV HI(ACy << T2), Ymem
  "'SUB' x '<< #16,' r,R '|| MOV HI'(R '<< T2'),y", // SUB Xmem << #16, ACx, ACy :: MOV HI(ACy << T2), Ymem

  "'MOV' x '<< #16,' R '|| MOV HI'(r '<< T2')", // MOV Xmem << #16, ACy :: MOV HI(ACx << T2), Ymem
  "'MACM'f 3x,t,R || 'MOV' y'<<#16'),R", // MACM[R] [T3 = ]Xmem, Tx, ACx :: MOV Ymem << #16, ACy
  "'MASM'f 3x,t,R || 'MOV' y'<<#16'),R", // MASM[R] [T3 = ]Xmem, Tx, ACx :: MOV Ymem << #16, ACy
};

static void address_stg(struct _PipeLine *pipeP, struct _Registers *Reg);
static void read_stg(struct _PipeLine *pipeP, struct _Registers *Reg);
static void execute(struct _PipeLine *pipeP, struct _Registers *Reg);
static void write_stg(struct _PipeLine *pipeP, struct _Registers *Reg);

Instruction_Class IMPLIED_PARALLEL_INSTR_Obj =
{
  "IMPLIED_PARALLEL_INSTR",
  NULL, // decode
  address_stg, // address_stg
  NULL, // access_1
  NULL, // access_2
  read_stg, // read_stg
  execute, // execute
  write_stg, // write 
  NULL, // write_plus
  8,
  mask,
  opcode,
};

static void address_stg(struct _PipeLine *pipeP, struct _Registers *Reg)
{
  switch ( pipeP->opcode_subType )
    {
    case 0:
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
      // MPYM[R] [T3 = ]Xmem, Tx, ACy :: MOV HI(ACx << T2), Ymem
      // MACM[R] [T3 = ]Xmem, Tx, ACy :: MOV HI(ACx << T2), Ymem
      // MASM[R] [T3 = ]Xmem, Tx, ACy :: MOV HI(ACx << T2), Ymem
      // ADD Xmem << #16, ACx, ACy :: MOV HI(ACy << T2), Ymem
      // SUB Xmem << #16, ACx, ACy :: MOV HI(ACy << T2), Ymem
      // MOV Xmem << #16, ACy :: MOV HI(ACx << T2), Ymem
      xmem_address_stg_b2(pipeP,Reg);
      break;
    case 6:
    case 7:
      // MACM[R] [T3 = ]Xmem, Tx, ACx :: MOV Ymem << #16, ACy
      // MASM[R] [T3 = ]Xmem, Tx, ACx :: MOV Ymem << #16, ACy
      xymem_address_stg_b2(pipeP,Reg);
      break;
    }
}

static void read_stg(struct _PipeLine *pipeP, struct _Registers *Reg)
{
  switch ( pipeP->opcode_subType )
    {
    case 0:
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
      // MPYM[R] [T3 = ]Xmem, Tx, ACy :: MOV HI(ACx << T2), Ymem
      // MACM[R] [T3 = ]Xmem, Tx, ACy :: MOV HI(ACx << T2), Ymem
      // MASM[R] [T3 = ]Xmem, Tx, ACy :: MOV HI(ACx << T2), Ymem
      // ADD Xmem << #16, ACx, ACy :: MOV HI(ACy << T2), Ymem
      // SUB Xmem << #16, ACx, ACy :: MOV HI(ACy << T2), Ymem
      // MOV Xmem << #16, ACy :: MOV HI(ACx << T2), Ymem
      smem_read_stg(pipeP,Reg);
      return;
    case 6:
    case 7:
      // MACM[R] [T3 = ]Xmem, Tx, ACx :: MOV Ymem << #16, ACy
      // MASM[R] [T3 = ]Xmem, Tx, ACx :: MOV Ymem << #16, ACy
      xymem_read_stg(pipeP,Reg);
      return;

    }
}

static void execute(struct _PipeLine *pipeP, struct _Registers *Reg)
{
  int r,R,flag,t;
  Opcode opcode;

  opcode = pipeP->decode_nfo.mach_code;

  switch ( pipeP->opcode_subType )
    {
    case 0:
      // MPYM[R] [T3 = ]Xmem, Tx, ACy :: MOV HI(ACx << T2), Ymem
      R=opcode.bop[2]&0x3;
      r=(opcode.bop[2]>>2)&0x3;
      t=((opcode.bop[3]>>2)&0x3)+4;
      flag =  (SHFT_DONT_SATURATE | SHFT_NO_OVERFLOW_DETECTION | 
	       SHFT_M40_IS_0 | SHFT_SIGN_EXTEND_USING_SXMD);
      shifter(r,3,0,flag,SHFT_EB,Reg);
      ymem_set_EAB_b23(pipeP,Reg);
      // no rounding ?
      pipeP->storage1 = get_k16_regHI(r,0);
      flag = (opcode.bop[3]&0x1) ? MULT_ROUND : 0;
      
      multiplier(MULT_DB,t,MULT_NO_MAC,R,flag,Reg);
      if ( opcode.bop[3]&0x2 )
	{
	  MMR->T3 = Reg->DB;
	}
      break;

    case 1:
      // MACM[R] [T3 = ]Xmem, Tx, ACy :: MOV HI(ACx << T2), Ymem
      R=opcode.bop[2]&0x3;
      r=(opcode.bop[2]>>2)&0x3;
      t=((opcode.bop[3]>>2)&0x3)+4;
      flag =  (SHFT_DONT_SATURATE | SHFT_NO_OVERFLOW_DETECTION | 
	       SHFT_M40_IS_0 | SHFT_SIGN_EXTEND_USING_SXMD);
      shifter(r,3,0,flag,SHFT_EB,Reg);
      ymem_set_EAB_b23(pipeP,Reg);
      // no rounding here?
      pipeP->storage1 = get_k16_regHI(r,0);
      flag = (opcode.bop[3]&0x1) ? MULT_ROUND : 0;
      
      multiplier(MULT_DB,t,R,R,flag,Reg);
      if ( opcode.bop[3]&0x2 )
	{
	  MMR->T3 = Reg->DB;
	}
      break;

    case 2:
      // MASM[R] [T3 = ]Xmem, Tx, ACy :: MOV HI(ACx << T2), Ymem
      R=opcode.bop[2]&0x3;
      r=(opcode.bop[2]>>2)&0x3;
      t=((opcode.bop[3]>>2)&0x3)+4;
      flag =  (SHFT_DONT_SATURATE | SHFT_NO_OVERFLOW_DETECTION | 
	       SHFT_M40_IS_0 | SHFT_SIGN_EXTEND_USING_SXMD);
      shifter(r,3,0,flag,SHFT_EB,Reg);
      ymem_set_EAB_b23(pipeP,Reg);
      // no rounding here?
      pipeP->storage1 = get_k16_regHI(r,0);
      flag = (opcode.bop[3]&0x1) 
	? (MULT_ROUND | MULT_SUBTRACT): MULT_SUBTRACT;
      
      multiplier(MULT_DB,t,R,R,flag,Reg);
      if ( opcode.bop[3]&0x2 )
	{
	  MMR->T3 = Reg->DB;
	}
      break;

    case 3:
      // ADD Xmem << #16, ACx, ACy :: MOV HI(ACy << T2), Ymem
      R=opcode.bop[2]&0x3;
      r=(opcode.bop[2]>>2)&0x3;
      flag =  (SHFT_DONT_SATURATE | SHFT_NO_OVERFLOW_DETECTION | 
	       SHFT_M40_IS_0 | SHFT_SIGN_EXTEND_USING_SXMD);
      shifter(R,3,0,flag,SHFT_EB,Reg);
      ymem_set_EAB_b23(pipeP,Reg);

      flag = C54CM(MMR) 
	? (SHFT_DONT_SATURATE | SHFT_NO_OVERFLOW_DETECTION | 
	   SHFT_M40_IS_1 | SHFT_SIGN_EXTEND_USING_SXMD)
	: (SHFT_USE_SATD_SATA | SHFT_SIGN_EXTEND_USING_SXMD | 
	   SHFT_USE_M40 | SHFT_SET_ACOV);
      shifter(SHFT_DB, SHFT_CONSTANT, 16, flag, SHFT_SHIFTER, Reg);
      alu(ALU_SHIFTER, r, R, 0, Reg);
      break;

    case 4:
      // SUB Xmem << #16, ACx, ACy :: MOV HI(ACy << T2), Ymem
      R=opcode.bop[2]&0x3;
      r=(opcode.bop[2]>>2)&0x3;
      flag =  (SHFT_DONT_SATURATE | SHFT_NO_OVERFLOW_DETECTION | 
	       SHFT_M40_IS_0 | SHFT_SIGN_EXTEND_USING_SXMD);
      shifter(R,3,0,flag,SHFT_EB,Reg);
      ymem_set_EAB_b23(pipeP,Reg);

      flag = C54CM(MMR) 
	? (SHFT_DONT_SATURATE | SHFT_NO_OVERFLOW_DETECTION | 
	   SHFT_M40_IS_1 | SHFT_SIGN_EXTEND_USING_SXMD)
	: (SHFT_USE_SATD_SATA | SHFT_SIGN_EXTEND_USING_SXMD | 
	   SHFT_USE_M40 | SHFT_SET_ACOV);
      shifter(SHFT_DB, SHFT_CONSTANT, 16, flag, SHFT_SHIFTER, Reg);
      alu(ALU_SHIFTER, r, R, ALU_SUB, Reg);
      break;

    case 5:
      // MOV Xmem << #16, ACy :: MOV HI(ACx << T2), Ymem

      // MOV HI(ACx << T2), Ymem
      r=(opcode.bop[2]&0xc)>>2;
      flag =  (SHFT_DONT_SATURATE | SHFT_NO_OVERFLOW_DETECTION | 
	       SHFT_M40_IS_0 | SHFT_SIGN_EXTEND_USING_SXMD);
      shifter(r,3,0,flag,SHFT_EB,Reg);
      pipeP->storage1 = get_k16_regHI(r,0);
      ymem_set_EAB_b23(pipeP,Reg);

      // MOV Xmem << #16, ACy
      R=opcode.bop[2]&0x3;
      flag =  (SHFT_DONT_SATURATE | SHFT_NO_OVERFLOW_DETECTION | 
	       SHFT_M40_IS_0 | SHFT_SIGN_EXTEND_USING_SXMD);
      shifter(SHFT_DB,SHFT_CONSTANT,16,flag,R,Reg);
      break;

    case 6:
      // MACM[R] [T3 = ]Xmem, Tx, ACx :: MOV Ymem << #16, ACy
      R=opcode.bop[2]&0x3;
      r=(opcode.bop[2]>>2)&0x3;
      t=((opcode.bop[3]>>2)&0x3)+4;
      flag =  (SHFT_DONT_SATURATE | SHFT_NO_OVERFLOW_DETECTION | 
	       SHFT_M40_IS_0 | SHFT_SIGN_EXTEND_USING_SXMD);
      shifter(SHFT_CB,SHFT_CONSTANT,16,flag,R,Reg);

      flag = (opcode.bop[3]&0x1) ? MULT_ROUND : 0;      
      multiplier(MULT_DB,t,r,r,flag,Reg);
      if ( opcode.bop[3]&0x2 )
	{
	  MMR->T3 = Reg->DB;
	}
      break;

    case 7:
      // MASM[R] [T3 = ]Xmem, Tx, ACx :: MOV Ymem << #16, ACy
      R=opcode.bop[2]&0x3;
      r=(opcode.bop[2]>>2)&0x3;
      t=((opcode.bop[3]>>2)&0x3)+4;
      flag =  (SHFT_DONT_SATURATE | SHFT_NO_OVERFLOW_DETECTION | 
	       SHFT_M40_IS_0 | SHFT_SIGN_EXTEND_USING_SXMD);
      shifter(SHFT_CB,SHFT_CONSTANT,16,flag,R,Reg);

      flag = (opcode.bop[3]&0x1) 
	? (MULT_ROUND | MULT_SUBTRACT): MULT_SUBTRACT;
      multiplier(MULT_DB,t,r,r,flag,Reg);
      if ( opcode.bop[3]&0x2 )
	{
	  MMR->T3 = Reg->DB;
	}
      break;

    }
}


static void write_stg(struct _PipeLine *pipeP, struct _Registers *Reg)
{
  
  switch ( pipeP->opcode_subType )
    {
    case 0:
    case 1:
    case 2:
    case 3:
    case 4:
      // MPYM[R] [T3 = ]Xmem, Tx, ACy :: MOV HI(ACx << T2), Ymem
      // MACM[R] [T3 = ]Xmem, Tx, ACy :: MOV HI(ACx << T2), Ymem
      // MASM[R] [T3 = ]Xmem, Tx, ACy :: MOV HI(ACx << T2), Ymem
      // ADD Xmem << #16, ACx, ACy :: MOV HI(ACy << T2), Ymem
      // SUB Xmem << #16, ACx, ACy :: MOV HI(ACy << T2), Ymem
      write_data_mem_long(Reg->EAB,Reg->EB);
      return;
    case 5:
      // MOV Xmem << #16, ACy :: MOV HI(ACx << T2), Ymem
      write_data_mem_long(Reg->EAB,pipeP->storage1);
      return;
    }
}
