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

#include <xymem.h>
#include <smem.h>
#include <decode.h>
#include <memory.h>

Word xmod_decode(int mod, int p)
{
  Word AB;
  Word *arfP;
  int c54cm;
  int circ;

  arfP=get_pointer_reg(p);

  c54cm = C54CM(MMR);
  circ = ARLC(MMR,p);

  switch (mod)
    {
    case 0:
      // *ARn
      AB = *arfP;
      return AB;
    case 1:
      // *ARn+
      AB = *arfP;
      if ( circ )
	{
	  *arfP = circular_update(*arfP,p,1);
	}
      else
	{
	  *arfP = *arfP + 1;
	}
      return AB;
    case 2:
      // *ARn-
      AB = *arfP;
      if ( circ )
	{
	  *arfP = circular_update(*arfP,p,-1);
	}
      else
	{
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
       // *(ARn+T1)
      AB = *arfP;
      *arfP = *arfP + (SWord)MMR->T1;
      return AB;
    case 5:
      // *(ARn-T0/AR0)
      AB = *arfP;
      if ( c54cm )
	*arfP = *arfP - (SWord)MMR->ar0;
      else
	*arfP = *arfP - (SWord)MMR->T0;
      return AB;
    case 6:
      AB = *arfP;
      *arfP = *arfP - (SWord)MMR->T1;
      return AB;
    case 7:
      // *ARn(T0/AR0)
      if ( c54cm )
	AB = *arfP + (SWord)MMR->ar0;
      else
	AB = *arfP + (SWord)MMR->T0;
      return AB;
    }
  return AB;
}

  // Sets CAB and DAB from Xmem and Ymem decode
void xymem_address_stg_b2(struct _PipeLine *pipeP, struct _Registers *Reg)
{
  unsigned int bits;
  int mod,arf;
  Opcode opcode;

  opcode = pipeP->decode_nfo.mach_code;

  // decode Xmem bits
  bits = (opcode.bop[1]>>2) && 0x3f;
  mod = bits & 7;
  arf = bits >> 3;
  Reg->DAB = xmod_decode(mod,arf);

  // decode Ymem bits
  bits = ((opcode.bop[1] & 0x3)<<2) | ((opcode.bop[1]>>4)&0x1f);
  mod = bits & 7;
  arf = bits >> 3;
  Reg->CAB = xmod_decode(mod,arf);

}
  
// Sets DAB from Xmem decode
void xmem_address_stg_b2(struct _PipeLine *pipeP, struct _Registers *Reg)
{
  unsigned int bits;
  int mod,arf;
  Opcode opcode;

  opcode = pipeP->decode_nfo.mach_code;

  // decode Xmem bits
  bits = (opcode.bop[1]>>2) && 0x3f;
  mod = bits & 7;
  arf = bits >> 3;
  Reg->DAB = xmod_decode(mod,arf);
}

  // Sets CAB and DAB from Xmem decode
void xlmem_address_stg_b2(struct _PipeLine *pipeP, struct _Registers *Reg)
{
  unsigned int bits;
  int mod,arf;
  Opcode opcode;

  opcode = pipeP->decode_nfo.mach_code;

  // decode Xmem bits
  bits = (opcode.bop[1]>>2) && 0x3f;
  mod = bits & 7;
  arf = bits >> 3;
  Reg->DAB = xmod_decode(mod,arf);

  Reg->CAB = Reg->DAB ^ 1;
}

// Sets DB
void xymem_read_stg(struct _PipeLine *pipeP, struct _Registers *Reg)
{
  int wait_state;

  Reg->DB = read_data_mem(Reg->DAB,&wait_state);
  Reg->CB = read_data_mem(Reg->CAB,&wait_state);
}

// Sets EAB from Ymem decode of vvvvvvvv vvvvvvyy yyyyvvvv
void ymem_set_EAB_b23(struct _PipeLine *pipeP, struct _Registers *Reg)
{
  unsigned int bits;
  int mod,arf;
  Opcode opcode;

  opcode = pipeP->decode_nfo.mach_code;

  // decode Ymem bits
  bits = ((opcode.bop[1] & 0x3)<<2) | ((opcode.bop[1]>>4)&0x1f);
  mod = bits & 7;
  arf = bits >> 3;
  Reg->EAB = xmod_decode(mod,arf);
}




