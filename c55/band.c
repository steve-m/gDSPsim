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
  "1111001C ssssssss hhhhhhhh hhhhhhhh", //    BAND Smem, k16, TCx
};

static gchar *opcode[] = 
{
  "'BAND' s,h,C", // BAND Smem, k16, TCx
};

static void execute(struct _PipeLine *pipeP, struct _Registers *Reg);

Instruction_Class BAND_Obj =
{
  "BAND",
  NULL, // decode
  smem_address_stg_b2, // address_stg
  NULL, // access_1
  NULL, // access_2
  smem_read_stg, // read_stg
  execute, // execute
  NULL, // write 
  NULL, // write_plus
  1,
  mask,
  opcode,
};

static void execute(struct _PipeLine *pipeP, struct _Registers *Reg)
{
  Opcode opcode;
  Word kword;

  opcode = pipeP->decode_nfo.mach_code;

  kword = (Word)opcode.bop[2]<<16 | (Word)opcode.bop[3];

  if ( kword & Reg->DB )
    {
      set_TCx(MMR,opcode.bop[0]&1,1);
    }
  else
    {
      set_TCx(MMR,opcode.bop[0]&1,0);
    }

}
