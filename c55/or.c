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

static gchar *mask[]=
{
  "0010101p rrrrRRRR", // OR src, dst
  "0001101p uuuuuuuu RRRRrrrr", // OR k8, src, dst
  "01111110 uuuuuuuu uuuuuuuu RRRRrrrr", // OR k16, src, dst
  "11011010 ssssssss RRRRrrrr", // OR Smem, src, dst
  "0001000p RRrr0001 vvnnnnnn", // OR ACx << #SHIFTW[, ACy]
  
  "01111010 uuuuuuuu uuuuuuuu rrRR011v", // OR k16 << #16, [ACx,] ACy
  "01110011 hhhhhhhh hhhhhhhh rrRRuuuu", // OR k16 << #SHFT, [ACx,] ACy
  "11110101 ssssssss hhhhhhhh hhhhhhhh", // OR k16, Smem
};

static gchar *opcode[] = 
{
  "'OR' r,R", // OR src, dst
  "'OR' #u,r,R", // OR k8, src, dst
  "'OR' #u,r,R", // OR k16, src, dst
  "'OR' s,r,R", // OR Smem, src, dst
  "'OR' r<<#n,R", // OR ACx << #SHIFTW[, ACy]

  "'OR' u<<#16,r,R", // OR k16 << #16, [ACx,] ACy
  "'OR' h<<#u,r,R", // OR k16 << #SHFT, [ACx,] ACy
  "'OR' h,s", // OR k16, Smem
};

static void address_stg(struct _PipeLine *pipeP, struct _Registers *Reg);
static void read_stg(struct _PipeLine *pipeP, struct _Registers *Reg);
static void execute(struct _PipeLine *pipeP, struct _Registers *Reg);
static void write_stg(struct _PipeLine *pipeP, struct _Registers *Reg);

Instruction_Class OR_Obj =
{
  "OR",
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
  if ( pipeP->opcode_subType == 3 )
    {
      smem_address_stg_b2(pipeP,Reg);
      pipeP->storage2 = Reg->DAB;
    }
}

static void read_stg(struct _PipeLine *pipeP, struct _Registers *Reg)
{
  if ( pipeP->opcode_subType == 3 )
    smem_read_stg(pipeP,Reg);
}

static void execute(struct _PipeLine *pipeP, struct _Registers *Reg)
{
  int r,R,shift,flag;
  Opcode opcode;
  SWord n;
  union _GP_Reg_Union reg_union1;
  union _GP_Reg_Union reg_union2;
  Word kword;

  opcode = pipeP->decode_nfo.mach_code;
  
  switch ( pipeP->opcode_subType )
    {
    case 0:
      // OR src, dst
      r = (opcode.bop[1]>>4)&0xf;
      R = opcode.bop[1]&0xf;

      if ( R < 4 )
	{
	  reg_union1 = get_register(r,0);
	  reg_union2 = get_register(R,0);
	  reg_union2.guint64 = reg_union1.guint64 | reg_union2.guint64;
	}
      else
	{
	  reg_union1 = get_register(r,0);
	  reg_union2 = get_register(R,0);
	  kword = reg_union1.words.low | reg_union2.words.low;
	  reg_union2.guint64 = kword;
	}
      set_register(reg_union2,R);
      return;

    case 1:
      // OR k8, src, dst
      R = (opcode.bop[2]>>4)&0xf;
      r = opcode.bop[2]&0xf;

      reg_union1 = get_register(r,0);
      kword = opcode.bop[1];

      reg_union2.guint64 = kword | reg_union1.guint64;
      set_register(reg_union2,R);
      return;

    case 2:
      // OR k16, src, dst
      R = (opcode.bop[3]>>4)&0xf;
      r = opcode.bop[3]&0xf;

      reg_union1 = get_register(r,0);
      kword = ((Word)opcode.bop[1])<<8 | opcode.bop[2];

      reg_union2.guint64 = kword | reg_union1.guint64;
      set_register(reg_union2,R);
      return;

    case 3:
      // OR Smem, src, dst
      R = (opcode.bop[2]>>4)&0xf;
      r = opcode.bop[2]&0xf;

      reg_union1 = get_register(r,0);

      reg_union2.guint64 = Reg->DB | reg_union1.guint64;
      set_register(reg_union2,R);
      return;
      
    case 4:
      // OR ACx << #SHIFTW[, ACy]

      R = (opcode.bop[1]>>6)&0x3;
      r = (opcode.bop[1]>>4)&0x3;
      n = (int)SIGN6BIT_TO_UINT(opcode.bop[2]&0x3f);
      reg_union2 = get_register(R,0); // ACy
      flag = C54CM(MMR) ? (SHFT_M40_IS_1 | SHFT_DONT_SATURATE | SHFT_UNSIGNED |
			   SHFT_NO_OVERFLOW_DETECTION | SHFT_DONT_SET_CARRY) : 
			  (SHFT_USE_M40 | SHFT_DONT_SATURATE | SHFT_UNSIGNED |
			   SHFT_NO_OVERFLOW_DETECTION | SHFT_DONT_SET_CARRY);
      shifter(r,SHFT_CONSTANT,n,flag,R,Reg);
      reg_union1 = get_register(R,0); // ACx << #n
      reg_union2.guint64 = reg_union1.guint64 | reg_union2.guint64;
      set_register(reg_union2,R);
      return;

    case 5:
      // OR k16 << #16, [ACx,] ACy
      R = (opcode.bop[3]>>6)&0x3;
      r = (opcode.bop[3]>>4)&0x3;
      kword = ((Word)opcode.bop[1])<<8 | opcode.bop[2];
      reg_union2.gint64 = SXMD(MMR) ? (SWord)kword : kword;
      reg_union1 = get_register(r,0);
      reg_union2.guint64 = reg_union2.guint64<<16 | reg_union1.guint64;
      set_register(reg_union2,R);
      return;

    case 6:
      // OR k16 << #SHFT, [ACx,] ACy
      R = (opcode.bop[3]>>6)&0x3;
      r = (opcode.bop[3]>>4)&0x3;
      shift = opcode.bop[3] & 0xf;
      kword = ((Word)opcode.bop[1])<<8 | opcode.bop[2];
      reg_union2.guint64 = SXMD(MMR) ? (SWord)kword : kword;
      reg_union1 = get_register(r,0);
      reg_union2.guint64 = reg_union2.guint64<<shift | reg_union1.guint64;
      set_register(reg_union2,R);
      return;
      
    case 7:
      //OR k16, Smem
      kword = ((Word)opcode.bop[2])<<8 | opcode.bop[3];
      pipeP->storage1 = kword | Reg->DB;
      // fixme? OR #0xfff,*ar1+ should EAB=DAB ?
      Reg->EAB = pipeP->storage2;
      //smem_set_EAB_b2(pipeP,Reg);
    }
}


static void write_stg(struct _PipeLine *pipeP, struct _Registers *Reg)
{
  if ( pipeP->opcode_subType == 7 )
    {
      write_data_mem_long(Reg->EAB,pipeP->storage1);
    }
}

