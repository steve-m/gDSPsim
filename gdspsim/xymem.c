/*
 * gDSPsim - GNU Digital Signal Processor Simulator
 *
 * Copyright (C) 2001, Kerry Keal, kerry@industrialmusic.com
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

#include "xymem.h"
#include "smem.h"
#include "hardware.h"
#include "memory.h"



// This updates the auxillary registers and returns the read address
// Xdecode == 0, use low 4 bits, Xdecode != 0 use high 4 bits
WordA update_xymem( Word XYmem, int Xdecode, struct _Registers *Reg)
{
  int mod,ar,bits;
  Word *arfP;
  WordA ReadAddress;

  if ( Xdecode )
    {
      bits = (XYmem & 0xf0)>>4;
    }
  else
    {
      bits = XYmem & 0xf;
    }
  ar = (bits & 3)+2;
  mod = (bits & 12)>>2;

  arfP=get_pointer_reg(ar,Reg, CMPT(MMR));

  switch ( mod )
    {
    case 0:
      // *ARx
      return *arfP;
    case 1:
      // *ARx-
      ReadAddress = *arfP;
      *arfP = *arfP - 1;
      return ReadAddress;
    case 2:
      // *ARx+
      ReadAddress = *arfP;
      *arfP = *arfP + 1;
      return ReadAddress;
    case 3:
      // *ARx+0%
      ReadAddress = *arfP;
      *arfP = circular_update(*arfP,(SWord)MMR->ar0,MMR->BK);
      return ReadAddress;
    }

  g_return_val_if_fail(0,0);
  
}

void xymem_read_stg1(struct _PipeLine *pipeP, struct _Registers *Reg)
{
  Reg->DAB = update_xymem(pipeP->current_opcode, 1, Reg);
  Reg->CAB = update_xymem(pipeP->current_opcode, 0, Reg);
}
void xymem_read_stg2(struct _PipeLine *pipeP, struct _Registers *Reg)
{
  int wait_state;
  
  Reg->DB = read_data_mem(Reg->DAB,&wait_state);
  Reg->CB = read_data_mem(Reg->DAB,&wait_state);
}
void xmem_read_stg1(struct _PipeLine *pipeP, struct _Registers *Reg)
{
  Reg->DAB = update_xymem(pipeP->current_opcode, 1, Reg);
}
void xmem_read_stg2(struct _PipeLine *pipeP, struct _Registers *Reg)
{
  int wait_state;
  
  Reg->DB = read_data_mem(Reg->DAB,&wait_state);
}

