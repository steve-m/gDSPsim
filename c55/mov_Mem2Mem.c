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
#include <cmem.h>

static gchar *mask[]=
{
  "11101111 ssssssss vvvv00cc", // MOV Cmem, Smem
  "11101111 ssssssss vvvv01cc", // MOV Smem, Cmem
  "11100110 ssssssss nnnnnnnn", // MOV K8, Smem
  "11111011 ssssssss nnnnnnnn nnnnnnnn", // MOV K16, Smem
  "11101111 ssssssss vvvv10cc", // MOV Cmem,dbl(Lmem)

  "11101111 ssssssss vvvv11cc", // MOV dbl(Lmem), Cmem
  "10000000 xxxxxxyy yyyy00vv", // MOV dbl(Xmem), dbl(Ymem)
  "10000000 xxxxxxyy yyyy01vv" // MOV Xmem, Ymem
  };

static gchar *opcode[] = 
{
  "'MOV' c,s",
  "'MOV' s,c",
  "'MOV' n,s",
  "'MOV' n,s",
  "'MOV' c,'dbl'(s)",

  "'MOV' 'dbl'(s),c",
  "'MOV' 'dbl'(x),'dbl'(y)",
  "'MOV' x,y",
};

static void address_stg(struct _PipeLine *pipeP, struct _Registers *Reg);
static void read_stg(struct _PipeLine *pipeP, struct _Registers *Reg);
static void execute(struct _PipeLine *pipeP, struct _Registers *Reg);
static void write_stg(struct _PipeLine *pipeP, struct _Registers *Reg);

Instruction_Class MOV_MEM_2_MEM_Obj =
{
  "MOV_MEM_2_MEM",
  NULL, // decode
  address_stg, // address_stg
  NULL, // access_1
  NULL, // access_2
  read_stg, // read_stg
  execute, // execute
  write_stg, // write 
  NULL, // write_plus
  8,
  mask,
  opcode,
};

static void address_stg(struct _PipeLine *pipeP, struct _Registers *Reg)
{

  switch ( pipeP->opcode_subType )
    {
    case 0:
      // MOV Cmem, Smem
    case 4:
      // MOV Cmem,dbl(Lmem)

      // fixme, not suppose to use BB bus. dedecated bus?
      // protected pipeline implementation guards against
      // bus race conditions
      cmem_address_stg_b3(pipeP,Reg);
      return;

    case 2:
      // MOV K8, Smem
    case 3:
      // MOV K16, Smem

      // do nothing
      return;

    case 1:
      // MOV Smem, Cmem
      smem_address_stg_b2(pipeP,Reg);
    case 5:
      // MOV dbl(Lmem), Cmem
      lmem_address_stg_b2(pipeP,Reg);
      return;

    case 6:
      // MOV dbl(Xmem), dbl(Ymem)
      xlmem_address_stg_b2(pipeP,Reg);
      return;

    case 7:
      // MOV Xmem, Ymem 
      xmem_address_stg_b2(pipeP,Reg);
      return;
    }
}
static void read_stg(struct _PipeLine *pipeP, struct _Registers *Reg)
{
  int wait_state;

  switch ( pipeP->opcode_subType )
    {
    case 0:
      // MOV Cmem, Smem
      pipeP->storage1 = read_data_mem(Reg->BAB,&wait_state);
      return;

    case 4:
      // MOV Cmem,dbl(Lmem)

      // Read BAB bus and set 
      pipeP->storage1 = read_data_mem(Reg->BAB,&wait_state);
      pipeP->storage2 = read_data_mem(Reg->BAB^1,&wait_state);
      return;

    case 2:
      // MOV K8, Smem
    case 3:
      // MOV K16, Smem

      // do nothing
      return;


    case 1:
      // MOV Smem, Cmem
    case 7:
      // MOV Xmem, Ymem

      // Read DAB bus and sets DB
      smem_read_stg(pipeP,Reg);
      return;

    case 5:
      // MOV dbl(Lmem), Cmem
    case 6:
      // MOV dbl(Xmem), dbl(Ymem)

      // Read CAB and DAB buses and sets CB and DB
      xymem_read_stg(pipeP,Reg);
      return;
    }
}

static void execute(struct _PipeLine *pipeP, struct _Registers *Reg)
{

  switch ( pipeP->opcode_subType )
    {
    case 0:
      // MOV Cmem, Smem
    case 2:
      // MOV K8, Smem
    case 3:
      // MOV K16, Smem
      smem_set_EAB_b2(pipeP,Reg);
      return;

    case 4:
      // MOV Cmem,dbl(Lmem)
      smem_set_EAB_b2(pipeP,Reg);
      return;

    case 1:
      // MOV Smem, Cmem
      pipeP->storage1 = Reg->DB;
      cmem_set_EAB_b3(pipeP,Reg);
      return;

    case 5:
      // MOV dbl(Lmem), Cmem
      pipeP->storage1 = Reg->DB;
      pipeP->storage2 = Reg->CB;
      cmem_set_EAB_b3(pipeP,Reg);
      return;

    case 6:
      // MOV dbl(Xmem), dbl(Ymem)
      pipeP->storage1 = Reg->DB;
      pipeP->storage2 = Reg->CB;
      ymem_set_EAB_b23(pipeP,Reg);
      return;

    case 7:
      // MOV Xmem, Ymem
      pipeP->storage1 = Reg->DB;
      ymem_set_EAB_b23(pipeP,Reg);
      return;
    }
}


static void write_stg(struct _PipeLine *pipeP, struct _Registers *Reg)
{
  Word kword;
  Opcode opcode;

  opcode = pipeP->decode_nfo.mach_code;

  switch ( pipeP->opcode_subType )
    {
    case 2:
      // MOV K8, Smem
      kword = (Word)((char)opcode.bop[2]);  // sign extend byte
      write_data_mem_long(Reg->EAB,kword);
      return;

    case 3:
      // MOV K16, Smem
      kword = opcode.wop[1];
      write_data_mem_long(Reg->EAB,kword);
      return;

  
    case 0:
      // MOV Cmem, Smem
    case 1:
      // MOV Smem, Cmem
    case 7:
      // MOV Xmem, Ymem
      write_data_mem_long(Reg->EAB,pipeP->storage1);
      return;


    case 4:
      // MOV Cmem,dbl(Lmem)
    case 5:
      // MOV dbl(Lmem), Cmem
    case 6:
      // MOV dbl(Xmem), dbl(Ymem)
      write_data_mem_long(Reg->EAB,pipeP->storage1);
      write_data_mem_long(Reg->EAB^1,pipeP->storage2);
      return;
      
    }
}
