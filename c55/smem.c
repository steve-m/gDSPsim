/*
 * gDSPsim - GNU Digital Signal Processor Simulator
 *
 * Copyright (C) 2001-2002, Kerry Keal, kerry@industrialmusic.com
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
#include <memory.h>

Word circular_update(Word start, int p, SWord step)
{
  Word BK;
  SWord index;
  Word N;

  if ( p < 4 )
    {
      BK = MMR->BK03;
    }
  else
    {
      BK = MMR->BK47;
    }

  // Determine the N LSBs of BK
  N=1;
  while (BK)
    {
      N = N << 1;
      BK = BK >> 1;
    }

  index = start & (N-1);

  if ( (index+step) < 0 )
    index = index + step + BK;
  else if ( (index + step) <= BK )
    index = index + step;
  else
    index = index + step - BK;

  start = (start & ~(N-1)) + index;
  return start;
}

// p is 0-7, pointer register number
// mod is type of modification 0-15, excluding  6 and 7
// b1,b2,b3 are bytes that follow
Word smem_decode(int p, int mod, unsigned char b1, 
		 unsigned char b2, unsigned char b3)
{
  Word *arfP;
  int circ=0;
  int c54cm;
  Word AB;

  // Does it use circular addressing?
  if ( b1==0x9d )
    circ=1;

  arfP=get_pointer_reg(p);

  c54cm = C54CM(MMR);

  switch (mod)
    {
    case 0:
      // *ARn
      AB = *arfP;
      return AB;
    case 1:
      // *ARn+
      if ( circ )
	{
	  AB = *arfP;
	  *arfP = circular_update(*arfP,p,1);
	}
      else
	{
	  AB = *arfP;
	  *arfP = *arfP + 1;
	}
      return AB;
    case 2:
      // *ARn-
      if ( circ )
	{
	  AB = *arfP;
	  *arfP = circular_update(*arfP,p,-1);
	}
      else
	{
	  AB = *arfP;
	  *arfP = *arfP - 1;
	}
      return AB;
    case 3:
      // *(ARn+T0/AR0)
      AB = *arfP;
      if ( c54cm )
	*arfP = *arfP + (SWord)MMR->ar0;
      else
	*arfP = *arfP + (SWord)MMR->T0;
      return AB;
    case 4:
      // *(ARn-T0/AR0)
      AB = *arfP;
      if ( c54cm )
	*arfP = *arfP - (SWord)MMR->ar0;
      else
	*arfP = *arfP - (SWord)MMR->T0;
      return AB;
    case 5:
      // *ARn(T0/AR0)
      if ( c54cm )
	AB = *arfP + (SWord)MMR->ar0;
      else
	AB = *arfP + (SWord)MMR->T0;
      return AB;
    case 6:
      // *ARn(#K16)
      AB = *arfP + (SWord)((((Word)(b1))<<8) + (Word)b2);
      return AB;
    case 7:
      // *+ARn(#K16)
      if ( ARLC(MMR,p) )
	{
	  AB = *arfP;
	  *arfP = circular_update(*arfP,p,(((Word)b1)<<8) + b2);
	  return AB;
	}
      else
	{
	  *arfP = *arfP + (SWord)((((Word)(b1))<<8) + (Word)b2);
	  AB = *arfP;
	  return AB;
	}
    case 8:
      switch (p)
	{
	case 0:
	  AB = (((Word)b1)<<8) + b2;
	  return AB;
	case 1:
	  {
	    int wait_state;
	    AB = read_data_mem_long((((WordA)b1)<<16) + 
						(((WordA)b2)<<8) + b3,
						&wait_state);
	    return AB;
	  }
	case 2:
	  {
	    int wait_state;
	    AB = read_port_mem((((WordA)b1)<<8) + b2,
					    &wait_state);
	    return AB;
	  }
	case 3:
	  AB = MMR->CDP;
	  return AB;
	case 4:
	  AB = MMR->CDP;
	  MMR->CDP = MMR->CDP + 1;
	  return AB;
	case 5:
	  AB = MMR->CDP;
	  MMR->CDP = MMR->CDP - 1;
	  return AB;
	case 6:
	  AB = MMR->CDP + (((Word)b1)<<8) + b2;
	  return AB;
	case 7:
	  MMR->CDP = MMR->CDP + 1;
	  AB = MMR->CDP;
	  return AB;
	}
    case 9:
      AB = *arfP;
       if ( ARMS(MMR) )
	 *arfP = *arfP + 1;
       else
	 *arfP = *arfP + (SWord)MMR->T1;
      return AB;
    case 10:
      AB = *arfP;
       if ( ARMS(MMR) )
	 *arfP = *arfP + 2;
       else
	 *arfP = *arfP - (SWord)MMR->T1;
      return AB;
    case 11:
       if ( ARMS(MMR) )
	 AB = *arfP + 3;
       else
	 AB = *arfP + (SWord)MMR->T1;
       return AB;
    case 12:
       if ( ARMS(MMR) )
	 AB = *arfP + 4;
       else
	 {
	   *arfP = *arfP + 1;
	   AB = *arfP;
	 }
      return AB;
    case 13:
       if ( ARMS(MMR) )
	 AB = *arfP + 5;
       else
	 {
	   *arfP = *arfP - 1;
	   AB = *arfP;
	 }
      return AB;
    case 14:
      if ( ARMS(MMR) )
	AB = *arfP + 6;
      else
	{
	  AB = *arfP;
	  *arfP = bit_reversal(*arfP,(SWord)1,MMR->T0);
	}
      return AB;
    case 15:
      if ( ARMS(MMR) )
	AB = *arfP + 7;
      else
	{
	  AB = *arfP;
	  *arfP = bit_reversal(*arfP,(SWord)-1,MMR->T0);
	}
    }
  // error
  g_error("bad call\n");
  return 0;
}

Word bit_reversal(Word start, Word bit_reversed_one, SWord adjustment)
{
  Word base;
  SWord index,br_index,add;

  bit_reversed_one = bit_reversed_one<<1;

  index = start & (bit_reversed_one-1);

  add=1;
  br_index = 0;
  while (index)
    {
      index=index<<1;
      if ( index & bit_reversed_one )
	{
	  br_index = br_index + add;
	}
      add = add << 1;
    }

  // now we can make the adjustment
  br_index = br_index + adjustment;

  // looped around
  if ( br_index == bit_reversed_one )
    br_index=0;
  if ( br_index < 0 )
    br_index=bit_reversed_one-1;

  // reverse again
  add=1;
  index = 0;
  while (br_index)
    {
      br_index=br_index<<1;
      if ( br_index & bit_reversed_one )
	{
	  index = index + add;
	}
      add = add << 1;
    }

  base = start & ~(bit_reversed_one-1);

  return base+index;

}

// r=0, LO(AC0)=value1
//      LO(AC1)=value2
// r=2, LO(AC2)=value1
//      LO(AC3)=value2
void set_k16_pair_LOreg(int r, Word value1, Word value2, int sign_extend)
{
  if ( r )
    {
      MMR->AC0.wgp.word0 = value1;
      MMR->AC1.wgp.word0 = value2;
    }
  else
    {
      MMR->AC0.wgp.word0 = value1;
      MMR->AC1.wgp.word0 = value2;
    }
}

// r=0, HI(AC0)=value1
//      HI(AC1)=value2
// r=2, HI(AC2)=value1
//      HI(AC3)=value2
void set_k16_pair_HIreg(int r, Word value1, Word value2, int sign_extend)
{
  if ( r )
    {
      MMR->AC0.wgp.word1 = value1;
      MMR->AC1.wgp.word1 = value2;
    }
  else
    {
      MMR->AC0.wgp.word1 = value1;
      MMR->AC1.wgp.word1 = value2;
    }
}

// r register number 0-15
void set_k16_reg(int r, Word value, int sign_extend)
{
  union _GP_Reg_Union reg_union;

  if ( sign_extend )
    reg_union.gint64 = (SWord)value;
  else
    reg_union.guint64 = (Word)value;

  switch ( r )
    {
    case 0:
      MMR->AC0 = reg_union.gp_reg;
      break;
    case 1:
      MMR->AC1 = reg_union.gp_reg;
      break;
    case 2:
      MMR->AC2 = reg_union.gp_reg;
      break;
    case 3:
      MMR->AC3 = reg_union.gp_reg;
      break;
    case 4:
      MMR->T0 = value;
      break;
    case 5:
      MMR->T1 = value;
      break;
    case 6:
      MMR->T2 = value;
      break;
    case 7:
      MMR->T3 = value;
      break;
    case 8:
      MMR->ar0 = value;
      break;
    case 9:
      MMR->ar1 = value;
      break;
    case 10:
      MMR->ar2 = value;
      break;
    case 11:
      MMR->ar3 = value;
      break;
    case 12:
      MMR->ar4 = value;
      break;
    case 13:
      MMR->ar5 = value;
      break;
    case 14:
      MMR->ar6 = value;
      break;
    case 15:
      MMR->ar7 = value;
      break;
    }
}

     
// r register number 0-15, returns low bits for accumulators
Word get_k16_reg(int r)
{
  switch ( r )
    {
    default:
    case 0:
      return MMR->AC0.wgp.word0;
    case 1:
      return MMR->AC1.wgp.word0;
    case 2:
      return MMR->AC2.wgp.word0;
    case 3:
      return MMR->AC3.wgp.word0;
    case 4:
      return MMR->T0;
    case 5:
      return MMR->T1;
    case 6:
      return MMR->T2;
    case 7:
      return MMR->T3;
    case 8:
      return MMR->ar0;
    case 9:
      return MMR->ar1;
    case 10:
      return MMR->ar2;
    case 11:
      return MMR->ar3;
    case 12:
      return MMR->ar4;
    case 13:
      return MMR->ar5;
    case 14:
      return MMR->ar6;
    case 15:
      return MMR->ar7;
    }
}

     
// r register number 0-15, returns hi bits for accumulators
Word get_k16_regHI(int r, int rnd)
{
union _GP_Reg_Union reg_union;

  if ( r > 3 )
    return get_k16_reg(r);

  reg_union.gint64 = 0;

  switch ( r )
    {
    default:
    case 0:
      reg_union.gp_reg = MMR->AC0;
    case 1:
      reg_union.gp_reg = MMR->AC1;
    case 2:
      reg_union.gp_reg = MMR->AC2;
    case 3:
      reg_union.gp_reg = MMR->AC3;
    }

  if ( rnd )
    {
      if ( RDM(MMR) )
	{
	  if ( reg_union.words.low > 0x8000 )
	    {
	      reg_union.guint64 = reg_union.guint64 + 0x8000;
	    }
	  else if ( (reg_union.words.low == 0x8000) && 
		    (reg_union.words.high & 1 ) )
	    {
	      reg_union.guint64 = reg_union.guint64 + 0x8000;
	    }
	  //	  reg_union.words.low = 0;
	}
      else
	{
	  reg_union.guint64 = reg_union.guint64 + 0x8000;
	  //	  reg_union.words.low = 0;
	}
    }

  return reg_union.words.high;

}

// r register number 0-15, returns low bits for accumulators
DWord get_k32_reg(int r)
{
  switch ( r )
    {
    default:
    case 0:
      return MMR->AC0.dword;
    case 1:
      return MMR->AC1.dword;
    case 2:
      return MMR->AC2.dword;
    case 3:
      return MMR->AC3.dword;
    }
}

// r register number 0-3
void set_k32_reg(int r, DWord value, int sign_extend40)
{
  union _GP_Reg_Union reg_union;

  if ( sign_extend40 )
    reg_union.gint64 = (gint32)value;
  else
    reg_union.guint64 = value;;

  switch ( r )
    {
    case 0:
      MMR->AC0 = reg_union.gp_reg;
      break;
    case 1:
      MMR->AC1 = reg_union.gp_reg;
      break;
    case 2:
      MMR->AC2 = reg_union.gp_reg;
      break;
    case 3:
      MMR->AC3 = reg_union.gp_reg;
      break;
    }
}

Word *get_pointer_reg(int p)
{
  switch (p)
    {
    case 0:
    default:
      return &MMR->ar0;
    case 1:
      return &MMR->ar1;
    case 2:
      return &MMR->ar2;
    case 3:
      return &MMR->ar3;
    case 4:
      return &MMR->ar4;
    case 5:
      return &MMR->ar5;
    case 6:
      return &MMR->ar6;
    case 7:
      return &MMR->ar7;
    }
}

// Sets DAB by reading info from 2nd byte
void smem_address_stg_b2(struct _PipeLine *pipeP, struct _Registers *Reg)
{
  Opcode mach_code;
  unsigned char b2;
  int p,mod;

  mach_code = pipeP->decode_nfo.mach_code;
  b2 = mach_code.bop[1];

  if ( b2 & 1 )
    {
      p = b2>>5;
      mod = (b2>>1) & 0xf;
      Reg->DAB = smem_decode(p,mod,mach_code.bop[2],mach_code.bop[3],mach_code.bop[4]);
    }
  else
    {
      // SP offset
      Reg->DAB = MMR->SP + (b2>>1);
    }
}
 
// Sets DAB and CAB from Smem decode
void lmem_address_stg_b2(struct _PipeLine *pipeP, struct _Registers *Reg)
{
  Opcode mach_code;
  unsigned char b2;
  int p,mod;

  mach_code = pipeP->decode_nfo.mach_code;
  b2 = mach_code.bop[1];

  if ( b2 & 1 )
    {
      p = b2>>5;
      mod = (b2>>1) & 0xf;
      Reg->DAB = smem_decode(p,mod,mach_code.bop[2],mach_code.bop[3],mach_code.bop[4]);
      Reg->CAB = Reg->DAB ^ 1;
    }
  else
    {
      // SP offset
      Reg->DAB = MMR->SP + (b2>>1);
      Reg->CAB = Reg->DAB ^ 1;
    }
}
 
void smem_read_stg(struct _PipeLine *pipeP, struct _Registers *Reg)
{
  int wait_state;

  Reg->DB = read_data_mem(Reg->DAB,&wait_state);
}

void smem_read_stg_dbl(struct _PipeLine *pipeP, struct _Registers *Reg)
{
  int wait_state;

  // FIXME need to use Reg->CAB
  Reg->DB = read_data_mem(Reg->DAB,&wait_state);
  Reg->CB = read_data_mem(Reg->DAB^1,&wait_state);
}

void smem_set_EAB_b2(struct _PipeLine *pipeP, struct _Registers *Reg)
{
  Opcode mach_code;
  unsigned char b2;
  int p,mod;

  mach_code = pipeP->decode_nfo.mach_code;
  b2 = mach_code.bop[1];

  if ( b2 & 1 )
    {
      p = b2>>5;
      mod = (b2>>1) & 0xf;
      Reg->EAB = smem_decode(p,mod,mach_code.bop[2],mach_code.bop[3],mach_code.bop[4]);
    }
  else
    {
      // SP offset
      Reg->EAB = MMR->SP + (b2>>1);
    }
}
