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

#include <chip_core.h>
#include <stdio.h>
#include <chip_help.h>
#include <memory.h>
#include <smem.h>

static void address_stg(struct _PipeLine *pipeP, struct _Registers *Reg);
static void read_stg(struct _PipeLine *pipeP, struct _Registers *Reg);
static void execute(struct _PipeLine *pipeP, struct _Registers *Reg);

static gchar *mask[]=
{
  "0001001p rrrrbb00 RRRRvAvC", // CMP[U] src RELOP dst, TCx
  "0001001E rrrrbb01 RRRR0ACD", // CMPAND[U] src RELOP dst, TCy, TCx
  "0001001E rrrrbb01 RRRR1ACD", // CMPAND[U] src RELOP dst, !TCy, TCx
  "0001001E rrrrbb10 RRRR0ACD", // CMPOR[U] src RELOP dst, TCy, TCx
  "0001001E rrrrbb10 RRRR1ACD", // CMPOR[U] src RELOP dst, !TCy, TCx

  "1111000C ssssssss nnnnnnnn nnnnnnnn", // CMP Smem == K16, TCx
};

static gchar *opcode[] = 
{ 
  "'CMP'A rbR,C" , // CMP[U] src RELOP dst, TCx
  "'CMPAND'A rbR,D,C" , // CMPAND[U] src RELOP dst, TCy, TCx
  "'CMPAND'A rbR,!D,C" , // CMPAND[U] src RELOP dst, !TCy, TCx
  "'CMPOR'A rbR,D,C" , // CMPOR[U] src RELOP dst, TCy, TCx
  "'CMPOR'A rbR,!D,C" , // CMPOR[U] src RELOP dst, !TCy, TCx

  "'CMP' s == n, C", // CMP Smem == K16, TCx
};

/* This definition is global because another routine will make have
 * an array that points to all the different instruction classes.
 */
Instruction_Class CMP_Obj =
{
  "CMP",
  NULL, // decode
  address_stg, // address_stg
  NULL, // access_1
  NULL, // access_2
  read_stg, // read_stg1 (access)
  execute, // execute
  NULL, // write 
  NULL, // write_plus
  6,
  mask,
  opcode,
};

static void address_stg(struct _PipeLine *pipeP, struct _Registers *Reg)
{
  if ( pipeP->opcode_subType == 5 )
    smem_address_stg_b2(pipeP,Reg);
}

static void read_stg(struct _PipeLine *pipeP, struct _Registers *Reg)
{
  if ( pipeP->opcode_subType == 5 )
    smem_read_stg(pipeP,Reg);
}

static void execute(struct _PipeLine *pipeP, struct _Registers *Reg)
{
  Opcode opcode;
  int r,R,TC,TCy,_m40;
  union _GP_Reg_Union reg_union1;
  union _GP_Reg_Union reg_union2;
  
  opcode = pipeP->decode_nfo.mach_code;

  if ( pipeP->opcode_subType < 5 )
    {
      r = (opcode.bop[1]>>4)&0xf;
      R = (opcode.bop[2]>>4)&0xf;

      if ( (r<4) && (R<4) )
	{
	  _m40 = C54CM(MMR) ? 1 : M40(MMR);
	  reg_union1 = get_register2(r,(opcode.bop[2]&0x4)^0x4,_m40);
	  reg_union2 = get_register2(R,(opcode.bop[2]&0x4)^0x4,_m40);
	}
      else
	{
	  if ( opcode.bop[2]&0x4 )
	    {
	      // Unsigned comparison
	      reg_union1.guint64 = get_k16_reg(r);
	      reg_union2.guint64 = get_k16_reg(R);
	    }
	  else
	    {
	      // Unsigned comparison
	      reg_union1.gint64 = (SWord)get_k16_reg(r);
	      reg_union2.gint64 = (SWord)get_k16_reg(R);
	    }

	}

      switch ( (opcode.bop[1]>>2)&3 )
	{
	case 0:
	  if ( reg_union1.gint64 == reg_union2.gint64 )
	    TC=1;
	  else
	    TC=0;
	  break;
	case 1:
	  if ( reg_union1.gint64 < reg_union2.gint64 )
	    TC=1;
	  else
	    TC=0;
	  break;
	case 2:
	  if ( reg_union1.gint64 >= reg_union2.gint64 )
	    TC=1;
	  else
	    TC=0;
	  break;
	case 3:
	  if ( reg_union1.gint64 != reg_union2.gint64 )
	    TC=1;
	  else
	    TC=0;
	  break;
	}

      switch ( pipeP->opcode_subType < 5 )
	{
	case 0:
	  // CMP[U] src RELOP dst, TCx
	  set_TCx(MMR,opcode.bop[2]&1,TC);
	  return;
	case 1:
	  // CMPAND[U] src RELOP dst, TCy, TCx
	  TCy = TCx(MMR,opcode.bop[2]&1);
	  TC = TC & TCy;
	  set_TCx(MMR,(opcode.bop[2]>1)&1,TC);
	  return;
	case 2:
	  // CMPAND[U] src RELOP dst, !TCy, TCx
	  TCy = TCx(MMR,opcode.bop[2]&1);
	  TC = !TC & TCy;
	  set_TCx(MMR,(opcode.bop[2]>1)&1,TC);
	  return;
	case 3:
	  // CMPOR[U] src RELOP dst, !TCy, TCx
	  TCy = TCx(MMR,opcode.bop[2]&1);
	  TC = TC | TCy;
	  set_TCx(MMR,(opcode.bop[2]>1)&1,TC);
	  return;
	case 4:
	  // CMPOR[U] src RELOP dst, TCy, TCx
	  TCy = TCx(MMR,opcode.bop[2]&1);
	  TC = ((!TC)&1) | TCy;
	  set_TCx(MMR,(opcode.bop[2]>1)&1,TC);
	  return;
	}
    }
  else
    {
      FIXME();
    }
}

