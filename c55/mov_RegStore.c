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

// is this decode correct?
// "11011101 ssssssss 00rr1000", // mov dbl(s),r

static gchar *mask[]=
{
  "1100rrrr ssssssss", // MOV src, Smem
  "11100101 ssssssss rrrr01v0", // MOV src, high_byte(Smem)
  "11100101 ssssssss rrrr01v1", // MOV src, low_byte(Smem)
  "101111SS ssssssss", // MOV HI(ACx), Smem
  "11101000 ssssssss rrvvv0vF", // MOV [rnd(]HI(ACx)[)], Smem

  "11100111 ssssssss rrtt00vv", // MOV ACx << Tx, Smem
  "11100111 ssssssss rrtt10vF", // MOV [rnd(]HI(ACx << Tx)[)], Smem
  "11101001 ssssssss rrnnnnnn", // MOV ACx << #SHIFTW, Smem
  "11101010 ssssssss rrnnnnnn", // MOV HI(ACx << #SHIFTW), Smem
  "11111010 ssssssss vvnnnnnn rrvvv0vF", // MOV [rnd(]HI(ACx << #SHIFTW)[)], Smem

  "11101000 ssssssss rrvvv1uF", // MOV [uns(] [rnd(]HI(saturate(ACx))[))], Smem
  "11100111 ssssssss rrtt11uF", // MOV [uns(] [rnd(]HI(saturate(ACx << Tx))[))], Smem
  "11111010 ssssssss uxSHIFTW SSxxx1xF", // MOV [uns(](rnd(]HI(saturate(ACx << #SHIFTW))[))],
  "11101011 ssssssss xxSS10x0", // MOV ACx, dbl(Lmem)
  "11101011 ssssssss xxSS10u1", // MOV [uns(]saturate(ACx)[)], dbl(Lmem)

  "11101011 ssssssss xxSS1101", // MOV ACx >> #1, dual(Lmem)
  "11101011 ssssssss xxSS1110", // MOV pair(HI(ACx)), dbl(Lmem)
  "11101011 ssssssss xxSS1111", // MOV pair(LO(ACx)), dbl(Lmem)
  "11101011 ssssssss FSSS1100", // MOV pair(TAx), dbl(Lmem)
  "10000000 XXXMMMYY YMMM10SS", // MOV ACx, Xmem, Ymem

  };

static gchar *opcode[] = 
{
  "'MOV' r,s",
  "'MOV' r,'high_byte'(s)",
  "'MOV' r,'low_byte'(s)",
  "'MOV' 'HI'(r),s",
  "'MOV' F'HI'(r)G,s",

  "'MOV' r<<t,s",
  "'MOV' F'HI'(r<<t)G,s",
  "'MOV' r<<#n,s",
  "'MOV' 'HI'(r<<#n),s",
  "'MOV' F'HI'(r<<#n)G,s",


};

static void execute(struct _PipeLine *pipeP, struct _Registers *Reg);
static void write_stg(struct _PipeLine *pipeP, struct _Registers *Reg);

Instruction_Class MOV_REG_STORE_Obj =
{
  "MOV_REG_STORE",
  NULL, // decode
  NULL, // address_stg
  NULL, // access_1
  NULL, // access_2
  NULL, // read_stg
  execute, // execute
  write_stg, // write 
  NULL, // write_plus
  20,
  mask,
  opcode,
};

static void execute(struct _PipeLine *pipeP, struct _Registers *Reg)
{
  int r,rnd,t,m40,flag;
  Opcode opcode;
  SWord n;


  opcode = pipeP->decode_nfo.mach_code;

  // affected by C54CM,M40,RDM,SATD,SXMD
  // affect ACOVx
  
  switch ( pipeP->opcode_subType )
    {
    case 0:
      // MOV src, Smem
    case 1:
      // MOV src, high_byte(Smem)
    case 2:
      // MOV src, high_byte(Smem)
    case 3:
      // MOV HI(ACx), Smem
    case 4:
      // MOV [rnd(]HI(ACx)[)], Smem
      smem_set_EAB_b2(pipeP,Reg);
      return;
    case 5:
      // MOV ACx << Tx, Smem
      r = opcode.bop[2]>>6;
      t = (opcode.bop[2]>>4)&0x3;
      shifter(r,t,0,0,r,Reg);
      smem_set_EAB_b2(pipeP,Reg);
      return;
    case 6:
      // MOV [rnd(]HI(ACx << Tx)[)], Smem
      r = opcode.bop[2]>>6;
      t = (opcode.bop[2]>>4)&0x3;
      rnd = opcode.bop[2]&1;
      shifter(r,t,0,rnd,r,Reg);
      smem_set_EAB_b2(pipeP,Reg);
      return;
    case 7:
      // MOV ACx << #SHIFTW, Smem
      r = opcode.bop[2]>>6;
      n = (int)SIGN6BIT_TO_UINT(opcode.bop[2]&0x3f);
      shifter(r,8,n,0,r,Reg);
      smem_set_EAB_b2(pipeP,Reg);
      return;
    case 8:
      // MOV ACx << #SHIFTW, Smem
      r = opcode.bop[2]>>6;
      n = (int)SIGN6BIT_TO_UINT(opcode.bop[2]&0x3f);
      shifter(r,8,n,0,r,Reg);
      smem_set_EAB_b2(pipeP,Reg);
      return;
    case 9:
      // MOV [rnd(]HI(ACx << #SHIFTW)[)], Smem
      r = opcode.bop[3]>>6;
      n = (int)SIGN6BIT_TO_UINT(opcode.bop[2]&0x3f);
      rnd = opcode.bop[3]&1;
      shifter(r,8,n,rnd,r,Reg);
      smem_set_EAB_b2(pipeP,Reg);
      return;
    case 10:
      // MOV [uns(] [rnd(]HI(saturate(ACx))[))], Smem
      smem_set_EAB_b2(pipeP,Reg);
      return;
    case 11:
      // MOV [uns(] [rnd(]HI(saturate(ACx << Tx))[))], Smem
      r = opcode.bop[2]>>6;
      t = (opcode.bop[2]>>4)&0x3;
      m40 = M40(MMR);
      set_M40(MMR,0);
      flag = (opcode.bop[2]&3) | 4;
      shifter(r,t,0,flag,r,Reg);
      set_M40(MMR,m40);

      smem_set_EAB_b2(pipeP,Reg);
      return;



    }
}


static void write_stg(struct _PipeLine *pipeP, struct _Registers *Reg)
{
  Word kword;
  int r;
  Opcode opcode;

  opcode = pipeP->decode_nfo.mach_code;

  switch ( pipeP->opcode_subType )
    {
    case 0:
      // MOV src, Smem
      r = opcode.bop[0] & 0xf;
      kword = get_k16_reg(r);
      write_data_mem_long(Reg->EAB,kword);
      break;
    case 1:
      // MOV src, high_byte(Smem)
      r = opcode.bop[0] & 0xf;
      kword = get_k16_reg(r);
      kword = kword >> 8;
      write_data_mem_long(Reg->EAB,kword);
      break;
    case 2:
      // MOV src, high_byte(Smem)
      r = opcode.bop[0] & 0xf;
      kword = get_k16_reg(r);
      kword = kword & 0xff;
      write_data_mem_long(Reg->EAB,kword);
      break;
    case 3:
      // MOV HI(ACx), Smem
      r = opcode.bop[0] & 0xf;
      kword = get_k16_regHI(r,0);
      write_data_mem_long(Reg->EAB,kword);
      break;
    case 4:
      // MOV [rnd(]HI(ACx)[)], Smem
      r = opcode.bop[2]>>6;
      if ( opcode.bop[2] & 1 )
	kword = get_k16_regHI(r,1);
      else
	kword = get_k16_regHI(r,1);
      write_data_mem_long(Reg->EAB,kword);
      break;
    case 5:
      // MOV ACx << Tx, Smem
      r = opcode.bop[2]>>6;
      kword = get_k16_reg(r);
      write_data_mem_long(Reg->EAB,kword);
      break;
    case 6:
      // MOV [rnd(]HI(ACx << Tx)[)], Smem
      r = opcode.bop[2]>>6;
      if ( opcode.bop[2] & 1 )
	kword = get_k16_regHI(r,1);
      else
	kword = get_k16_regHI(r,1);
      write_data_mem_long(Reg->EAB,kword);
      break;
    case 7:
      // MOV ACx << #SHIFTW, Smem
      r = opcode.bop[2]>>6;
      kword = get_k16_reg(r);
      write_data_mem_long(Reg->EAB,kword);
      break;
    case 8:
      // MOV ACx << #SHIFTW, Smem
      r = opcode.bop[2]>>6;
      kword = get_k16_regHI(r,1);
      write_data_mem_long(Reg->EAB,kword);
      break;
    case 9:
      // MOV [rnd(]HI(ACx << #SHIFTW)[)], Smem
      r = opcode.bop[3]>>6;
      if ( opcode.bop[3] & 1 )
	kword = get_k16_regHI(r,1);
      else
	kword = get_k16_regHI(r,1);
      write_data_mem_long(Reg->EAB,kword);
      break;

    case 10:
    case 11:
    case 12:
      break;
    }
}
