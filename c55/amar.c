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
#include <chip_help.h>

static gchar *mask[]=
  {
    "10110100 ssssssss", // AMAR Smem
    "11101100 ssssssss XXXX1110", // 0100=XSP ?
  };

static gchar *opcode[] = 
{ 
  "'AMAR' s",
  "'AMAR' s,X",
};

static void address_stg(struct _PipeLine *pipeP, struct _Registers *Reg);

Instruction_Class AMAR_Obj =
{
  "AMAR",
  NULL, // decode
  address_stg, // address_stg
  NULL, // access_1
  NULL, // access_2
  NULL, // read_stg
  NULL, // execute
  NULL, // write 
  NULL, // write_plus
  2,
  mask,
  opcode,
};

static void address_stg(struct _PipeLine *pipeP, struct _Registers *Reg)
{
  Opcode opcode;
  int xr;

  smem_address_stg_b2(pipeP,Reg);

  opcode = pipeP->decode_nfo.mach_code;

  if ( pipeP->opcode_subType == 1 )
    {
      xr = (opcode.bop[2]>>4)&0xf;

      set_extended_reg(Reg->DAB,xr,Reg);
    }
}
