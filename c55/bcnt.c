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
  "0001000p vvrr1010 RRttxxxC", // BCNT ACx, ACy,TCx, Tx
};

static gchar *opcode[] = 
{
  "'BCNT' r,R,T,t", // AND src, dst
};

static void execute(struct _PipeLine *pipeP, struct _Registers *Reg);

Instruction_Class BCNT_Obj =
{
  "BCNT",
  NULL, // decode
  NULL, // address_stg
  NULL, // access_1
  NULL, // access_2
  NULL, // read_stg
  execute, // execute
  NULL, // write 
  NULL, // write_plus
  1,
  mask,
  opcode,
};

static void execute(struct _PipeLine *pipeP, struct _Registers *Reg)
{
  int r,R,Tx;
  Opcode opcode;
  union _GP_Reg_Union reg_union1;
  union _GP_Reg_Union reg_union2;
  Word kword;

  opcode = pipeP->decode_nfo.mach_code;

  r = (opcode.bop[1]>>4)&0x3;
  R = (opcode.bop[2]>>6)&0x3;

  reg_union1 = get_register(r,0);
  reg_union2 = get_register(R,0);
  
  reg_union2.guint64 = reg_union1.guint64 & reg_union2.guint64;

  // now count number of bits that are one
  kword=0;
  while (reg_union2.guint64 > 0 )
    {
      if ( reg_union2.guint64 & 1 )
	kword++;
      reg_union2.guint64 = reg_union2.guint64>>1;
    }

  Tx = (opcode.bop[2]>>4)&0x3;
  set_k16_reg(4+Tx,kword,0);

  if ( kword & 1 )
    {
      set_TCx(MMR,opcode.bop[2]&1,1);
    }
  else
    {
      set_TCx(MMR,opcode.bop[2]&1,0);
    }

}
