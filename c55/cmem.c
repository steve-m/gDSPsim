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

#include "cmem.h"

// decode CDP encoding, update pointer and return value of CDP.
Word cmem_decode(int mod)
{
  Word AB;

  switch (mod)
    {
    case 0:
      AB = MMR->CDP;
      return AB;
    case 1:
      AB = MMR->CDP;
      MMR->CDP = MMR->CDP + 1;
      return AB;
    case 2:
      AB = MMR->CDP;
      MMR->CDP = MMR->CDP - 1;
      return AB;
    case 3:
      // *(CDP+T0)
      AB = MMR->CDP;
      MMR->CDP = MMR->CDP + (SWord)MMR->T0;
      return AB;
    default:
      printf("Error in call! %s:%d\n",__FILE__,__LINE__);
      return 0;
    }
}

// Sets EAB based upon Cmem (CDP). Reads the last 2 bits of the
// 3rd byte to figure out how to modify CDP
void cmem_set_EAB_b3(struct _PipeLine *pipeP, struct _Registers *Reg)
{
  unsigned char b3;
  Opcode mach_code;

  mach_code = pipeP->decode_nfo.mach_code;
  b3 = mach_code.bop[2];
  
  Reg->EAB = cmem_decode((int)(b3 & 0x3));
}

// Sets the BAB bus for CDP by decoding last 2 bits of 3rd byte.
void cmem_address_stg_b3(struct _PipeLine *pipeP, struct _Registers *Reg)
{
  unsigned char b3;
  Opcode mach_code;

  mach_code = pipeP->decode_nfo.mach_code;
  b3 = mach_code.bop[2];
  
  Reg->BAB = cmem_decode((int)(b3 & 0x3));
}

