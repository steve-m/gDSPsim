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

static void execute(struct _PipeLine *pipeP, struct _Registers *Reg);

static GPtrArray *machine_code(gchar *opcode_text);

static gchar *mask[]= 
{
  "0000000s aaaaaaaa",
  "0000000s aaaaaaaa",
  "001111sd aaaaaaaa",
  "01101111 aaaaaaaa 000011sd 000nnnnn",
  "1001000s xxxxuuuu",
  "1010000d xxxxyyyy",
  "111100sd 0000uuuu nnnnnnnn nnnnnnnn",
  "111100sd 01100000 nnnnnnnn nnnnnnnn",
  "111101sd 000nnnnn",
  "111101sd 10000000" 
};

static gchar *opcode[] = 
{
"ADD a,s",
"ADD a,TS,s",
"ADD a,16,s,(opt=s)d",
"ADD s,(opt='0')n,s,(opt=d)d",
"ADD x,u,s",
"ADD x,y,d",
"ADD #n,(opt='0')u,s,(opt=s)d",
"ADD #n,16,s,(opt=s)d",
"ADD s,(opt='0')n,(opt=s)d",
"ADD s,_ASM,(opt=s)d"
};

static gchar *comment[]=  
{
"$s=$i$a<<TS+$s",
"$(s)=$(a)<<TS+$(s)",
"$(d)=$(a)<<16+$(s)",
"$(d)=$(a)<<$(n)+$(s)",
"$s)=$(x)<<$(u)+$(s)",
"$(d)=$(x)+$(y)<<16",
"$(d)=$(n)<<$(u)+$(s)",
"$(d)=$(n)<<$(n)#,_16,src[9],dst[8]",
"$(d)=$(d)+$(s)<<$(n)",
"$(d)=$(d)+$(s)<<ASM"
};

/* This definition is global because another routine will make have
 * an array that points to all the different instruction classes.
 */
Instruction_Class ADD_Obj =
{
  "ADD",
  NULL, // prefetch
  NULL, // fetch
  NULL, // decode
  NULL, // read_stg1 (access)
  NULL, // read_stg2 (read)
  execute, // execute
  NULL, // number_words 
  NULL, // set_cycle_number
  10,
  mask,
  opcode,
  comment,
  machine_code
};


/* Executes the instruction. The operands are passed as pointers to
 * Registers. These pointers point to Registers in the Reg variable.
 * Flags may also be affected or cause different execution results,
 * therefore the Register pointer is also passed. Many instructions
 * have slightly different implementations, therefore type is passed.
 */

static void execute(struct _PipeLine *pipeP, struct _Registers *Reg)
{
  int SubType;
  Operand_List *operands;

  SubType = pipeP->opcode_subType;
  operands = &pipeP->operands;

  // operands are pointers to register values.
  // type tells which type instance this is.
  
  switch ( SubType )
    {
    case 1: // ADD Smem,src
      {
	GP_Reg *src;
	SWord *Smem;
	
	Smem = operands->op0.arP;
	src = operands->op1.regP;

	*src = alu_add(*src,0,*Smem,Reg);

	break;
      }
    case 2: // ADD Smem,TS,src
      {
	GP_Reg *src;
	SWord *Smem;
	SWord *TS;

	Smem = operands->op0.arP;
	TS = operands->op1.arP;
	src = operands->op2.regP;

	*src = alu_add(*src,*TS,*Smem,Reg);

	break;
      }
    case 3: // ADD Smem,16,src,dst
      {
	GP_Reg *src,*dst;
	SWord *Smem;
	
	Smem = operands->op0.arP;
	src = operands->op1.regP;
	dst = operands->op2.regP;

	*dst = alu_add(*src,16,*Smem,Reg);
	
	break;
      }
    case 4: // ADD Smem,shift,src,dst
      {
	GP_Reg *src,*dst;
	SWord *Smem;
	SWord shift;
	
	Smem = operands->op0.arP;
	shift = operands->op1.constant;
	src = operands->op2.regP;
	dst = operands->op3.regP;

	*dst = alu_add(*src,shift,*Smem,Reg);
	break;
      }
    case 5: // ADD Xmem,shift,src
      {
	GP_Reg *src;
	Word shift;
	SWord *Xmem;
	
	Xmem = operands->op0.arP;
	shift = operands->op1.u_constant;
	src = operands->op2.regP;

	*src = alu_add(*src,*Xmem,shift,Reg);
	break;
      }
    case 6: // ADD Xmem,Ymem,dst
      {
	SWord *Xmem,*Ymem;
	GP_Reg *dst,*temp_Reg;

	Xmem = operands->op0.arP;
	Ymem = operands->op1.arP;
	dst = operands->op2.regP;
	
	*temp_Reg = word_to_GP_Reg(*Xmem,16);

	*dst = alu_add(*temp_Reg,16,*Ymem,Reg);	
	break;
      }
    case 7: // ADD  #lk [, SHFT], src [, dst ]
      {
 	SWord lk;
	Word shift;
	GP_Reg *src,*dst;

	lk = operands->op0.constant;
	shift = operands->op1.u_constant;
	src = operands->op2.regP;
	dst = operands->op3.regP;

	*dst = alu_add(*src,shift,lk,Reg);	
	break;
      }
    case 8: // ADD  #lk, 16, src [, dst ]
      {
 	SWord lk;
	GP_Reg *src,*dst;

	lk = operands->op0.constant;
	src = operands->op1.regP;
	dst = operands->op2.regP;

	*dst = alu_add(*src,16,lk,Reg);	
      }
    case 9: // ADD  src [, SHIFT], [, dst ]
      {
 	SWord shift,temp_word;
	GP_Reg *src,*dst;

	src = operands->op0.regP;
	shift = operands->op1.u_constant;
	dst = operands->op2.regP;

	printf("Double check ADD case 9 is executed correctly\n");
	temp_word = GP_Reg_Low_to_Word(*src,shift);
	*dst = alu_add(*src,temp_word,0,Reg);	
	break;
      }
    case 10: // ADD  src, ASM, [, dst ]
      {
 	SWord *ASM,temp_word;
	GP_Reg *src,*dst;

	src = operands->op0.regP;
	ASM = operands->op1.arP;
	dst = operands->op2.regP;

	printf("Double check ADD case 10 is executed correctly\n");
	temp_word = GP_Reg_Low_to_Word(*src,*ASM);
	*dst = alu_add(*src,temp_word,0,Reg);	
      }

    }
}

/* Generates an array of Words that this opcode text generates or NULL
 * if it doesn't. There should not be any comments in the string
 */
static GPtrArray *machine_code(gchar *opcode_text)
{
  return NULL;
}


#if 0  
1: (Smem) + (src) ³ src
2: (Smem) << (TS) + (src) ³ src
3: (Smem) << 16 + (src) ³ dst
4: (Smem) [<< SHIFT] + (src) ³ dst
5: (Xmem) << SHFT + (src) ³ src
6:    ADD  Xmem, Ymem, dst
7:    ADD  #lk [, SHFT], src [, dst ]
8:    ADD  #lk, 16, src [, dst ]
9:    ADD  src [, SHIFT], [, dst ]
10: ADD  src, ASM [, dst ]
Syntaxes 1, 2, 3, and 5: Class 3A (see page 3-5
Syntaxes 1, 2, and 3: Class 3B (see page 3-6)
Syntax 4: Class 4A (see page 3-7)
Syntax 4: Class 4B (see page 3-8)
Syntax 6: Class 7 (see page 3-12)
Syntaxes 7 and 8: Class 2 (see page 3-4)
Syntaxes 9 and 10: Class 1 (see page 3-3)
#endif
