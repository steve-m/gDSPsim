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

static void read_stg(struct _PipeLine *pipeP, struct _Registers *Reg);

static gchar *mask[]=
{
  "0100101p 0LLLLLLL", // B L7 
  "0000011p LLLLLLLL LLLLLLLL", // B L16
  "01101010 llllllll llllllll llllllll", // B P24
};

static gchar *opcode[] =
{ 
  "'B' L", // B L7 
  "'B' L", // B L16
  "'B' l", // B P24
};

/* This definition is global because another routine will make have
 * an array that points to all the different instruction classes.
 */
Instruction_Class B_Obj =
{
  "B",
  NULL, // decode
  NULL, // address_stg
  NULL, // access_1
  NULL, // access_2
  read_stg, // read_stg1 (access)
  NULL, // execute
  NULL, // write 
  NULL, // write_plus
  3,
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
      // B L7 
      offset = opcode.bop[1] & 0x7f;
      addrs = Reg->PC + pipeP->decode_nfo.length + offset;
      pipeP->flags = pipeP->flags | PIPE_PC_CHANGED;
      Reg->PC = addrs;
      pipeP->cycles = 6;
      return;
    case 1:
      // B L16 
      offset = (guint16)((((guint16)opcode.bop[1])<<8) | opcode.bop[2] );
      addrs = Reg->PC + pipeP->decode_nfo.length + offset;
      pipeP->flags = pipeP->flags | PIPE_PC_CHANGED;
      Reg->PC = addrs;
      pipeP->cycles = 6;
      return;
    case 2:
      // B P24
      addrs  = ((WordP)opcode.bop[1]<<16) | ((WordP)opcode.bop[2]<<8) |
	((WordP)opcode.bop[3]);
      pipeP->flags = pipeP->flags | PIPE_PC_CHANGED;
      Reg->PC = addrs;
      pipeP->cycles = 6;
      return;
    }
}
