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

static void read_stg(struct _PipeLine *pipeP, struct _Registers *Reg);

static gchar *mask[]=
{
  "01100LLL Lbbbbbbb", // BCC l4, cond
  "0000010p vbbbbbbb LLLLLLLL", // BCC L8, cond
  "01101101 vbbbbbbb LLLLLLLL LLLLLLLL", // BCC L16, cond
  "01101000 vbbbbbbb llllllll llllllll lllllllll", // BCC P24, cond
  //  "01101111 FSSSccxu KKKKKKKK LLLLLLLL", // BCC[U] L8, src RELOP K8
  //  "11111100 ssssssss LLLLLLLL LLLLLLLL", // BCC L16, ARn_mod ! = #0
};

static gchar *opcode[] = 
{ 
  "'BCC' L,b" , // BCC L4, cond
  "'BCC' L,b" , // BCC L8, cond
  "'BCC' L,b" , // BCC L16, cond
  "'BCC' l,b" , // BCC P24, cond
};

/* This definition is global because another routine will make have
 * an array that points to all the different instruction classes.
 */
Instruction_Class BCC_Obj =
{
  "BCC",
  NULL, // decode
  NULL, // address_stg
  NULL, // access_1
  NULL, // access_2
  read_stg, // read_stg1 (access)
  NULL, // execute
  NULL, // write 
  NULL, // write_plus
  4,
  mask,
  opcode,
};

static void read_stg(struct _PipeLine *pipeP, struct _Registers *Reg)
{
  Opcode opcode;
  WordP addrs;
  gint16 offset;

  opcode = pipeP->decode_nfo.mach_code;
  
  switch ( pipeP->opcode_subType )
    {
    case 0:
      // BCC L4, cond
      if ( check_condition(opcode.bop[1]&0x7f,Reg) )
	{
	  offset = ((opcode.bop[0]&0x7)<<1) | ((opcode.bop[1]>>7)&0x1);
	  addrs = Reg->PC + pipeP->decode_nfo.length + offset;
	  pipeP->flags = pipeP->flags | PIPE_PC_CHANGED;
	  Reg->PC = addrs;
	  pipeP->cycles = 5;
	}
      else
	{
	  pipeP->cycles = 6;
	}
      break;
    case 1:
      // BCC L8, cond
      if ( check_condition(opcode.bop[1]&0x7f,Reg) )
	{
	  offset = (signed char)opcode.bop[2];
	  addrs = Reg->PC + pipeP->decode_nfo.length + offset;
	  pipeP->flags = pipeP->flags | PIPE_PC_CHANGED;
	  Reg->PC = addrs;
	  pipeP->cycles = 5;
	}
      else
	{
	  pipeP->cycles = 6;
	}
      break;
    case 2:
      // BCC L16, cond
      if ( check_condition(opcode.bop[1]&0x7f,Reg) )
	{
	  offset = (guint16)((((guint16)opcode.bop[2])<<8) | opcode.bop[3] );
	  addrs = Reg->PC + pipeP->decode_nfo.length + offset;
	  pipeP->flags = pipeP->flags | PIPE_PC_CHANGED;
	  Reg->PC = addrs;
	  pipeP->cycles = 5;
	}
      else
	{
	  pipeP->cycles = 6;
	}
      break;
    case 3:
      // BCC P24, cond
      if ( check_condition(opcode.bop[1]&0x7f,Reg) )
	{
	  addrs  = ((WordP)opcode.bop[2]<<16) | ((WordP)opcode.bop[3]<<8) |
	    ((WordP)opcode.bop[4]);
	  pipeP->flags = pipeP->flags | PIPE_PC_CHANGED;
	  Reg->PC = addrs;
	  pipeP->cycles = 5;
	}
      else
	{
	  pipeP->cycles = 5;
	}
      break;
    }
}

