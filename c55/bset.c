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
  };

static gchar *opcode[] = 
{ 
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
};

static void execute(struct _PipeLine *pipeP, struct _Registers *Reg);

Instruction_Class BSET_Obj =
{
  "BSET",
  NULL, // decode
  NULL, // address_stg
  NULL, // access_1
  NULL, // access_2
  NULL, // read_stg
  execute, // execute
  NULL, // write 
  NULL, // write_plus
  5,
  mask,
  opcode,
};



static void execute(struct _PipeLine *pipeP, struct _Registers *Reg)
{
}


