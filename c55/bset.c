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
#include <smem.h>
#include <memory.h>
static gchar *mask[]=
  {
    "0100011p uuuu0001", // BSET k4, ST0_55
    "0100011p uuuu0011", // BSET k4, ST1_55
    "0100011p uuuu0101", // BSET k4, ST2_55
    "0100011p uuuu0111", // BSET k4, ST3_55
    "11100011 ssssssss rrrr1100", // BSET src, Smem

    "11101100 ssssssss rrrr000v", // BSET Baddr, src

#if 0
    "0100 0110 1001 0001", // bset ACOV1
    "0100 0110 1010 0001", // bset ACOV0
    "0100 0110 1011 0001", // bset CARRY
    "0100 0110 1100 0001", // bset TC2
    "0100 0110 1101 0001", // bset TC1
    "0100 0110 1110 0001", // bset ACOV3
    "0100 0110 1111 0001", // bset ACOV2

    "0100 0110 0101 0011", // bset C54CM
    "0100 0110 0110 0011", // bset FRCT
    "0100 0110 0111 0011", // bset C16
    "0100 0110 1000 0011", // bset SXMD
    "0100 0110 1001 0011", // bset SATD
    "0100 0110 1010 0011", // bset M40
    "0100 0110 1011 0011", // bset INTM
    "0100 0110 1100 0011", // bset HM
    "0100 0110 1101 0011", // bset XF
    "0100 0110 1110 0011", // bset CPL
    "0100 0110 1111 0011", // bset BRAF

    "0100 0110 0uuu 0101", // bset ARuLC
    "0100 0110 1000 0101", // bset CDPLC
    "0100 0110 1010 0101", // bset RDM
    "0100 0110 1011 0101", // bset EALLOW
    "0100 0110 1100 0101", // bset DBGM
    "0100 0110 1111 0101", // bset ARMS

    "0100 0110 0000 0111", // bset SST
    "0100 0110 0001 0111", // bset SMUL
    "0100 0110 0010 0111", // bset CLKOFF
    "0100 0110 0101 0111", // bset SATA
    "0100 0110 0110 0111", // bset MPNMC
    "0100 0110 0111 0111", // bset CBERR
    "0100 0110 1100 0111", // bset HINT
    "0100 0110 1101 0111", // bset CACLR
    "0100 0110 1110 0111", // bset CAEN
    "0100 0110 1111 0111", // bset CAFRZ
#endif
  };

static gchar *opcode[] = 
{ 
  "'BSET' u,'ST0_55'",
  "'BSET' u,'ST1_55'",
  "'BSET' u,'ST2_55'",
  "'BSET' u,'ST3_55'",
  "'BSET' r,s",
  "'BSET' s,r",

#if 0
  "BSET ACOV1",
  "BSET ACOV0",
  "BSET CARRY",
  "BSET TC2",
  "BSET TC1",
  "BSET ACOV3",
  "BSET ACOV2",

  "BSET C54CM",
  "BSET FRCT",
  "BSET C16",
  "BSET SXMD",
  "BSET SATD",
  "BSET M40",
  "BSET INTM",
  "BSET HM",
  "BSET XF",
  "BSET CPL",
  "BSET BRAF",

  "BSET ARuLC",
  "BSET CDPLC",
  "BSET RDM",
  "BSET EALLOW",
  "BSET DBGM",
  "BSET ARMS",

  "BSET SST",
  "BSET SMUL",
  "BSET CLKOFF",
  "BSET SATA",
  "BSET MPNMC",
  "BSET CBERR",
  "BSET HINT",
  "BSET CACLR",
  "BSET CAEN",
  "BSET CAFRZ",
#endif
};

static void address_stg(struct _PipeLine *pipeP, struct _Registers *Reg);
static void read_stg(struct _PipeLine *pipeP, struct _Registers *Reg);
static void execute(struct _PipeLine *pipeP, struct _Registers *Reg);
static void write_stg(struct _PipeLine *pipeP, struct _Registers *Reg);

Instruction_Class BSET_Obj =
{
  "BSET",
  NULL, // decode
  address_stg, // address_stg
  NULL, // access_1
  NULL, // access_2
  read_stg, // read_stg
  execute, // execute
  write_stg, // write 
  NULL, // write_plus
  5,
  mask,
  opcode,
};

static void address_stg(struct _PipeLine *pipeP, struct _Registers *Reg)
{
  if ( pipeP->opcode_subType==4 || pipeP->opcode_subType==5 )
    { 
      smem_address_stg_b2(pipeP,Reg);
      pipeP->storage2 = Reg->DAB;
    } 
}

static void read_stg(struct _PipeLine *pipeP, struct _Registers *Reg)
{
  if ( pipeP->opcode_subType==4 || pipeP->opcode_subType==5 )
    { 
      smem_read_stg(pipeP,Reg);
    } 
}

static void execute(struct _PipeLine *pipeP, struct _Registers *Reg)
{
  Opcode opcode;
  int bit,r;
  union _GP_Reg_Union reg_union;

  opcode = pipeP->decode_nfo.mach_code;
  
  switch ( pipeP->opcode_subType )
    {
    case 0:
      bit = (opcode.bop[1]>>4)&0xf;
      MMR->ST0_55 = MMR->ST0_55 | 1<<bit;
      break;
    case 1:
      bit = (opcode.bop[1]>>4)&0xf;
      MMR->ST0_55 = MMR->ST1_55 | 1<<bit;
      break;
    case 2:
      bit = (opcode.bop[1]>>4)&0xf;
      MMR->ST0_55 = MMR->ST2_55 | 1<<bit;
      break;
    case 3:
      bit = (opcode.bop[1]>>4)&0xf;
      if ( bit > 12 )
	{
	  // flush pipeline
	  FIXME();
	}
      MMR->ST0_55 = MMR->ST3_55 | 1<<bit;
      break;
    case 4:
      // BSET src, Smem
      r = (opcode.bop[2]>>4)&0xf;
      reg_union = get_register(r,0);
      bit = reg_union.words.low & 0xf;
      pipeP->storage1 = Reg->DB | 1<<bit;
      break;
    case 5:
      // BSET Baddr, src
      r = (opcode.bop[2]>>4)&0xf;
      bit = Reg->DB & 0xf;
      reg_union = get_register(r,0);
      reg_union.guint64 = reg_union.guint64  | 1<<bit;
      set_register(reg_union,r);
      break;
    }
}

static void write_stg(struct _PipeLine *pipeP, struct _Registers *Reg)
{
  if ( pipeP->opcode_subType==4 )
    { 
      write_data_mem_long(pipeP->storage2,pipeP->storage1);
    } 
}
