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
  // "0001100p uuuuuuuu RRRRrrrr", // ADD k8, src, dst
  // "0001 0000 00rr 00qq 0011 0000", // ADD r<<#-16,q

  "0010010p rrrrRRRR", // ADD src, dst   (fixme? maybe 0010010p)
  "0100000p uuuuRRRR", // ADD k4, dst
  "01111011 uuuuuuuu uuuuuuuu RRRRrrrr", // ADD k16, src, dst
  "11010110 ssssssss RRRRrrrr", // ADD Smem, src, dst
  "0101101p RRrrtt00", // ADD ACx << Tx, ACy

  "0001000p RRrr0011 vvnnnnnn", // ADD ACx << #SHIFTW[, ACy]
  "01111010 hhhhhhhh hhhhhhhh rrRR000v", // ADD k16 << #16, [ACx,] ACy
  "01110000 hhhhhhhh hhhhhhhh rrRRuuuu", // ADD k16 << #SHFT, [ACx,] ACy
  "11011101 ssssssss rrRRtt00", // ADD Smem << Tx, [ACx,] ACy
  "11011110 ssssssss rrRR0100", // ADD Smem << #16, [ACx,] ACy

  "11011111 ssssssss rrRR100U", // ADD [uns(]Smem[)], CARRY, [ACx,] ACy
  "11011111 ssssssss rrRR110U", // ADD [uns(]Smem[)], [ACx,] ACy
  "11111001 ssssssss Uvnnnnnn rrRR00vv", // ADD [uns(]Smem[)] << #SHIFTW, [ACx,] ACy
  "11101101 ssssssss rrRR000v", // ADD dbl(Lmem), [ACx,] ACy
  "10000001 xxxxxxyy yyyy00RR", // ADD Xmem, Ymem, ACx
 
  "11110111 ssssssss hhhhhhhh hhhhhhhh", // ADD k16, Smem

  // Dual 16-bit arithmetic
  "11101110 ssssssss rrRR000v", // ADD dual(Lmem), [ACx,] ACy
  "11101110 ssssssss ttRR100v", // ADD dual(Lmem), Tx, ACx

};

static gchar *opcode[] = 
{
  "'ADD' r,R", // ADD src, dst
  "'ADD' #u,R", // ADD k4, dst
  "'ADD' #u,r,R", // ADD k16, src, dst
  "'ADD' s,r,R", // ADD Smem, src, dst
  "'ADD' r<<t,R", // ADD ACx << Tx, ACy

  "'ADD' r<<#n,R",  // ADD ACx << #SHIFTW[, ACy]
  "'ADD' h<<#16,r,R", // ADD k16 << #16, [ACx,] ACy
  "'ADD' h<<#u,r,R", // ADD k16 << #SHFT, [ACx,] ACy
  "'ADD' s<<t,r,R",  // ADD Smem << Tx, [ACx,] ACy
  "'ADD' s<<#16,r,R",  // ADD Smem << #16, [ACx,] ACy

  "'ADD' UsV,'CARRY',r,R",  // ADD [uns(]Smem[)], CARRY, [ACx,] ACy
  "'ADD' UsV,r,R",  // ADD [uns(]Smem[)], [ACx,] ACy
  "'ADD' UsV<<#n,r,R",  // ADD [uns(]Smem[)] << #SHIFTW, [ACx,] ACy
  "'ADD' 'dbl'(s),r,R",  // ADD dbl(Lmem), [ACx,] ACy
  "'ADD' x,y,R",  // ADD Xmem, Ymem, ACx

  "'ADD' u,s", //ADD k16, Smem

  // Dual 16-bit arithmetic
  "'ADD' 'dual'(s),r,R", // ADD dual(Lmem), [ACx,] ACy
  "'ADD' 'dual'(s),t,R", // ADD dual(Lmem), Tx, ACx
};

static void address_stg(struct _PipeLine *pipeP, struct _Registers *Reg);
static void read_stg(struct _PipeLine *pipeP, struct _Registers *Reg);
static void execute(struct _PipeLine *pipeP, struct _Registers *Reg);
static void write_stg(struct _PipeLine *pipeP, struct _Registers *Reg);

Instruction_Class ADD_Obj =
{
  "ADD",
  NULL, // decode
  address_stg, // address_stg
  NULL, // access_1
  NULL, // access_2
  read_stg, // read_stg
  execute, // execute
  write_stg, // write 
  NULL, // write_plus
  18,
  mask,
  opcode,
};

static void address_stg(struct _PipeLine *pipeP, struct _Registers *Reg)
{
  switch ( pipeP->opcode_subType )
    {
    case 3:
    case 8:
    case 9:
    case 10:
    case 11:
    case 12:
      smem_address_stg_b2(pipeP,Reg);
      break;
    case 13:
      lmem_address_stg_b2(pipeP,Reg);
      break;
    case 14:
      xymem_address_stg_b2(pipeP,Reg);
      break;
    case 15:
      smem_address_stg_b2(pipeP,Reg);
      pipeP->storage2 = Reg->DAB;
      break;
    }
}

static void read_stg(struct _PipeLine *pipeP, struct _Registers *Reg)
{
  switch ( pipeP->opcode_subType )
    {
    case 3:
    case 8:
    case 9:
    case 10:
    case 11:
    case 12:
    case 15:
      smem_read_stg(pipeP,Reg);
      break;
    case 13:
    case 14:
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
  Word kword;

  opcode = pipeP->decode_nfo.mach_code;
  
  switch ( pipeP->opcode_subType )
    {
    case 0:
      // ADD src, dst
      r = (opcode.bop[1]>>4)&0xf;
      R = opcode.bop[1]&0xf;
      alu(r, R, R, 0, Reg);
      return;

    case 1:
      // ADD k4, dst
      R = opcode.bop[1]&0xf;
      BYTES_TO_GP_REG(Reg->Shifter,0,0,(opcode.bop[1]>>4)&0xf);
      alu(ALU_SHIFTER, R, R, 0, Reg);
      return;

    case 2:
      // ADD k16, src, dst
      R = (opcode.bop[3]>>4)&0xf;
      r = opcode.bop[3]&0xf;
      BYTES_TO_GP_REG(Reg->Shifter,SXMD(MMR),opcode.bop[1],opcode.bop[2]);
      alu(ALU_SHIFTER, r, R, 0, Reg);
      return;

    case 3:
      // ADD Smem, src, dst
      R = (opcode.bop[2]>>4)&0xf;
      r = opcode.bop[2]&0xf;
      alu(ALU_DB, r, R, 0, Reg);
      return;

    case 4:
      // ADD ACx << Tx, ACy
      R = (opcode.bop[1]>>6)&0x3;
      r = (opcode.bop[1]>>4)&0x3;
      t = (opcode.bop[1]>>2)&0x3;
      flag = C54CM(MMR) 
	? (SHFT_DONT_SATURATE | SHFT_NO_OVERFLOW_DETECTION | 
	   SHFT_M40_IS_1 | SHFT_SIGN_EXTEND_USING_SXMD)
	: (SHFT_USE_SATD_SATA | SHFT_SIGN_EXTEND_USING_SXMD | 
	   SHFT_USE_M40 | SHFT_SET_ACOV);
      shifter(r, t, 0, flag, SHFT_SHIFTER, Reg);
      alu(ALU_SHIFTER, R, R, 0, Reg);
      return;

    case 5:
      // ADD ACx << #SHIFTW[, ACy]
      R = (opcode.bop[1]>>6)&0x3;
      r = (opcode.bop[1]>>4)&0x3;
      n = (int)SIGN6BIT_TO_UINT(opcode.bop[2]&0x3f);
      flag = C54CM(MMR) 
	? (SHFT_DONT_SATURATE | SHFT_NO_OVERFLOW_DETECTION | 
	   SHFT_M40_IS_1 | SHFT_SIGN_EXTEND_USING_SXMD)
	: (SHFT_USE_SATD_SATA | SHFT_SIGN_EXTEND_USING_SXMD | 
	   SHFT_USE_M40 | SHFT_SET_ACOV);
      shifter(r, SHFT_CONSTANT, n, flag, SHFT_SHIFTER, Reg);
      alu(ALU_SHIFTER, R, R, 0, Reg);
      return;

    case 6:
      // ADD k16 << #16, [ACx,] ACy
      R = (opcode.bop[3]>>6)&0x3;
      r = (opcode.bop[3]>>4)&0x3;
      BYTES_SHFT16_TO_GP_REG(Reg->Shifter,SXMD(MMR),opcode.bop[1],opcode.bop[2]);
      alu(ALU_SHIFTER, r, R, 0, Reg);
      return;

    case 7:
      // ADD k16 << #SHFT, [ACx,] ACy
      R = (opcode.bop[3]>>4)&0x3;
      r = (opcode.bop[3]>>6)&0x3;
      shift = opcode.bop[3] & 0xf;
      kword = ((Word)opcode.bop[1])<<8 | opcode.bop[2];
      reg_union.guint64 = SXMD(MMR) ? (SWord)kword : kword;
      reg_union.guint64 = reg_union.guint64 << shift;
      Reg->Shifter = reg_union.gp_reg;
      alu(ALU_SHIFTER, r, R, 0, Reg);
      return;
      
    case 8:
      // ADD Smem << Tx, [ACx,] ACy
      R = (opcode.bop[2]>>4)&0x3;
      r = (opcode.bop[2]>>6)&0x3;
      t = (opcode.bop[2]>>2)&0x3;
      flag = C54CM(MMR) ? (0xc5) : (M40(MMR) | 0xc4);
      shifter(SHFT_DB, t, 0, flag, SHFT_SHIFTER, Reg);
      alu(ALU_SHIFTER, R, R, 0, Reg);
      return;

    case 9:
      // ADD Smem << #16, [ACx,] ACy
      R = (opcode.bop[2]>>4)&0x3;
      r = (opcode.bop[2]>>6)&0x3;
      flag = C54CM(MMR) ? (0xc5) : (M40(MMR) | 0xc4);
      shifter(SHFT_DB, SHFT_CONSTANT, 16, flag, SHFT_SHIFTER, Reg);
      alu(ALU_SHIFTER, R, R, 0, Reg);
      return;

    case 10:
      // ADD [uns(]Smem[)], CARRY, [ACx,] ACy
      R = (opcode.bop[2]>>4)&0x3;
      r = (opcode.bop[2]>>6)&0x3;
      flag = (opcode.bop[2]&1) ? (ALU_UNSIGNED_X | ALU_WITH_CARRY) : ALU_WITH_CARRY;
      alu(ALU_DB, r, R, flag, Reg);
      return;

    case 11:
      // ADD [uns(]Smem[)], [ACx,] ACy
      R = (opcode.bop[2]>>4)&0x3;
      r = (opcode.bop[2]>>6)&0x3;
      flag = (opcode.bop[2]&1) ? ALU_UNSIGNED_X : 0;
      alu(ALU_DB, r, R, flag, Reg);
      return;

    case 12:
      // ADD [uns(]Smem[)] << #SHIFTW, [ACx,] ACy
      R = (opcode.bop[3]>>4)&0x3;
      r = (opcode.bop[3]>>6)&0x3;
      shift = (int)SIGN6BIT_TO_UINT(opcode.bop[3]&0x3f);
      flag = C54CM(MMR) 
	? (SHFT_DONT_SATURATE | SHFT_NO_OVERFLOW_DETECTION | 
	   SHFT_M40_IS_1 | SHFT_SIGN_EXTEND_USING_SXMD)
	: (SHFT_USE_SATD_SATA | SHFT_SIGN_EXTEND_USING_SXMD | 
	   SHFT_USE_M40 | SHFT_SET_ACOV);
      shifter(SHFT_DB, SHFT_CONSTANT, shift, flag, SHFT_SHIFTER, Reg);
      flag = (opcode.bop[2]&0x80) ? ALU_UNSIGNED_X : 0;
      alu(ALU_SHIFTER, r, R, flag, Reg);
      return;

    case 13:
      // ADD dbl(Lmem), [ACx,] ACy
      R = (opcode.bop[2]>>4)&0x3;
      r = (opcode.bop[2]>>6)&0x3;
      alu(ALU_DBL, r, R, 0, Reg);
      return;

    case 14:
      // ADD Xmem, Ymem, ACx
      R = (opcode.bop[2])&0x3;
      alu(ALU_DB, ALU_CB, R, 0, Reg);
      return;

    case 15:
      //ADD k16, Smem
      BYTES_SHFT16_TO_GP_REG(Reg->Shifter,SXMD(MMR),opcode.bop[2],opcode.bop[3]);
      alu(ALU_SHIFTER, ALU_DB, ALU_EB, 0, Reg);
      // fixme? ADD #0xfff,*ar1+ should EAB=DAB ?
      Reg->EAB = pipeP->storage2;
      //smem_set_EAB_b2(pipeP,Reg);
      return;

    case 16:
      // ADD dual(Lmem), [ACx,] ACy
      R = (opcode.bop[2]>>4)&0x3;
      r = (opcode.bop[2]>>6)&0x3;
      alu(ALU_DBL, r, R, ALU_DUAL, Reg);
      return;
   
    case 17:
      // ADD dual(Lmem), Tx, ACx
      R = (opcode.bop[2]>>4)&0x3;
      r = ((opcode.bop[2]>>6)&0x3)+4;
      alu(ALU_DBL, r, R, ALU_DUAL, Reg);
      return;
    }
}

static void write_stg(struct _PipeLine *pipeP, struct _Registers *Reg)
{
  if ( pipeP->opcode_subType == 15 )
    {
      write_data_mem_long(Reg->EAB,Reg->EB);
    }
}
