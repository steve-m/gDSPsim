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

static void address_stg(struct _PipeLine *pipeP, struct _Registers *Reg);

static gchar *mask[]=
{
  "0000111p LLLLLLLL LLLLLLLL", // RPTB L16
};

static gchar *opcode[] = 
{ 
  "'RPTB' L",
};

/* This definition is global because another routine will make have
 * an array that points to all the different instruction classes.
 */
Instruction_Class RPTB_Obj =
{
  "RPTB",
  NULL, // decode
  address_stg, // address_stg
  NULL, // access_1
  NULL, // access_2
  NULL, // read_stg1 (access)
  NULL, // execute
  NULL, // write 
  NULL, // write_plus
  1,
  mask,
  opcode,
};


static void address_stg(struct _PipeLine *pipeP, struct _Registers *Reg)
{
  Opcode opcode;

  opcode = pipeP->decode_nfo.mach_code;

  if ( C54CM(MMR) )
    {
      if ( BRAF(MMR) )
	{
	  printf("Repeat loop in C54x mode already started %s:%d\n",
		 __FILE__,__LINE__);
	}
      else
	{
	  MMR->RSA0 = Reg->PC + pipeP->decode_nfo.length;
	  MMR->REA0 = ( ((WordP)opcode.bop[1]<<8) | ((WordP)opcode.bop[3]) ) +
	    pipeP->decode_nfo.length + Reg->PC;
	  set_BRAF(MMR,1);
	}
    }
  else
    {
      switch (Reg->CFCT & 0xf)
	{
	case 0:
	  // Both Loops are open
	  MMR->RSA0 = Reg->PC + pipeP->decode_nfo.length;
	  MMR->REA0 = ( ((WordP)opcode.bop[1]<<8) | ((WordP)opcode.bop[3]) ) +
	    pipeP->decode_nfo.length + Reg->PC;
	  Reg->CFCT = Reg->CFCT | 2;
	  break;
	case 2:
	  // Loop 1 is open
	  MMR->RSA1 = Reg->PC + pipeP->decode_nfo.length;
	  MMR->REA1 = ( ((WordP)opcode.bop[1]<<8) | ((WordP)opcode.bop[3]) ) +
	    pipeP->decode_nfo.length + Reg->PC;
	  Reg->CFCT = Reg->CFCT + 5;
	  break;
	default:
	  printf("Error in loop! %s:%d\n",__FILE__,__LINE__);
	  break;
	}
    }
}


