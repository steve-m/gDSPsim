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
#include <memory.h>
#include <chip_help.h>

static gchar *mask[]=
{
  "11100011 ssssssss uuuu01Cv", // BTSTCLR k4, Smem, TCx
};

static gchar *opcode[] = 
{
  "'BTSTCLR' u,s,C", // BTSTCLR k4, Smem, TCx
};

static void address_stg(struct _PipeLine *pipeP, struct _Registers *Reg);
static void execute(struct _PipeLine *pipeP, struct _Registers *Reg);
static void write_stg(struct _PipeLine *pipeP, struct _Registers *Reg);

Instruction_Class BTSTCLR_Obj =
{
  "BTSTCLR",
  NULL, // decode
  address_stg, // address_stg
  NULL, // access_1
  NULL, // access_2
  smem_read_stg, // read_stg
  execute, // execute
  write_stg, // write 
  NULL, // write_plus
  1,
  mask,
  opcode,
};

static void address_stg(struct _PipeLine *pipeP, struct _Registers *Reg)
{
  smem_address_stg_b2(pipeP,Reg);
  pipeP->storage2 = Reg->DAB;
}

static void execute(struct _PipeLine *pipeP, struct _Registers *Reg)
{
  Opcode opcode;
  Word bit;
  
  opcode = pipeP->decode_nfo.mach_code;
  
  // BTSTCLR k4, Smem, TC1
  bit = (opcode.bop[2]>>4)&0xf;
  if ( (1<<bit) & Reg->DB )
    {
      set_TCx(MMR,(opcode.bop[2]>>1)&1,1);
    }
  else
    {
      set_TCx(MMR,(opcode.bop[2]>>1)&1,0);
    }
  pipeP->storage1 = Reg->DB & ~(1<<bit);
  
}

static void write_stg(struct _PipeLine *pipeP, struct _Registers *Reg)
{
  if ( pipeP->opcode_subType==4 )
    { 
      write_data_mem_long(pipeP->storage2,pipeP->storage1);
    } 
}
