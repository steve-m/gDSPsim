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
#include <memory.h>
#include <shifter.h>

static gchar *mask[]=
{
  "10000111 xxxxxxyy yyyyrrRR 110vvvvv", // MOV Xmem << #16, ACy :: MOV HI(ACx << T2), Ymem
};

static gchar *opcode[] = 
{
  "'MOV' x '<< #16,' R ':: MOV HI'(r '<< T2')",
};

static void address_stg(struct _PipeLine *pipeP, struct _Registers *Reg);
static void read_stg(struct _PipeLine *pipeP, struct _Registers *Reg);
static void execute(struct _PipeLine *pipeP, struct _Registers *Reg);
static void write_stg(struct _PipeLine *pipeP, struct _Registers *Reg);

Instruction_Class IMPLIED_PARALLEL_INSTR_Obj =
{
  "IMPLIED_PARALLEL_INSTR",
  NULL, // decode
  address_stg, // address_stg
  NULL, // access_1
  NULL, // access_2
  read_stg, // read_stg
  execute, // execute
  write_stg, // write 
  NULL, // write_plus
  1,
  mask,
  opcode,
};

static void address_stg(struct _PipeLine *pipeP, struct _Registers *Reg)
{
  switch ( pipeP->opcode_subType )
    {
    case 0:
      // should be case 5
      // MOV Xmem << #16, ACy :: MOV HI(ACx << T2), Ymem
      xmem_address_stg_b2(pipeP,Reg);
      return;
    }
}

static void read_stg(struct _PipeLine *pipeP, struct _Registers *Reg)
{
  switch ( pipeP->opcode_subType )
    {
    case 0:
      // should be case 5
      // MOV Xmem << #16, ACy :: MOV HI(ACx << T2), Ymem
      // Read DAB bus and sets DB
      smem_read_stg(pipeP,Reg);
      return;
    }
}

static void execute(struct _PipeLine *pipeP, struct _Registers *Reg)
{
  int r,R;
  Opcode opcode;

  opcode = pipeP->decode_nfo.mach_code;

  switch ( pipeP->opcode_subType )
    {
    case 0:
      // should be case 5
      // MOV Xmem << #16, ACy :: MOV HI(ACx << T2), Ymem

      // MOV HI(ACx << T2), Ymem
      r=(opcode.bop[2]&0xc)>>2;
      shifter(r,2,0,0,r,Reg);
      pipeP->storage1 = get_k16_regHI(r,0);
      ymem_set_EAB_b23(pipeP,Reg);

      // MOV Xmem << #16, ACy
      R=opcode.bop[2]&0x3;
      shifter(16,8,16,0,R,Reg);
      return;
    }
}


static void write_stg(struct _PipeLine *pipeP, struct _Registers *Reg)
{
  
  switch ( pipeP->opcode_subType )
    {
    case 0:
      // should be case 5
      // MOV Xmem << #16, ACy :: MOV HI(ACx << T2), Ymem
      write_data_mem_long(Reg->EAB,pipeP->storage1);
      return;
    }
}
