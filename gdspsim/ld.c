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

#include "c54_core.h"
#include "hardware.h"
#include <stdio.h>
#include "xymem.h"
#include "smem.h"
#include "shifter.h"

static GPtrArray *machine_code(gchar *opcode_text);
static void read_stg1(struct _PipeLine *pipeP, struct _Registers *Reg);
static void read_stg2(struct _PipeLine *pipeP, struct _Registers *Reg);
static void execute(struct _PipeLine *pipeP, struct _Registers *Reg);
static int number_words(struct _PipeLine *pipeP);



static gchar *mask[]=
  {
    "0001000s aaaaaaaa",
    "0001010s aaaaaaaa",
    "0100010s aaaaaaaa",
    "01101111 aaaaaaaa 0000110s 010nnnnn",
    "1001010s xxxxuuuu",
    
    "1110100s nnnnnnnn",
    "1111000s 0010uuuu nnnnnnnn nnnnnnnn",
    "1111000s 01100010 nnnnnnnn nnnnnnnn",
    "111101sd 10000010",
    "111101sd 010nnnnn",

    "00110000 aaaaaaaa",
    "01000110 aaaaaaaa",
    "1110101u uuuuuuuu",
    "11101101 000nnnnn",
    "11110100 10100uuu",
    "00110010 aaaaaaaa"

};

static gchar *opcode[] = 
  { 
    "LD a,s",
    "LD a,TS,s",
    "LD a,16,s",
    "LD a,n,s",
    "LD x,u,s",

    "LD #n,s",
    "LD #n,u,s",
    "LD #n,16,s",
    "LD s,ASM,d",
    "LD s,n,d",

    "LD a,T",
    "LD a,DP",
    "LD #u,DP",
    "LD #n,ASM",
    "LD #u,ARP",
    "LD a,ASM"
  };

static gchar *comment[]= 
{
  "$(s)=$(a)",
  "$(s)=$(a)<<TS",
  "$(s)=$(a)<<16",
  "$(s)=$(a)<<$(n)",
  "$(s)=$(x)<<$(u)",
  
  "$(s)=$(n)",
  "$(s)=$(n)<<$(u)",
  "$(s)=$(n)<<16",
  "$(d)=$(s)<<ASM",
  "$(d)=$(s)<<$(n)",

  "T=$(a)",
  "DP=$(a)",
  "DP=$(u)",
  "ASM=$(n)",
  "ARP=$(u)",
  "ASM=$(a)"
};

/* This definition is global because another routine will make have
 * an array that points to all the different instruction classes.
 */
Instruction_Class LD_Obj =
{
  "LD",
  NULL, // prefetch
  NULL, // fetch
  NULL, // decode
  read_stg1, // read_stg1 (access)
  read_stg2, // read_stg2 (read)
  execute, // execute
  number_words, // number_words 
  NULL, // set_cycle_number
  16,
  mask,
  opcode,
  comment,
  machine_code
};

static void read_stg1(struct _PipeLine *pipeP, struct _Registers *Reg)
{
  switch ( pipeP->opcode_subType )
    {
    case 0:
    case 1:
    case 2:
    case 10:
    case 11:
    case 15:
      smem_read_stg1(pipeP,Reg);
      return;
    case 6:
    case 7:
      // always 2 words
      if ( pipeP->word_number == 1 )
	{
	  pipeP->storage1 = Reg->IR;
	}
    case 3:
      // 2 or 3 Smem word
      printf("Fixme %s:%d\n",__FILE__,__LINE__);
    case 4:
      xmem_read_stg1(pipeP,Reg);
    }
}


static void read_stg2(struct _PipeLine *pipeP, struct _Registers *Reg)
{
  switch ( pipeP->opcode_subType )
    {
    case 0:
    case 1:
    case 2:
    case 10:
    case 11:
    case 15:
      smem_read_stg2(pipeP,Reg);
      return;
    case 6:
    case 7:
      // always 2 words
      if ( pipeP->word_number == 1 )
	{
	  pipeP->storage1 = Reg->IR;
	}
    case 3:
      // 2 or 3 Smem word
      printf("Fixme %s:%d\n",__FILE__,__LINE__);
    case 4:
      xmem_read_stg2(pipeP,Reg);
    }
}

static void execute(struct _PipeLine *pipeP, struct _Registers *Reg)
{
  union _GP_Reg_Union reg_union;

  switch ( pipeP->opcode_subType )
    {
    case 0:
      // LD *ARx,A
      reg_union.guint64 = 0;
      reg_union.words.low = Reg->DB;
      if ( pipeP->current_opcode & 0x10000 )
	MMR->B = reg_union.gp_reg;
      else
	MMR->A = reg_union.gp_reg;
      return;
    case 1:
      // LD *ARx,TS,A
      shifter2GPreg(2,(int)MMR->T,SXM(MMR),Reg,pipeP->current_opcode & 0x10000);
      return;
    case 2:
      // LD *ARx,16,A
      shifter2GPreg(2,16,SXM(MMR),Reg,pipeP->current_opcode & 0x10000);
      return;
    case 3:
      // LD *ARx,5,A
      if ( pipeP->word_number == 1 )
	{
	  int shift;
	  
	  shift = pipeP->storage1 && 0x1f;
	  if (shift >= 0x10)
	    shift = shift - 0x20;
	  shifter2GPreg(2,shift,SXM(MMR),Reg,pipeP->storage1 & 0x10000);
	}
      return;
    case 4:
      // LD *ARx,u,A
      shifter2GPreg(2,(int)(pipeP->storage1 & 0xf),
		    SXM(MMR),Reg,pipeP->current_opcode & 0x10000);
      return;
    case 5:
      // LD #n,s
      {
	
	SWord n;

	n = ( pipeP->current_opcode & 0xff );
	if ( n >= 0x80 )
	  {
	    reg_union.gint64 = n - 0x100;
	  }
	else
	  {
	    // reg_union.guint64 = 0;
	    reg_union.guint64 = n;
	  }
	if ( pipeP->current_opcode & 0x10000 )
	  MMR->B = reg_union.gp_reg;
	else
	  MMR->A = reg_union.gp_reg;
	return;
      }
    case 6:
      // LD #n,u,s      
      shiftWord2GPreg((SWord)pipeP->storage1,(int)pipeP->current_opcode & 0xf,
		    1,Reg,pipeP->current_opcode & 0x10000);
      
    case 7:
      // LD #n,16,s
      shiftWord2GPreg((SWord)pipeP->storage1,(int)16,
		    1,Reg,pipeP->current_opcode & 0x10000);
    case 8:
      // LD s,ASM,d
      shifter2EBand_Store(pipeP->current_opcode & 0x20000,1,pipeP->current_opcode & 0x10000,SXM(MMR));
    case 9:
      // LD s,n,d
    case 10:
      // LD a,T
      MMR->T = Reg->DB;
    case 11:
      // LD a,DP
      set_DP(MMR,Reg->DB);
    case 12:
      // LD #u,DP
      set_DP(MMR,pipeP->current_opcode);
    case 13:
      // LD #n,ASM
      set_ASM(MMR,pipeP->current_opcode);
    case 14:
      // LD #u,ARP
      set_ARP(MMR,pipeP->current_opcode);
    case 15:
      // LD a,ASM
      set_ASM(MMR,Reg->DB);

    }
  

// shifter();



}
int number_words(struct _PipeLine *pipeP)
{
  switch ( pipeP->opcode_subType )
    {
    case 4:
    case 5:
    case 8:
    case 9:
    case 12:
    case 13:
    case 14:
      return 1;
    case 6:
    case 7:
      return 2;
    case 0:
    case 1:
    case 2:
    case 10:
    case 11:
    case 15:
      return num_words_for_smem(pipeP);
    case 3:
      return num_words_for_smem_plus1(pipeP);
    }
  printf("Error! Bad subtype %s:%d\n",__FILE__,__LINE__);
  return 1;  
}

/* Generates an array of Words that this opcode text generates or NULL
 * if it doesn't. There should not be any comments in the string
 */
static GPtrArray *machine_code(gchar *opcode_text)
{
  return NULL;
}


