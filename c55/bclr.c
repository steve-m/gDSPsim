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
      
#include "chip_core.h"
#include "smem.h"
static gchar *mask[]=
  {
    "0100 0110 1001 0000", // bclr ACOV1
    "0100 0110 1010 0000", // bclr ACOV0
    "0100 0110 1011 0000", // bclr CARRY
    "0100 0110 1100 0000", // bclr TC2
    "0100 0110 1101 0000", // bclr TC1
    "0100 0110 1110 0000", // bclr ACOV3
    "0100 0110 1111 0000", // bclr ACOV2

    "0100 0110 0101 0010", // bclr C54CM
    "0100 0110 0110 0010", // bclr FRCT
    "0100 0110 0111 0010", // bclr C16
    "0100 0110 1000 0010", // bclr SXMD
    "0100 0110 1001 0010", // bclr SATD
    "0100 0110 1010 0010", // bclr M40
    "0100 0110 1011 0010", // bclr INTM
    "0100 0110 1100 0010", // bclr HM
    "0100 0110 1101 0010", // bclr XF
    "0100 0110 1110 0010", // bclr CPL
    "0100 0110 1111 0010", // bclr BRAF

    "0100 0110 0uuu 0100", // bclr ARuLC
    "0100 0110 1000 0100", // bclr CDPLC
    "0100 0110 1010 0100", // bclr RDM
    "0100 0110 1011 0100", // bclr EALLOW
    "0100 0110 1100 0100", // bclr DBGM
    "0100 0110 1111 0100", // bclr ARMS

    "0100 0110 0000 0110", // bclr SST
    "0100 0110 0001 0110", // bclr SMUL
    "0100 0110 0010 0110", // bclr CLKOFF
    "0100 0110 0101 0110", // bclr SATA
    "0100 0110 0110 0110", // bclr MPNMC
    "0100 0110 0111 0110", // bclr CBERR
    "0100 0110 1100 0110", // bclr HINT
    "0100 0110 1101 0110", // bclr CACLR
    "0100 0110 1110 0110", // bclr CAEN
    "0100 0110 1111 0110", // bclr CAFRZ
  };

static gchar *opcode[] = 
{ 
  "BCLR ACOV1",
  "BCLR ACOV0",
  "BCLR CARRY",
  "BCLR TC2",
  "BCLR TC1",
  "BCLR ACOV3",
  "BCLR ACOV2",

  "BCLR C54CM",
  "BCLR FRCT",
  "BCLR C16",
  "BCLR SXMD",
  "BCLR SATD",
  "BCLR M40",
  "BCLR INTM",
  "BCLR HM",
  "BCLR XF",
  "BCLR CPL",
  "BCLR BRAF",

  "BCLR ARuLC",
  "BCLR CDPLC",
  "BCLR RDM",
  "BCLR EALLOW",
  "BCLR DBGM",
  "BCLR ARMS",

  "BCLR SST",
  "BCLR SMUL",
  "BCLR CLKOFF",
  "BCLR SATA",
  "BCLR MPNMC",
  "BCLR CBERR",
  "BCLR HINT",
  "BCLR CACLR",
  "BCLR CAEN",
  "BCLR CAFRZ",
};

static void execute(struct _PipeLine *pipeP, struct _Registers *Reg);

Instruction_Class BCLR_Obj =
{
  "BCLR",
  NULL, // decode
  NULL, // address_stg
  NULL, // access_1
  NULL, // access_2
  NULL, // read_stg
  execute, // execute
  NULL, // write 
  NULL, // write_plus
  34,
  mask,
  opcode,
};



static void execute(struct _PipeLine *pipeP, struct _Registers *Reg)
{
}


