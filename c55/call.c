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
static void address_stg(struct _PipeLine *pipeP, struct _Registers *Reg);

static gchar *mask[]=
{
  "0000100p llllllll llllllll", // CALL L16
  "01101001 vbbbbbbb llllllll llllllll llllllll", // CALLCC P24, cond
  "01101100 llllllll llllllll llllllll", // CALL P24
  "01101110 vbbbbbbb llllllll llllllll", // CALLCC L16, cond
  "10010010 vvvvvvrr", // CALL ACx
};

static gchar *opcode[] = 
{ 
  "'CALL' l" , // CALL L16
  "'CALLCC' l,b" , // CALLCC P24, cond
  "'CALL' l" , // CALL P24
  "'CALLCC' l,b" , // CALLCC L16, cond
  "'CALL' r" , // CALL ACx   10 cycles
};

/* This definition is global because another routine will make have
 * an array that points to all the different instruction classes.
 */
Instruction_Class CALL_Obj =
{
  "CALL",
  NULL, // decode
  address_stg, // address_stg
  NULL, // access_1
  NULL, // access_2
  read_stg, // read_stg1 (access)
  NULL, // execute
  NULL, // write 
  NULL, // write_plus
  5,
  mask,
  opcode,
};


static void address_stg(struct _PipeLine *pipeP, struct _Registers *Reg)
{
  Opcode opcode;
  int wait_state;

  opcode = pipeP->decode_nfo.mach_code;
  
  switch ( pipeP->opcode_subType )
    {
    case 0:
    case 2:
    case 4:
      // CALL L16
      MMR->SP--;
      MMR->SSP--;
      Reg->RETA = Reg->PC+pipeP->decode_nfo.length;
      wait_state = write_data_mem_long(MMR->SP,(Word)(Reg->RETA&0xffff));
      wait_state = write_data_mem_long(MMR->SSP,(Word)(Reg->RETA>>16) | (Reg->CFCT >> 4) );
     
      // CFCT
      // whether a single repeat is active
      // whether a conditional single repeat loop is active
      // 5-4 reserved
      // 
      // BRAF
    }
}

static void read_stg(struct _PipeLine *pipeP, struct _Registers *Reg)
{
  Opcode opcode;
  WordP addrs;
  int wait_state;

  opcode = pipeP->decode_nfo.mach_code;
  
  switch ( pipeP->opcode_subType )
    {
    case 0:
      // CALL L16
      addrs  = ((WordP)opcode.bop[1]<<8) | ((WordP)opcode.bop[2]);
      pipeP->flags = pipeP->flags | PIPE_PC_CHANGED;
      Reg->PC = addrs;
      pipeP->cycles = 6;
      break;
    case 1:
      // CALLCC P24, cond
      if ( check_condition(opcode.bop[1]&0x7f,Reg) )
	{
	  MMR->SP--;
	  MMR->SSP--;
	  Reg->RETA = Reg->PC+pipeP->decode_nfo.length;
	  wait_state = write_data_mem_long(MMR->SP,(Word)(Reg->RETA&0xffff));
	  wait_state = write_data_mem_long(MMR->SSP,(Word)(Reg->RETA>>16) | (Reg->CFCT >> 4) );
	  
	  addrs  = ((WordP)opcode.bop[2]<<16) | ((WordP)opcode.bop[3]<<8) |
	    ((WordP)opcode.bop[3]);
	  pipeP->flags = pipeP->flags | PIPE_PC_CHANGED;
	  Reg->PC = addrs;
	}
      pipeP->cycles = 5;
      break;
    case 2:
      // CALL P24
      addrs  = ((WordP)opcode.bop[1]<<16) | ((WordP)opcode.bop[2]<<8) |
	((WordP)opcode.bop[3]);
      pipeP->flags = pipeP->flags | PIPE_PC_CHANGED;
      Reg->PC = addrs;
      pipeP->cycles = 5;
      break;
    case 3:
      // CALLCC L16, cond
      if ( check_condition(opcode.bop[1]&0x7f,Reg) )
	{
	  MMR->SP--;
	  MMR->SSP--;
	  Reg->RETA = Reg->PC+pipeP->decode_nfo.length;
	  wait_state = write_data_mem_long(MMR->SP,(Word)(Reg->RETA&0xffff));
	  wait_state = write_data_mem_long(MMR->SSP,(Word)(Reg->RETA>>16) | (Reg->CFCT >> 4) );
	  
	  addrs  = ((WordP)opcode.bop[2]<<8) | ((WordP)opcode.bop[3]);
	  pipeP->flags = pipeP->flags | PIPE_PC_CHANGED;
	  Reg->PC = addrs;
	  pipeP->cycles = 6;
	}
      else
	{
	  pipeP->cycles = 5;
	}
      break;
    case 4:
      {
	union _GP_Reg_Union reg_union;
	
	reg_union = get_register2(opcode.bop[1]&0x3,0,0);
	addrs = (WordP)reg_union.guint64;
	pipeP->flags = pipeP->flags | PIPE_PC_CHANGED;
	Reg->PC = addrs;
	pipeP->cycles = 10;
	break;
      }
    }
}

