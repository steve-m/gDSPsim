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

// is this decode correct?
// "11011101 ssssssss 00rr1000", // mov dbl(s),r

static gchar *mask[]=
{
  "1010rrrr 000uuuu0", // MOV  u,ACr  bug in manual?
  "1010rrrr 111mmmm0", // MOV -m,ACr  bug in manual?
  "01110110 nnnnnnnn nnnnnnnn rrrr10xx", //  MOV K16, dst (verified)
  "1010rrrr ssssssss", // MOV Smem, dst (verified)
  "11011111 ssssssss rrrr000U", // MOV [uns(]high_byte(Smem)[)], dst
  "11011111 ssssssss rrrr001U", // MOV [uns(]low_byte(Smem)[)], dst
  "01111010 nnnnnnnn nnnnnnnn vvrr101v", // MOV K16 << #16, ACx (verified, can it move to Tx,ARx?)
  "01110101 nnnnnnnn nnnnnnnn vvrruuuu", // MOV K16 << #SHFT, ACx (verified, can it move to Tx,ARx?)
  "11011101 ssssssss vFrrtt11", //  MOV [rnd(]Smem << Tx[)], ACx
  "11100001 ssssssss rrnnnnnn", // MOV low_byte(Smem) << #SHIFTW, ACx
  "11100010 ssssssss rrnnnnnn", // MOV high_byte(Smem) << #SHIFTW, ACx
  "101100rr ssssssss", // MOV Smem << #16, ACx
  "11011111 ssssssss vvrr010U", // MOV [uns(]Smem[)], ACx (verified)
  "11111001 ssssssss Uvnnnnnn vvrr10vv", // MOV [uns(]Smem[)]<<#SHIFTW, ACx
  "11101101 ssssssss vvrr1004", // MOV[40] dbl(Lmem), ACx (verified, for 40)
  "10000001 xxxxxxyy yyyy10rr", // MOV Xmem, Ymem, ACx
  "11101101 ssssssss xxrr101v", // MOV dbl(Lmem), pair(HI(ACx)) (verified)
  "11101101 ssssssss xxrr110v", // MOV dbl(Lmem), pair(LO(ACx)) (verified)
  "11101101 ssssssss 00tt111v", // MOV dbl(Lmem), pair(TAx) (recheck,fixme?)
  };

static gchar *opcode[] = 
{
  "'MOV' u,r",
  "'MOV' m,r",
  "'MOV' m,r",
  "'MOV' s,r",
  "'MOV' U'high_byte'(s)V,r",
  
  "'MOV' U'low_byte'(s)V,r",
  "'MOV' n<<#16,r",
  "'MOV' n<<#u,r",
  "'MOV' Fs<<tG,r",
  "'MOV' 'low_byte'(s)<<#n,r",
  
  "'MOV' 'high_byte'(s)<<#n,r",
  "'MOV' s<<#16,r",
  "'MOV' UsV,r",
  "'MOV' UsV<<#n,r",
  "'MOV'4 'dbl'(s),r",
  
  "'MOV' x,y,r",
  "'MOV' 'dbl'(s),'pair(HI'(r))",
  "'MOV' 'dbl'(s),'pair(LO'(r))",
  "'MOV' 'dbl'(s),'pair'(t)",
};

static void address_stg(struct _PipeLine *pipeP, struct _Registers *Reg);
static void read_stg(struct _PipeLine *pipeP, struct _Registers *Reg);
static void execute(struct _PipeLine *pipeP, struct _Registers *Reg);

Instruction_Class MOV_REG_LOAD_Obj =
{
  "MOV_REG_LOAD",
  NULL, // decode
  address_stg, // address_stg
  NULL, // access_1
  NULL, // access_2
  read_stg, // read_stg
  execute, // execute
  NULL, // write 
  NULL, // write_plus
  20,
  mask,
  opcode,
};

static void address_stg(struct _PipeLine *pipeP, struct _Registers *Reg)
{
  switch ( pipeP->opcode_subType )
    {
    case 3:
    case 4:
    case 5:
    case 8:
    case 9:
    case 10:
    case 11:
    case 12:
    case 13:
    case 14:
    case 16:
    case 17:
    case 18:
      smem_address_stg_b2(pipeP,Reg);
      break;
    case 15:
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
    case 5:
    case 8:
    case 9:
    case 10:
    case 11:
    case 12:
    case 13:
      smem_read_stg(pipeP,Reg);
      break;

    case 14:
    case 16:
    case 17:
    case 18:
      smem_read_stg_dbl(pipeP,Reg);
      break;
    case 15:
      xymem_read_stg(pipeP,Reg);
      break;

    }
}

static void execute(struct _PipeLine *pipeP, struct _Registers *Reg)
{
  Word kword;
  int r,uns,rnd,t,m40;
  Opcode opcode;
  union _GP_Reg_Union reg_union;
  SWord n;
  DWord dword;

  opcode = pipeP->decode_nfo.mach_code;

  // affected by C54CM,M40,RDM,SATD,SXMD
  // affect ACOVx

  switch ( pipeP->opcode_subType )
    {
    case 0:
      // MOV k4,dst
      r = opcode.bop[0] & 0xf;
      kword = (opcode.bop[1] & 0x1e)>>1;
      set_k16_reg(r,kword,1);
      return;
    case 1:
      // MOV -k4,dst
      r = opcode.bop[0] & 0xf;
      kword = 0xfff0 | ((opcode.bop[1] & 0x1e)>>1);
      set_k16_reg(r,kword,1);
      return;
    case 2:
      // MOV k16,dst
      r = opcode.bop[3]>>4;
      kword = (((Word)opcode.bop[1])<<8) | (Word)opcode.bop[2];
      set_k16_reg(r,kword,SXMD(MMR));
      return;
    case 3:
      // MOV Smem,dst
      r = opcode.bop[0] & 0xf;
      set_k16_reg(r,Reg->DB,SXMD(MMR));
      return;
    case 4:
      // MOV [uns(]high_byte(Smem)[)], dst
      r = opcode.bop[2]>>4;
      uns = opcode.bop[2] & 1;
      if ( (uns==0) && (SXMD(MMR) || (r > 3)) )
	{
	  // sign extended if SXMD or 16 bits
	  kword = (SWord)((char)((Word)(Reg->DB >> 8)));
	  set_k16_reg(r,kword,1);
	}
      else
	{
	  kword = Reg->DB >> 8;
	  set_k16_reg(r,kword,0);
	}
      return;

    case 5:
      // MOV [uns(]low_byte(Smem)[)], dst
      r = opcode.bop[2]>>4;
      uns = opcode.bop[2] & 1;
      if ( (uns==0) && (SXMD(MMR) || (r > 3) ))
	{
	  // sign extended if SXMD or 16 bits
	  kword = (SWord)((char)((Word)(Reg->DB & 0xff)));
	  set_k16_reg(r,kword,1);
	}
      else
	{
	  kword = Reg->DB & 0xff;
	  set_k16_reg(r,kword,0);
	}
      return;

    case 6:
      // MOV K16 << #16, ACx
      r = (opcode.bop[3]>>4) && 0x3; // can it move to Tx and ARx ? fixme
      kword = (((Word)opcode.bop[1])<<8) | opcode.bop[2];
      // OK, the only way this can overflow is if M40=0, SXMD=0
      // and we have what would be a negative 16 bit number
      set_k16_reg(r,kword,SXMD(MMR));
      shifter(r,8,16,r,Reg);
      return;

    case 7:
      // MOV K16 << #16, ACx
      r = (opcode.bop[3]>>4) & 0x3; // can it move to Tx and ARx ? fixme
      kword = (((Word)opcode.bop[1])<<8) | opcode.bop[2];
      set_k16_reg(r,kword,SXMD(MMR));
      shifter(r,8,16,r,Reg);
      return;

    case 8:
      // MOV [rnd(]Smem << Tx[)], ACx
      r = (opcode.bop[3]>>4) & 0x3;
      rnd = (opcode.bop[3]&0x40)>>4;
      t = (opcode.bop[3]>>2) & 0x3;
      shifter(16,t+rnd,0,r,Reg);

    case 9:
      // MOV low_byte(Smem) << #SHIFTW, ACx
     r = opcode.bop[2]>>6;
      if ( SXMD(MMR) )
	{
	  // sign extended if SXMD
	  kword = (SWord)((char)((Word)(Reg->DB & 0xff)));
	  set_k16_reg(r,kword,1);
	}
      else
	{
	  kword = Reg->DB & 0xff;
	  set_k16_reg(r,kword,0);
	}
      n = (SWord)SIGN6BIT_TO_UINT(opcode.bop[2] & 0x2f);
      shifter(r,8,kword,r,Reg);
      return;

    case 10:
      // MOV high_byte(Smem) << #SHIFTW, ACx
      r = opcode.bop[2]>>6;
      if ( SXMD(MMR) )
	{
	  // sign extended if SXMD or 16 bits
	  kword = (SWord)((char)((Word)(Reg->DB >> 8)));
	  set_k16_reg(r,kword,1);
	}
      else
	{
	  kword = Reg->DB >> 8;
	  set_k16_reg(r,kword,0);
	}
      n = (SWord)SIGN6BIT_TO_UINT(opcode.bop[2] & 0x2f);
      shifter(r,8,kword,r,Reg);
      return;

    case 11:
      // MOV Smem << #16, ACx
      r = opcode.bop[0]&0x3;
      shifter(16,8,16,r,Reg);
      return;

    case 12:
      // MOV [uns(]Smem[)], ACx
      r = (opcode.bop[2]>>4) & 0x3;
      uns = opcode.bop[2] & 0x1;
      if ( uns )
	set_k16_reg(r,Reg->DB,0);
      else
	set_k16_reg(r,Reg->DB,SXMD(MMR));
      return;

    case 13:
      // MOV [uns(]Smem[)]<<#SHIFTW, ACx
      r = (opcode.bop[3]>>4)&0x3;
      uns = opcode.bop[2]>>7;
      if ( uns )
	set_k16_reg(r,Reg->DB,0);
      else
	set_k16_reg(r,Reg->DB,SXMD(MMR));
      n = (SWord)SIGN6BIT_TO_UINT(opcode.bop[2] & 0x2f);
      shifter(r,8,n,r,Reg);

    case 14:
      // MOV[40] dbl(Lmem), ACx
      r = (opcode.bop[2]>>4)&0x3;
      dword = Reg->DB2;
      m40 = opcode.bop[2]&1;
      set_k32_reg(r,dword,SXMD(MMR) & (M40(MMR) | m40));
      return;

    case 15:
      // MOV Xmem, Ymem, ACx
      reg_union.gint64 = 0;
      reg_union.words.high = Reg->CB; // Ymem
      if ( SXMD(MMR) && (Reg->CB & 0x8000) )
	{
	  reg_union.words.ext = 0xffff;
	}
      reg_union.words.low = Reg->DB; // Ymem
      return;

    case 16:
      // MOV dbl(Lmem), pair(HI(ACx))
      r = (opcode.bop[2]>>4)&0x3;
      dword = Reg->DB2 & 0xffff0000;
      set_k32_reg(r,dword,SXMD(MMR) & M40(MMR) );
      dword = Reg->DB2 << 16;
      r=r+1;
      r=r&3;
      set_k32_reg(r,dword,SXMD(MMR) & M40(MMR) );
      return;

    case 17:
      // MOV dbl(Lmem), pair(LO(ACx))
      r = (opcode.bop[2]>>4)&0x3;
      kword = (DWord)(Reg->DB2 >> 16);
      set_k16_reg(r,kword,SXMD(MMR) );
      kword = Reg->DB2 & 0xffff;
      r=r+1;
      r=r&3;
      set_k16_reg(r,kword,SXMD(MMR) );
      return;

    case 18:
      // MOV dbl(Lmem), pair(TAx)
      t = (opcode.bop[2]>>4)&0x3;
      kword = (DWord)(Reg->DB2 >> 16);
      set_k16_reg(r,kword,SXMD(MMR) );
      kword = Reg->DB2 & 0xffff;
      r=r+1;
      r=r&3;
      set_k16_reg(t+4,kword,0 );
      return;

    }
}


