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
#include <smem.h>

static gchar *mask[]=
  {
    "11010011 ssssssss 3frr10vv", // SQRM[R] [T3 = ]Smem, ACx
  };

static gchar *opcode[] = 
{ 
  "'SQRM'f 3s,r",
};

static void execute(struct _PipeLine *pipeP, struct _Registers *Reg);

Instruction_Class SQRM_Obj =
{
  "SQRM",
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
  int r,flag;

  opcode = pipeP->decode_nfo.mach_code;

  r=(opcode.bop[2]>>4)&0x3;
  flag = (opcode.bop[2]&0x40) ? MULT_ROUND : 0;
  
  multiplier(MULT_DB,MULT_DB,MULT_NO_MAC,r,flag,Reg);
  
  if ( opcode.bop[2]&0x80 )
    {
      MMR->T3 = Reg->DB;
    }
}


