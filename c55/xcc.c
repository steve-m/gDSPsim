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
#include <chip_help.h>

static gchar *mask[]=
{
  "10010110 0bbbbbbb", // XCC [label, ]cond
  "10010110 1bbbbbbb", // XCCPART [label, ]cond
  "10011110 0bbbbbbb", // XCC [label, ]cond     (prefered)
  "10011110 1bbbbbbb", // XCCPART [label, ]cond
  "10011111 0bbbbbbb", // XCC [label, ]cond

  "10011111 1bbbbbbb", // XCCPART [label, ]cond
};

static gchar *opcode[] = 
{
  "'XCC' b", // XCC [label, ]cond
  "'XCCPART' b", // XCC [label, ]cond
  "'XCC' b", // XCC [label, ]cond
  "'XCCPART' b", // XCC [label, ]cond
  "'XCC' b", // XCC [label, ]cond

  "'XCCPART' b", // XCC [label, ]cond
};

static void address_stg(struct _PipeLine *pipeP, struct _Registers *Reg);

Instruction_Class XCC_Obj =
{
  "XCC",
  NULL, // decode
  address_stg, // address_stg
  NULL, // access_1
  NULL, // access_2
  NULL, // read_stg
  NULL, // execute
  NULL, // write 
  NULL, // write_plus
  6,
  mask,
  opcode,
};

static void address_stg(struct _PipeLine *pipeP, struct _Registers *Reg)
{
  Opcode opcode;

  opcode = pipeP->decode_nfo.mach_code;
  
  switch ( pipeP->opcode_subType )
    {
    case 0:
    case 2:
    case 4:
      if ( check_condition(opcode.bop[1]&0x7f,Reg) )
	  pipeP->flags = pipeP->flags | PIPE_SKIP_NEXT_INSTR;
      return;
    case 1:
    case 3:
    case 5:
      if ( check_condition(opcode.bop[1]&0x7f,Reg) )
	  pipeP->flags = pipeP->flags | PIPE_SKIP_NEXT_EXECUTE;
      return;
    }
}
