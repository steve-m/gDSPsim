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
#include <shifter.h>
#include <memory.h>
#include <chip_help.h>
#include <alu.h>

static gchar *mask[]=
{
  "0010011p rrrrRRRR", // SUB src, dst
  "0100001p uuuuRRRR", // SUB k4, dst
  "01111100 uuuuuuuu uuuuuuuu RRRRrrrr", // SUB k16, src, dst
  "11010111 ssssssss RRRRrrrr", // SUB Smem, src, dst
  "11011000 ssssssss RRRRrrrr", // SUB src, Smem, dst
  
  "0101101p RRrrtt01", // SUB ACx << Tx, ACy
  "0001000p RRrr0100 vvnnnnnn", // SUB ACx << #SHIFTW[, ACy]
  "01111010 hhhhhhhh hhhhhhhh rrRR001v", // SUB k16 << #16, [ACx,] ACy
  "01110001 hhhhhhhh hhhhhhhh rrRRuuuu", // SUB k16 << #SHFT, [ACx,] ACy
  "11011101 ssssssss rrRRtt01", // SUB Smem << Tx, [ACx,] ACy
  
  "11011110 ssssssss rrRR0101", // SUB Smem << #16, [ACx,] ACy
  "11011110 ssssssss rrRR0110", // SUB ACX, Smem << #16, ACy
  "11011111 ssssssss rrRR101U", // SUB [uns(]Smem[)], BORROW, [ACx,] ACy
  "11011111 ssssssss rrRR111U", // SUB [uns(]Smem[)], [ACx,] ACy
  "11111001 ssssssss Uvnnnnnn rrRR01vv", // SUB [uns(]Smem[)] << #SHIFTW, [ACx,] ACy
  
  "11101101 ssssssss rrRR001v", // SUB dbl(Lmem), [ACx,] ACy
  "11101101 ssssssss rrRR010v", // SUB ACX, dbl(Lmem), ACy
  "10000001 xxxxxxyy yyyy01RR", // SUB Xmem, Ymem, ACx
  
  // Dual 16-bit arithmetic
  "11101110 ssssssss rrRR001v", // SUB dual(Lmem), [ACx,] ACy
  "11101110 ssssssss rrRR010v", // SUB ACx, dual(Lmem), ACy
  
  "11101110 ssssssss ttRR011v", // SUB dual(Lmem), Tx, ACx
  "11101110 ssssssss ttRR101v", // SUB Tx, dual(Lmem), ACx
};

static gchar *opcode[] = 
{
  "'SUB' r,R", // SUB src, dst
  "'SUB' #u,R", // SUB k4, dst
  "'SUB' #u,r,R", // SUB k16, src, dst
  "'SUB' s,r,R", // SUB Smem, src, dst
  "'SUB' r,s,R", // SUB src, Smem, dst

  "'SUB' r<<t,R", // SUB ACx << Tx, ACy
  "'SUB' r<<#n,R",  // SUB ACx << #SHIFTW[, ACy]
  "'SUB' h<<#16,r,R", // SUB k16 << #16, [ACx,] ACy
  "'SUB' h<<#u,r,R", // SUB k16 << #SHFT, [ACx,] ACy
  "'SUB' s<<t,r,R",  // SUB Smem << Tx, [ACx,] ACy

  "'SUB' s<<#16,r,R",  // SUB Smem << #16, [ACx,] ACy
  "'SUB' r,s<<#16,R",  // SUB ACx, Smem << #16, ACy
  "'SUB' UsV,'BORROW',r,R",  // SUB [uns(]Smem[)], BORROW, [ACx,] ACy
  "'SUB' UsV,r,R",  // SUB [uns(]Smem[)], [ACx,] ACy
  "'SUB' UsV<<#n,r,R",  // SUB [uns(]Smem[)] << #SHIFTW, [ACx,] ACy

  "'SUB' 'dbl'(s),r,R",  // SUB dbl(Lmem), [ACx,] ACy
  "'SUB' r,'dbl'(s),R",  // SUB ACX, dbl(Lmem), ACy
  "'SUB' x,y,R",  // SUB Xmem, Ymem, ACx

  // Dual 16-bit arithmetic
  "'SUB' 'dual'(s),r,R", // SUB dual(Lmem), [ACx,] ACy
  "'SUB' r,'dual'(s),R", // SUB ACx, dual(Lmem), ACy

  "'SUB' 'dual'(s),t,R", // SUB dual(Lmem), Tx, ACx
  "'SUB' t,'dual'(s),R", // SUB Tx, dual(Lmem), ACx
};

static void address_stg(struct _PipeLine *pipeP, struct _Registers *Reg);
static void read_stg(struct _PipeLine *pipeP, struct _Registers *Reg);
static void execute(struct _PipeLine *pipeP, struct _Registers *Reg);

Instruction_Class SUB_Obj =
{
  "SUB",
  NULL, // decode
  address_stg, // address_stg
  NULL, // access_1
  NULL, // access_2
  read_stg, // read_stg
  execute, // execute
  NULL, // write 
  NULL, // write_plus
  22,
  mask,
  opcode,
};

static void address_stg(struct _PipeLine *pipeP, struct _Registers *Reg)
{
  switch ( pipeP->opcode_subType )
    {
    case 3:
    case 4:
    case 9:
    case 10:
    case 11:
    case 12:
    case 13:
    case 14:
      smem_address_stg_b2(pipeP,Reg);
      break;
    case 15:
    case 16:
    case 18:
    case 19:
    case 20:
    case 21:
      lmem_address_stg_b2(pipeP,Reg);
      break;
    case 17:
      xymem_address_stg_b2(pipeP,Reg);
      break;
    }
}

static void read_stg(struct _PipeLine *pipeP, struct _Registers *Reg)
{
  switch ( pipeP->opcode_subType )
    {
    case 3:
    case 4:
    case 9:
    case 10:
    case 11:
    case 12:
    case 13:
    case 14:
      smem_read_stg(pipeP,Reg);
    case 15:
    case 16:
    case 17:
    case 18:
    case 19:
    case 20:
    case 21:
      xymem_read_stg(pipeP,Reg);
      break;
    }
}

static void execute(struct _PipeLine *pipeP, struct _Registers *Reg)
{
  int r,R,shift,t,flag;
  Opcode opcode;
  SWord n;
  union _GP_Reg_Union reg_union;
  // union _GP_Reg_Union reg_union2;
  Word kword;

  opcode = pipeP->decode_nfo.mach_code;

  
  switch ( pipeP->opcode_subType )
    {
    case 0:
      // SUB src, dst
      r = (opcode.bop[1]>>4)&0xf;
      R = opcode.bop[1]&0xf;
      alu(r, R, R, ALU_SUB, Reg);
      return;

    case 1:
      // SUB k4, dst
      R = opcode.bop[1]&0xf;
      BYTES_TO_GP_REG(Reg->Shifter,0,0,(opcode.bop[1]>>4)&0xf);
      alu(ALU_SHIFTER, R, R, ALU_SUB, Reg);
      return;

    case 2:
      // SUB k16, src, dst
      R = (opcode.bop[3]>>4)&0xf;
      r = opcode.bop[3]&0xf;
      BYTES_TO_GP_REG(Reg->Shifter,SXMD(MMR),opcode.bop[1],opcode.bop[2]);
      alu(ALU_SHIFTER, r, R, ALU_SUB, Reg);
      return;

    case 3:
      // SUB Smem, src, dst
      R = (opcode.bop[2]>>4)&0xf;
      r = opcode.bop[2]&0xf;
      alu(ALU_DB, r, R, ALU_SUB, Reg);
      return;

    case 4:
      // SUB src, Smem,  dst
      R = (opcode.bop[2]>>4)&0xf;
      r = opcode.bop[2]&0xf;
      alu(r, ALU_DB, R, ALU_SUB, Reg);
      return;

    case 5:
      // SUB ACx << Tx, ACy
      R = (opcode.bop[1]>>6)&0x3;
      r = (opcode.bop[1]>>4)&0x3;
      t = (opcode.bop[1]>>2)&0x3;
      flag = C54CM(MMR) 
	? (SHFT_DONT_SATURATE | SHFT_NO_OVERFLOW_DETECTION | 
	   SHFT_M40_IS_1 | SHFT_SIGN_EXTEND_USING_SXMD)
	: (SHFT_USE_SATD_SATA | SHFT_SIGN_EXTEND_USING_SXMD | 
	   SHFT_USE_M40 | SHFT_SET_ACOV);
      shifter(r, t, 0, flag, SHFT_SHIFTER, Reg);
      alu(ALU_SHIFTER, R, R, ALU_SUB, Reg);
      return;

    case 6:
      // SUB ACx << #SHIFTW[, ACy]
      R = (opcode.bop[1]>>6)&0x3;
      r = (opcode.bop[1]>>4)&0x3;
      n = (int)SIGN6BIT_TO_UINT(opcode.bop[2]&0x3f);
      flag = C54CM(MMR) 
	? (SHFT_DONT_SATURATE | SHFT_NO_OVERFLOW_DETECTION | 
	   SHFT_M40_IS_1 | SHFT_SIGN_EXTEND_USING_SXMD)
	: (SHFT_USE_SATD_SATA | SHFT_SIGN_EXTEND_USING_SXMD | 
	   SHFT_USE_M40 | SHFT_SET_ACOV);
      shifter(r, SHFT_CONSTANT, n, flag, SHFT_SHIFTER, Reg);
      alu(ALU_SHIFTER, R, R, ALU_SUB, Reg);
      return;

    case 7:
      // SUB k16 << #16, [ACx,] ACy
      R = (opcode.bop[3]>>6)&0x3;
      r = (opcode.bop[3]>>4)&0x3;
      BYTES_SHFT16_TO_GP_REG(Reg->Shifter,SXMD(MMR),opcode.bop[1],opcode.bop[2]);
      alu(ALU_SHIFTER, r, R, ALU_SUB, Reg);
      return;

    case 8:
      // SUB k16 << #SHFT, [ACx,] ACy
      R = (opcode.bop[3]>>4)&0x3;
      r = (opcode.bop[3]>>6)&0x3;
      shift = opcode.bop[3] & 0xf;
      kword = ((Word)opcode.bop[1])<<8 | opcode.bop[2];
      reg_union.guint64 = SXMD(MMR) ? (SWord)kword : kword;
      reg_union.guint64 = reg_union.guint64 << shift;
      Reg->Shifter = reg_union.gp_reg;
      alu(ALU_SHIFTER, r, R, ALU_SUB, Reg);
      return;
      
    case 9:
      // SUB Smem << Tx, [ACx,] ACy
      R = (opcode.bop[2]>>4)&0x3;
      r = (opcode.bop[2]>>6)&0x3;
      t = (opcode.bop[2]>>2)&0x3;
      flag = C54CM(MMR) ? (0xc5) : (M40(MMR) | 0xc4);
      shifter(SHFT_DB, t, 0, flag, SHFT_SHIFTER, Reg);
      alu(ALU_SHIFTER, R, R, ALU_SUB, Reg);
      return;

    case 10:
      // SUB Smem << #16, [ACx,] ACy
      R = (opcode.bop[2]>>4)&0x3;
      r = (opcode.bop[2]>>6)&0x3;
      flag = C54CM(MMR) ? (0xc5) : (M40(MMR) | 0xc4);
      shifter(SHFT_DB, SHFT_CONSTANT, 16, flag, SHFT_SHIFTER, Reg);
      alu(ALU_SHIFTER, R, R, ALU_SUB, Reg);
      return;

    case 11:
      // SUB ACx, Smem << #16, ACy
      R = (opcode.bop[2]>>4)&0x3;
      r = (opcode.bop[2]>>6)&0x3;
      flag = C54CM(MMR) ? (0xc5) : (M40(MMR) | 0xc4);
      shifter(SHFT_DB, SHFT_CONSTANT, 16, flag, SHFT_SHIFTER, Reg);
      alu(R, ALU_SHIFTER, R, ALU_SUB, Reg);
      return;

    case 12:
      // SUB [uns(]Smem[)], BORROW, [ACx,] ACy
      R = (opcode.bop[2]>>4)&0x3;
      r = (opcode.bop[2]>>6)&0x3;
      flag = (opcode.bop[2]&1) 
	? (ALU_UNSIGNED_X | ALU_WITH_CARRY | ALU_SUB) 
	: ALU_WITH_CARRY | ALU_SUB;
      alu(ALU_DB, r, R, flag, Reg);
      return;

    case 13:
      // SUB [uns(]Smem[)], [ACx,] ACy
      R = (opcode.bop[2]>>4)&0x3;
      r = (opcode.bop[2]>>6)&0x3;
      flag = (opcode.bop[2]&1) ? ALU_UNSIGNED_X | ALU_SUB : ALU_SUB;
      alu(ALU_DB, r, R, flag, Reg);
      return;

    case 14:
      // SUB [uns(]Smem[)] << #SHIFTW, [ACx,] ACy
      R = (opcode.bop[3]>>4)&0x3;
      r = (opcode.bop[3]>>6)&0x3;
      shift = (int)SIGN6BIT_TO_UINT(opcode.bop[3]&0x3f);
      flag = C54CM(MMR) 
	? (SHFT_DONT_SATURATE | SHFT_NO_OVERFLOW_DETECTION | 
	   SHFT_M40_IS_1 | SHFT_SIGN_EXTEND_USING_SXMD)
	: (SHFT_USE_SATD_SATA | SHFT_SIGN_EXTEND_USING_SXMD | 
	   SHFT_USE_M40 | SHFT_SET_ACOV);
      shifter(SHFT_DB, SHFT_CONSTANT, shift, flag, SHFT_SHIFTER, Reg);
      flag = (opcode.bop[2]&0x80) ? ALU_UNSIGNED_X | ALU_SUB : ALU_SUB;
      alu(ALU_SHIFTER, r, R, flag, Reg);
      return;

    case 15:
      // SUB dbl(Lmem), [ACx,] ACy
      R = (opcode.bop[2]>>4)&0x3;
      r = (opcode.bop[2]>>6)&0x3;
      alu(ALU_DBL, r, R, ALU_SUB, Reg);
      return;

    case 16:
      // SUB ACX, dbl(Lmem), ACy
      R = (opcode.bop[2]>>4)&0x3;
      r = (opcode.bop[2]>>6)&0x3;
      alu(ALU_DBL, r, R, ALU_SUB, Reg);
      return;

    case 17:
      // SUB Xmem, Ymem, ACx
      R = (opcode.bop[2])&0x3;
      alu(ALU_DB, ALU_CB, R, ALU_SUB, Reg);
      return;

    case 18:
      // SUB dual(Lmem), [ACx,] ACy
      R = (opcode.bop[2]>>4)&0x3;
      r = (opcode.bop[2]>>6)&0x3;
      alu(ALU_DBL, r, R, ALU_SUB | ALU_DUAL, Reg);
      return;

    case 19:
      // SUB ACx, dual(Lmem), ACy
      R = (opcode.bop[2]>>4)&0x3;
      r = (opcode.bop[2]>>6)&0x3;
      alu(r, ALU_DBL, R, ALU_SUB | ALU_DUAL, Reg);
      return;

    case 20:
      // SUB dual(Lmem), Tx, ACx
      R = (opcode.bop[2]>>4)&0x3;
      r = ((opcode.bop[2]>>6)&0x3)+4;
      alu(ALU_DBL, r, R, ALU_SUB | ALU_DUAL, Reg);
      return;

    case 21:
      // SUB Tx, dual(Lmem), ACx
      R = (opcode.bop[2]>>4)&0x3;
      r = ((opcode.bop[2]>>6)&0x3)+4;
      alu(r, ALU_DBL, R, ALU_SUB | ALU_DUAL, Reg);
      return;
    }
}
