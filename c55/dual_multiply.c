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

  // H is for all Cmem, U is for both Xmem and Ymem
  "10000010 xxxxxxyy yyyy00cc UHrrRR4f", // MPY[R][40] [uns(]Xmem[)], [uns(]Cmem[)], ACx :: MPY[R][40] [uns(]Ymem[)], [uns(]Cmem[)], ACy
  "10000010 xxxxxxyy yyyy01cc UHrrRR4f", // MAC[R][40] [uns(]Xmem[)], [uns(]Cmem[)], ACx :: MPY[R][40] [uns(]Ymem[)], [uns(]Cmem[)], ACy
  "10000010 xxxxxxyy yyyy10cc UHrrRR4f", // MAS[R][40] [uns(]Xmem[)], [uns(]Cmem[)], ACx :: MPY[R][40] [uns(]Ymem[)], [uns(]Cmem[)], ACy
  "10000010 xxxxxxyy yyyy11cc UHvvRR4f", // AMAR Xmem :: MPY[R][40] [uns(]Ymem[)], [uns(]Cmem[)], ACx
  "10000011 xxxxxxyy yyyy00cc UHrrRR4f", // MAC[R][40] [uns(]Xmem[)], [uns(]Cmem[)], ACx :: MAC[R][40] [uns(]Ymem[)], [uns(]Cmem[)], ACy


  "10000011 xxxxxxyy yyyy01cc UHrrRR4f", // MAS[R][40] [uns(]Xmem[)], [uns(]Cmem[)], ACx :: MAC[R][40] [uns(]Ymem[)], [uns(]Cmem[)], ACy
  "10000011 xxxxxxyy yyyy11cc UHvvRR4f", // AMAR Xmem :: MAC[R][40] [uns(]Ymem[)], [uns(]Cmem[)], ACx
  "10000101 xxxxxxyy yyyy01cc UHrrRR4f", // MAS[R][40] [uns(]Xmem[)], [uns(]Cmem[)], ACx :: MAS[R][40] [uns(]Ymem[)], [uns(]Cmem[)], ACy
  "10000101 xxxxxxyy yyyy00cc UHvvRR4f", // AMAR Xmem :: MAS[R][40] [uns(]Ymem[)], [uns(]Cmem[)], ACx
  "10000011 xxxxxxyy yyyy10cc UHrrRR4f", // MAC[R][40] [uns(]Xmem[)], [uns(]Cmem[)], ACx >> #16 :: MAC[R][40] [uns(]Ymem[)], [uns(]Cmem[)], ACy

  "10000100 xxxxxxyy yyyy10cc UHrrRR4f", // MPY[R][40] [uns(]Xmem[)], [uns(]Cmem[)], ACx :: MAC[R][40] [uns(]Ymem[)], [uns(]Cmem[)], ACy >> #16
  "10000100 xxxxxxyy yyyy11cc UHrrRR4f", // MAC[R][40] [uns(]Xmem[)], [uns(]Cmem[)], ACx >> #16 :: MAC[R][40] [uns(]Ymem[)], [uns(]Cmem[)], ACy >> #16
  "10000100 xxxxxxyy yyyy00cc UHrrRR4f", // MAS[R][40] [uns(]Xmem[)], [uns(]Cmem[)], ACx :: MAC[R][40] [uns(]Ymem[)], [uns(]Cmem[)], ACy >> #16
  "10000100 xxxxxxyy yyyy01cc UHvvRR4f", // AMAR Xmem :: MAC[R][40] [uns(]Ymem[)], [uns(]Cmem[)], ACx >> #16
  "10000101 xxxxxxyy yyyy10cc vvvvvvvv", // AMAR Xmem, Ymem, Cmem
};

static gchar *opcode[] = 
{ 
  // MPY[R][40] [uns(]Xmem[)], [uns(]Cmem[)], ACx :: MPY[R][40] [uns(]Ymem[)], [uns(]Cmem[)], ACy
  "'MPY'f4 UxV,HcI,r '|| MPY'f4 UyV,HcI,R", 
  // MAC[R][40] [uns(]Xmem[)], [uns(]Cmem[)], ACx :: MPY[R][40] [uns(]Ymem[)], [uns(]Cmem[)], ACy
  "'MAC'f4 UxV,HcI,r '|| MPY'f4 UyV,HcI,R", 
  // MAS[R][40] [uns(]Xmem[)], [uns(]Cmem[)], ACx :: MPY[R][40] [uns(]Ymem[)], [uns(]Cmem[)], ACy
  "'MAS'f4 UxV,HcI,r '|| MPY'f4 UyV,HcI,R", 
  // AMAR Xmem :: MPY[R][40] [uns(]Ymem[)], [uns(]Cmem[)], ACx
  "'AMAR' x '|| MPY'f4 UyV,HcI,R", 
  // MAC[R][40] [uns(]Xmem[)], [uns(]Cmem[)], ACx :: MAC[R][40] [uns(]Ymem[)], [uns(]Cmem[)], ACy
  "'MAC'f4 UxV,HcI,r '|| MAC'f4 UyV,HcI,R", 

  // MAS[R][40] [uns(]Xmem[)], [uns(]Cmem[)], ACx :: MAC[R][40] [uns(]Ymem[)], [uns(]Cmem[)], ACy
  "'MAS'f4 UxV,HcI,r '|| MAC'f4 UyV,HcI,R", 
  // AMAR Xmem :: MAC[R][40] [uns(]Ymem[)], [uns(]Cmem[)], ACx
  "'AMAR' x '|| MAC'f4 UyV,HcI,R", 
  // MAS[R][40] [uns(]Xmem[)], [uns(]Cmem[)], ACx :: MAS[R][40] [uns(]Ymem[)], [uns(]Cmem[)], ACy
  "'MAS'f4 UxV,HcI,r '|| MAS'f4 UyV,HcI,R", 
  // AMAR Xmem :: MAS[R][40] [uns(]Ymem[)], [uns(]Cmem[)], ACx
  "'AMAR' x '|| MAS'f4 UyV,HcI,R", 
  // MAC[R][40] [uns(]Xmem[)], [uns(]Cmem[)], ACx >> #16 :: MAC[R][40] [uns(]Ymem[)], [uns(]Cmem[)], ACy
  "'MAC'f4 UxV,HcI,r'>>#16 || MAC'f4 UyV,HcI,R", 

  // MPY[R][40] [uns(]Xmem[)], [uns(]Cmem[)], ACx :: MAC[R][40] [uns(]Ymem[)], [uns(]Cmem[)], ACy >> #16
  "'MPY'f4 UxV,HcI,r '|| MAC'f4 UyV,HcI,R'>>#16'", 
  // MAC[R][40] [uns(]Xmem[)], [uns(]Cmem[)], ACx >> #16 :: MAC[R][40] [uns(]Ymem[)], [uns(]Cmem[)], ACy >> #16
  "'MAC'f4 UxV,HcI,r'>>#16 || MAC'f4 UyV,HcI,R'>>#16'", 
  // MAS[R][40] [uns(]Xmem[)], [uns(]Cmem[)], ACx :: MAC[R][40] [uns(]Ymem[)], [uns(]Cmem[)], ACy >> #16
  "'MAS'f4 UxV,HcI,r '|| MAC'f4 UyV,HcI,R'>>#16'", 
  // AMAR Xmem :: MAC[R][40] [uns(]Ymem[)], [uns(]Cmem[)], ACx >> #16
  "'AMAR' x '|| MAC'f4 UyV,HcI,R'>>#16'", 
  // AMAR Xmem, Ymem, Cmem
  "'AMAR' x,y,c",
};

static void address_stg(struct _PipeLine *pipeP, struct _Registers *Reg);
static void read_stg(struct _PipeLine *pipeP, struct _Registers *Reg);
static void execute(struct _PipeLine *pipeP, struct _Registers *Reg);

Instruction_Class DUAL_MULTIPLY_Obj =
{
  "DUAL_MULTIPLY",
  NULL, // decode
  address_stg, // address_stg
  NULL, // access_1
  NULL, // access_2
  read_stg, // read_stg
  execute, // execute
  NULL, // write 
  NULL, // write_plus
  15,
  mask,
  opcode,
};

static void address_stg(struct _PipeLine *pipeP, struct _Registers *Reg)
{
  xymem_address_stg_b2(pipeP,Reg);
  cmem_address_stg_b3(pipeP,Reg);
}

static void read_stg(struct _PipeLine *pipeP, struct _Registers *Reg)
{
  cmem_read_stg(pipeP,Reg);
  xymem_read_stg(pipeP,Reg);
}

static void execute(struct _PipeLine *pipeP, struct _Registers *Reg)
{
  Opcode opcode;
  int r,R,flag;

  opcode = pipeP->decode_nfo.mach_code;

  switch ( pipeP->opcode_subType )
    {
    case 0:
      // MPY[R][40] [uns(]Xmem[)], [uns(]Cmem[)], ACx :: MPY[R][40] [uns(]Ymem[)], [uns(]Cmem[)], ACy
      r=(opcode.bop[3]>>4)&0x3;
      R=(opcode.bop[3]>>2)&0x3;
      flag = (opcode.bop[3]&0x1) ? MULT_ROUND : 0;
      flag = (opcode.bop[3]&0x80) ? (flag | MULT_UNSIGNED_X) : flag;
      flag = (opcode.bop[3]&0x40) ? (flag | MULT_UNSIGNED_Y) : flag;
      flag = (opcode.bop[3]&0x2) ? (flag | MULT_M40_IS_1) : flag;
      multiplier(MULT_DB,MULT_BB,MULT_NO_MAC,r,flag,Reg);
      multiplier(MULT_CB,MULT_BB,MULT_NO_MAC,R,flag,Reg);
      break;

    case 1:
      // MAC[R][40] [uns(]Xmem[)], [uns(]Cmem[)], ACx :: MPY[R][40] [uns(]Ymem[)], [uns(]Cmem[)], ACy
      r=(opcode.bop[3]>>4)&0x3;
      R=(opcode.bop[3]>>2)&0x3;
      flag = (opcode.bop[3]&0x1) ? MULT_ROUND : 0;
      flag = (opcode.bop[3]&0x80) ? (flag | MULT_UNSIGNED_X) : flag;
      flag = (opcode.bop[3]&0x40) ? (flag | MULT_UNSIGNED_Y) : flag;
      flag = (opcode.bop[3]&0x2) ? (flag | MULT_M40_IS_1) : flag;
      multiplier(MULT_DB,MULT_BB,r,r,flag,Reg);
      multiplier(MULT_CB,MULT_BB,MULT_NO_MAC,R,flag,Reg);
      break;

    case 2:
      // MAS[R][40] [uns(]Xmem[)], [uns(]Cmem[)], ACx :: MPY[R][40] [uns(]Ymem[)], [uns(]Cmem[)], ACy
      r=(opcode.bop[3]>>4)&0x3;
      R=(opcode.bop[3]>>2)&0x3;
      flag = (opcode.bop[3]&0x1) ? MULT_ROUND : 0;
      flag = (opcode.bop[3]&0x80) ? (flag | MULT_UNSIGNED_X) : flag;
      flag = (opcode.bop[3]&0x40) ? (flag | MULT_UNSIGNED_Y) : flag;
      flag = (opcode.bop[3]&0x2) ? (flag | MULT_M40_IS_1) : flag;
      multiplier(MULT_DB,MULT_BB,r,r,flag | MULT_SUBTRACT,Reg);
      multiplier(MULT_CB,MULT_BB,MULT_NO_MAC,R,flag,Reg);
      break;

    case 3:
      // AMAR Xmem :: MPY[R][40] [uns(]Ymem[)], [uns(]Cmem[)], ACx
      R=(opcode.bop[3]>>2)&0x3;
      flag = (opcode.bop[3]&0x1) ? MULT_ROUND : 0;
      flag = (opcode.bop[3]&0x80) ? (flag | MULT_UNSIGNED_X) : flag;
      flag = (opcode.bop[3]&0x40) ? (flag | MULT_UNSIGNED_Y) : flag;
      flag = (opcode.bop[3]&0x2) ? (flag | MULT_M40_IS_1) : flag;
      multiplier(MULT_CB,MULT_BB,MULT_NO_MAC,R,flag,Reg);
      break;

    case 4:
      // MAC[R][40] [uns(]Xmem[)], [uns(]Cmem[)], ACx :: MAC[R][40] [uns(]Ymem[)], [uns(]Cmem[)], ACy
      r=(opcode.bop[3]>>4)&0x3;
      R=(opcode.bop[3]>>2)&0x3;
      flag = (opcode.bop[3]&0x1) ? MULT_ROUND : 0;
      flag = (opcode.bop[3]&0x80) ? (flag | MULT_UNSIGNED_X) : flag;
      flag = (opcode.bop[3]&0x40) ? (flag | MULT_UNSIGNED_Y) : flag;
      flag = (opcode.bop[3]&0x2) ? (flag | MULT_M40_IS_1) : flag;
      multiplier(MULT_DB,MULT_BB,r,r,flag,Reg);
      multiplier(MULT_CB,MULT_BB,R,R,flag,Reg);
      break;

   case 5:
     // MAS[R][40] [uns(]Xmem[)], [uns(]Cmem[)], ACx :: MAC[R][40] [uns(]Ymem[)], [uns(]Cmem[)], ACy
      r=(opcode.bop[3]>>4)&0x3;
      R=(opcode.bop[3]>>2)&0x3;
      flag = (opcode.bop[3]&0x1) ? MULT_ROUND : 0;
      flag = (opcode.bop[3]&0x80) ? (flag | MULT_UNSIGNED_X) : flag;
      flag = (opcode.bop[3]&0x40) ? (flag | MULT_UNSIGNED_Y) : flag;
      flag = (opcode.bop[3]&0x2) ? (flag | MULT_M40_IS_1) : flag;
      multiplier(MULT_DB,MULT_BB,r,r,flag | MULT_SUBTRACT,Reg);
      multiplier(MULT_CB,MULT_BB,R,R,flag,Reg);
      break;

    case 6:
      // AMAR Xmem :: MAC[R][40] [uns(]Ymem[)], [uns(]Cmem[)], ACx
      R=(opcode.bop[3]>>2)&0x3;
      flag = (opcode.bop[3]&0x1) ? MULT_ROUND : 0;
      flag = (opcode.bop[3]&0x80) ? (flag | MULT_UNSIGNED_X) : flag;
      flag = (opcode.bop[3]&0x40) ? (flag | MULT_UNSIGNED_Y) : flag;
      flag = (opcode.bop[3]&0x2) ? (flag | MULT_M40_IS_1) : flag;
      multiplier(MULT_CB,MULT_BB,R,R,flag,Reg);
      break;

    case 7:
      // MAS[R][40] [uns(]Xmem[)], [uns(]Cmem[)], ACx :: MAS[R][40] [uns(]Ymem[)], [uns(]Cmem[)], ACy
      r=(opcode.bop[3]>>4)&0x3;
      R=(opcode.bop[3]>>2)&0x3;
      flag = (opcode.bop[3]&0x1) 
	? (MULT_ROUND | MULT_SUBTRACT) : MULT_SUBTRACT;
      flag = (opcode.bop[3]&0x80) ? (flag | MULT_UNSIGNED_X) : flag;
      flag = (opcode.bop[3]&0x40) ? (flag | MULT_UNSIGNED_Y) : flag;
      flag = (opcode.bop[3]&0x2) ? (flag | MULT_M40_IS_1) : flag;
      multiplier(MULT_DB,MULT_BB,r,r,flag,Reg);
      multiplier(MULT_CB,MULT_BB,R,R,flag,Reg);
      break;

    case 8:
      // AMAR Xmem :: MAS[R][40] [uns(]Ymem[)], [uns(]Cmem[)], ACx
      R=(opcode.bop[3]>>2)&0x3;
      flag = (opcode.bop[3]&0x1) 
	? (MULT_ROUND | MULT_SUBTRACT) : MULT_SUBTRACT;
      flag = (opcode.bop[3]&0x80) ? (flag | MULT_UNSIGNED_X) : flag;
      flag = (opcode.bop[3]&0x40) ? (flag | MULT_UNSIGNED_Y) : flag;
      flag = (opcode.bop[3]&0x2) ? (flag | MULT_M40_IS_1) : flag;
      multiplier(MULT_CB,MULT_BB,R,R,flag,Reg);
      break;

    case 9:
      // MAC[R][40] [uns(]Xmem[)], [uns(]Cmem[)], ACx >> #16 :: MAC[R][40] [uns(]Ymem[)], [uns(]Cmem[)], ACy
      r=(opcode.bop[3]>>4)&0x3;
      R=(opcode.bop[3]>>2)&0x3;
      flag = (opcode.bop[3]&0x1) ? MULT_ROUND : 0;
      flag = (opcode.bop[3]&0x80) ? (flag | MULT_UNSIGNED_X) : flag;
      flag = (opcode.bop[3]&0x40) ? (flag | MULT_UNSIGNED_Y) : flag;
      flag = (opcode.bop[3]&0x2) ? (flag | MULT_M40_IS_1) : flag;
      multiplier(MULT_DB,MULT_BB,r,r,flag | MULT_RIGHT_SHIFT_MAC,Reg);
      multiplier(MULT_CB,MULT_BB,R,R,flag,Reg);
      break;

    case 10:
      // MPY[R][40] [uns(]Xmem[)], [uns(]Cmem[)], ACx :: MAC[R][40] [uns(]Ymem[)], [uns(]Cmem[)], ACy >> #16
      r=(opcode.bop[3]>>4)&0x3;
      R=(opcode.bop[3]>>2)&0x3;
      flag = (opcode.bop[3]&0x1) ? MULT_ROUND : 0;
      flag = (opcode.bop[3]&0x80) ? (flag | MULT_UNSIGNED_X) : flag;
      flag = (opcode.bop[3]&0x40) ? (flag | MULT_UNSIGNED_Y) : flag;
      flag = (opcode.bop[3]&0x2) ? (flag | MULT_M40_IS_1) : flag;
      multiplier(MULT_DB,MULT_BB,MULT_NO_MAC,r,flag,Reg);
      multiplier(MULT_CB,MULT_BB,R,R,flag | MULT_RIGHT_SHIFT_MAC,Reg);
      break;

    case 11:
      // MAC[R][40] [uns(]Xmem[)], [uns(]Cmem[)], ACx >> #16 :: MAC[R][40] [uns(]Ymem[)], [uns(]Cmem[)], ACy >> #16
      r=(opcode.bop[3]>>4)&0x3;
      R=(opcode.bop[3]>>2)&0x3;
      flag = (opcode.bop[3]&0x1) 
	? (MULT_ROUND | MULT_RIGHT_SHIFT_MAC) : MULT_RIGHT_SHIFT_MAC;
      flag = (opcode.bop[3]&0x80) ? (flag | MULT_UNSIGNED_X) : flag;
      flag = (opcode.bop[3]&0x40) ? (flag | MULT_UNSIGNED_Y) : flag;
      flag = (opcode.bop[3]&0x2) ? (flag | MULT_M40_IS_1) : flag;
      multiplier(MULT_DB,MULT_BB,r,r,flag,Reg);
      multiplier(MULT_CB,MULT_BB,R,R,flag,Reg);
      break;

    case 12:
      // MAS[R][40] [uns(]Xmem[)], [uns(]Cmem[)], ACx :: MAC[R][40] [uns(]Ymem[)], [uns(]Cmem[)], ACy >> #16
      r=(opcode.bop[3]>>4)&0x3;
      R=(opcode.bop[3]>>2)&0x3;
      flag = (opcode.bop[3]&0x1) ? MULT_ROUND : 0;
      flag = (opcode.bop[3]&0x80) ? (flag | MULT_UNSIGNED_X) : flag;
      flag = (opcode.bop[3]&0x40) ? (flag | MULT_UNSIGNED_Y) : flag;
      flag = (opcode.bop[3]&0x2) ? (flag | MULT_M40_IS_1) : flag;
      multiplier(MULT_DB,MULT_BB,r,r,flag | MULT_SUBTRACT,Reg);
      multiplier(MULT_CB,MULT_BB,R,R,flag | MULT_RIGHT_SHIFT_MAC,Reg);
      break;

    case 13:
      // AMAR Xmem :: MAC[R][40] [uns(]Ymem[)], [uns(]Cmem[)], ACx >> #16
      R=(opcode.bop[3]>>2)&0x3;
      flag = (opcode.bop[3]&0x1) 
	? (MULT_ROUND | MULT_RIGHT_SHIFT_MAC) : MULT_RIGHT_SHIFT_MAC;
      flag = (opcode.bop[3]&0x80) ? (flag | MULT_UNSIGNED_X) : flag;
      flag = (opcode.bop[3]&0x40) ? (flag | MULT_UNSIGNED_Y) : flag;
      flag = (opcode.bop[3]&0x2) ? (flag | MULT_M40_IS_1) : flag;
      multiplier(MULT_CB,MULT_BB,R,R,flag,Reg);
    }
}
