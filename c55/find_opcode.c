/*
 * gDSPsim - GNU Digital Signal Processor Simulator
 *
 * Copyright (C) 2001-2002, Kerry Keal, kerry@industrialmusic.com
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
#include <decode.h>
#include <stdio.h>
#include <string.h>
#include <memory.h>
#include <find_opcode.h>
#include <symbols.h>

extern Instruction_Class AADD_Obj;
extern Instruction_Class ABS_Obj;
extern Instruction_Class ADD_Obj;
extern Instruction_Class ADDSUB_Obj;
extern Instruction_Class AND_Obj;
extern Instruction_Class BAND_Obj;
extern Instruction_Class BCLR_Obj;
extern Instruction_Class BCNT_Obj;
extern Instruction_Class BFXPA_Obj;
extern Instruction_Class BFXTR_Obj;
extern Instruction_Class BNOT_Obj;
extern Instruction_Class BSET_Obj;
extern Instruction_Class BTST_Obj;
extern Instruction_Class BTSTCLR_Obj;
extern Instruction_Class BTSTNOT_Obj;
extern Instruction_Class BTSTSET_Obj;
extern Instruction_Class BTSTP_Obj;
extern Instruction_Class DUAL_MULTIPLY_Obj;
extern Instruction_Class IMPLIED_PARALLEL_INSTR_Obj;
extern Instruction_Class MAC_Obj;
extern Instruction_Class MAS_Obj;
extern Instruction_Class MOV_MEM_2_MEM_Obj;
extern Instruction_Class MOV_REG_LOAD_Obj;
extern Instruction_Class MOV_REG_MOVE_Obj;
extern Instruction_Class MOV_REG_STORE_Obj;
extern Instruction_Class MOV_SPEC_REG_LOAD_Obj;
extern Instruction_Class MOV_SPEC_REG_MOVE_Obj;
extern Instruction_Class MOV_SPEC_REG_SAVE_Obj;
extern Instruction_Class MPY_Obj;
extern Instruction_Class MPYK_Obj;
extern Instruction_Class NEG_Obj;
extern Instruction_Class NOT_Obj;
extern Instruction_Class OR_Obj;
extern Instruction_Class ROL_Obj;
extern Instruction_Class ROR_Obj;
extern Instruction_Class PSH_Obj;
extern Instruction_Class SFTL_Obj;
extern Instruction_Class SFTS_Obj;
extern Instruction_Class SFTSC_Obj;
extern Instruction_Class SQR_Obj;
extern Instruction_Class SQRM_Obj;
extern Instruction_Class SUB_Obj;
extern Instruction_Class SUBADD_Obj;
extern Instruction_Class XOR_Obj;

#define All_Objects_Len  44
static const Instruction_Class *All_Objects[All_Objects_Len]=
{
  &AADD_Obj,
  &ABS_Obj,
  &ADD_Obj,
  &ADDSUB_Obj,
  &AND_Obj,
  &BAND_Obj,
  &BCLR_Obj,
  &BCNT_Obj,
  &BFXPA_Obj,
  &BFXTR_Obj,
  &BNOT_Obj,
  &BSET_Obj,
  &BTST_Obj,
  &BTSTCLR_Obj,
  &BTSTNOT_Obj,
  &BTSTSET_Obj,
  &BTSTP_Obj,
  &DUAL_MULTIPLY_Obj,
  &IMPLIED_PARALLEL_INSTR_Obj,
  &MAC_Obj,
  &MAS_Obj,
  &MOV_MEM_2_MEM_Obj,
  &MOV_REG_LOAD_Obj,
  &MOV_REG_MOVE_Obj,
  &MOV_REG_STORE_Obj,
  &MOV_SPEC_REG_LOAD_Obj,
  &MOV_SPEC_REG_MOVE_Obj,
  &MOV_SPEC_REG_SAVE_Obj,
  &MPY_Obj,
  &MPYK_Obj,
  &NEG_Obj,
  &NOT_Obj,
  &OR_Obj,
  &ROL_Obj,
  &ROR_Obj,
  &PSH_Obj,
  &SFTL_Obj,
  &SFTS_Obj,
  &SFTSC_Obj,
  &SQR_Obj,
  &SQRM_Obj,
  &SUB_Obj,
  &SUBADD_Obj,
  &XOR_Obj,
};

#define NUM_MASK_CODE 26
static Decode_Func mask_function[NUM_MASK_CODE]=
{
  t3_decode,
  m4_decode,
  A_decode,
  C_decode,
  F_decode,
  G_decode,
  U_decode,
  V_decode,
  rR_decode,
  T_decode,
  U_decode,
  V_decode,
  zZ_decode,
  c_decode,
  f_decode,
  h_decode,
  m_decode,
  n_decode,
  p_decode,
  rR_decode,
  t_decode,
  u_decode,
  v_decode,
  xy_decode,
  xy_decode,
  zZ_decode,
};
static gchar mask_code[NUM_MASK_CODE]={"34ACFGHIRTUVZcfhmnprtuvxyz"};


// Sets class,sub_type,length,mach_code1,mach_code2 of decode_nfo. 
// Returns 0 for OK, 1 for Error. Assumes mach_code0 and address have
// been set.
int find_object( struct _decoded_opcode *decode_nfo)
{
  int k,n;

  for (k=0;k<All_Objects_Len;k++)
    {
      for (n=0;n<All_Objects[k]->size;n++)
	{
          if ( check_mask(All_Objects[k]->mask[n],decode_nfo) )
            {
              decode_nfo->sub_type=n;
              decode_nfo->class = All_Objects[k];
              return 0;
            }
	}
    }
  printf("Error! couldn't decode object 0x%x\n",decode_nfo->mach_code.bop[0]);

  return 1;
}

void mach_code_to_text( struct _decoded_opcode *decode_nfo, 
                        struct _decode_opcode *op )
{
  gchar *opcode,*chP,ans[MAX_OP_STR_LEN],*ansP,*mask;
  gchar ch[MAX_SUB_OP];
  int len,k,found_code;
  gchar info;

  op->address = g_strdup_printf("0x%.4x",decode_nfo->address);

  if ( decode_nfo->class == NULL )
    {
      op->opcode_text = g_strdup("Undefined");
      op->machine_code = g_strdup_printf("0x%.2x",decode_nfo->mach_code.bop[0]);
      return;
    }
  else
    {
      ansP = ans;
      len = 1; // length of ans with \0

      opcode = decode_nfo->class->opcode[decode_nfo->sub_type];
      mask = decode_nfo->class->mask[decode_nfo->sub_type];
      while ( *opcode )
        {
	  if ( *opcode == '\'' )
	    {
	      opcode++;
	      while ( *opcode && *opcode!='\'' )
		{
		  // Quoted text
		  if ( len < MAX_OP_STR_LEN )
		    {
		      *ansP++ = *opcode++;
		      len++;
		    }
		}
	    }
	  else
	    {
	      // Look through all the markers
	      found_code=0;
	      for (k=0;k<NUM_MASK_CODE;k++)
		{
		  if ( mask_code[k] == *opcode )
		    {
		      info = *opcode;
		      mask_function[k](ch,mask,info,decode_nfo);
	  
		      // Copy ch to ansP
		      chP = ch;
		      while ( *chP && (len < MAX_OP_STR_LEN) )
			{
			  len++;
			  *ansP++ = *chP++;
			}
		  
		      // done with this masking marker
		      while ( *opcode == info )
			{
			  opcode++;
			}
		      found_code=1;
		      break; // Break out of for loop
		    }
		}
	  
	      if ( !found_code )
		{
		  // OK, it's not a marker and should just be copied
		  if ( len < MAX_OP_STR_LEN )
		    {
		      *ansP++ = *opcode++;
		      len++;
		    }
		}
	    }
	}
      *ansP = '\0';

      {
	gchar *mach_str;
	mach_str = g_new(gchar,decode_nfo->length*2+3);
	op->machine_code = mach_str;
	sprintf(mach_str,"0x%.2x",decode_nfo->mach_code.bop[0]);
	
	mach_str=mach_str+4;
	for (k=1;k<decode_nfo->length;k++)
	  {
	    sprintf(mach_str,"%.2x",decode_nfo->mach_code.bop[k]);
	    mach_str=mach_str+2;
	  }
      }
      op->opcode_text = g_strdup(ans);
    }
}

#if 0
// This function can be cleaned up using length info.
struct _decode_opcode *mach_code_to_text(Word mach_code, 
                                         const Instruction_Class *classP, 
                                         int subtype, WordA *location, 
                                         int length)
{
  gchar *opcode,*ch,*chP,ans[MAX_OP_STR_LEN],*ansP,*mask,*a_ch;
  int len;
  gchar info;
  WordA passed_location;
  struct _decode_opcode *op;

  // op freed in insert_text
  op = g_new(struct _decode_opcode,1);

  op->address = g_strdup_printf("0x%.4x",*location);

  passed_location = *location;
  ansP = ans;
  len = 1; // count null termination.

  if (classP==NULL)
    {
      op->opcode_text = g_strdup("Undefined");
      op->machine_code = g_strdup_printf("0x%.4x",mach_code);
      return op;
    }

  // Some opcodes may be 2 or 3 words depending on how the 'a'
  // tag decodes. So figure out that first.
  opcode = classP->opcode[subtype];
  mask = classP->mask[subtype];
  a_ch=NULL;
  while ( *opcode )
    {
      if ( *opcode == 'a' )
	{
	  a_ch = a_decode(mask,'a',mach_code,location);
          *location = passed_location;

	  while ( *opcode == 'a' )
	    {
	      opcode++;
	    }
	}
      else
	opcode++;
    }

  opcode = classP->opcode[subtype];
  while ( *opcode )
    {
      if ( *opcode == '(' )
	{
	  // Check to see if it's (opt*), and if so ignored it.
	  size_t sub_len;
	  sub_len = strlen(opcode);
	  if ( sub_len >= 4 )
	    {
	      if ( strncmp(opcode,"(opt",4) == 0 )
		{
		  opcode = opcode + 4;
		  while ( *opcode && *opcode != ')' )
		    opcode++;
		  if ( *opcode )
		    opcode++;
		}
	      else
		{
		  // OK, it's not a marker and should just be copied
		  if ( len < MAX_OP_STR_LEN )
		    {
		      *ansP++ = *opcode++;
		      len++;
		    }
		}
	    }
	}
      else if ( *opcode == 'a' )
	{
	  // Already processed some of this
	  // Copy ch to ansP
	  chP = a_ch;
	  while ( *chP && (len < MAX_OP_STR_LEN) )
	    {
	      len++;
	      *ansP++ = *chP++;
	    }
	  
	  // done with this masking marker
	  while ( *opcode == 'a' )
	    {
	      opcode++;
	    }
	}
      else
	{
	  int k;
	  int found_code=0;

	  for (k=0;k<NUM_MASK_CODE;k++)
	    {
	      if ( mask_code[k] == *opcode )
		{
		  info = *opcode;
		  ch = mask_function[k](mask,info,mach_code,location);
	          *location = passed_location;
	  
		  // Copy ch to ansP
		  chP = ch;
		  while ( *chP && (len < MAX_OP_STR_LEN) )
		    {
		      len++;
		      *ansP++ = *chP++;
		    }
		  g_free( ch );
		  
		  // done with this masking marker
		  while ( *opcode == info )
		    {
		      opcode++;
		    }
		  found_code=1;
		  break; // Break out of for loop
		}
	    }
	  
	  if ( !found_code )
	    {
	      // OK, it's not a marker and should just be copied
	      if ( len < MAX_OP_STR_LEN )
		{
		  *ansP++ = *opcode++;
		  len++;
		}
	    }
	}
    }
  *ansP = '\0';

  if ( a_ch )
    g_free(a_ch);

  if ( length == 1 )
    {
      op->address = g_strdup_printf("0x%.4x",passed_location);
      op->machine_code = g_strdup_printf("0x%.4x",mach_code);
    }
  else if ( length == 2 )
    {
      Word mach_code2;
      int wait_state;
      mach_code2 = read_program_mem(passed_location+1,&wait_state);
      op->address = g_strdup_printf("0x%.4x",passed_location);
      op->machine_code = g_strdup_printf("0x%.4x 0x%.4x",mach_code,mach_code2);
    }
  else
    {
      Word mach_code2,mach_code3;
      int wait_state;
      mach_code2 = read_program_mem(passed_location+1,&wait_state);
      mach_code3 = read_program_mem(passed_location+2,&wait_state);

      op->address = g_strdup_printf("0x%.4x",passed_location);
      op->machine_code = g_strdup_printf("0x%.4x 0x%.4x 0x%.4x",mach_code,mach_code2,mach_code3);
    }

  op->opcode_text = g_strdup(ans);

  *location = *location + (length-1);
  return op;

}
#endif

/* Returns an array of strings of decoded opcodes */
void decoded_opcodes(GPtrArray *textA,WordA start,WordA end, GArray *word2line)
{
  //const Instruction_Class *instructO;
  //int subtype;
  int wait_state;
  Word mach_code;
  int line_no=0;
  WordA k;
  struct _decode_opcode *op;
  //int length;
  struct _decoded_opcode decode_nfo;
  int num_to_read;

  decode_nfo.length = MAX_OP_LEN;
  num_to_read = MAX_OP_LEN;

  while (start < end)
    {
      op = g_new(struct _decode_opcode,1);

      for (k=MAX_OP_LEN-num_to_read;k<MAX_OP_LEN;k++)
	{
	  decode_nfo.mach_code.bop[k-MAX_OP_LEN+num_to_read] = decode_nfo.mach_code.bop[k];
	}
      for (k=0;k<num_to_read;k++)
	{
	  mach_code = read_program_mem(start+k+MAX_OP_LEN-num_to_read,&wait_state);
	  decode_nfo.mach_code.bop[k+MAX_OP_LEN-num_to_read] = mach_code;
	}
      decode_nfo.address = start;

      if ( find_object(&decode_nfo)==0 )
	{
	  // op freed in insert_text
	  mach_code_to_text(&decode_nfo,op);
	  
	  // Used to match word location to line number
	  for(k=start;k<start+decode_nfo.length;k++)
	    {
	      gchar *sym_name;

	      sym_name=get_symbol(k);
	      if (sym_name)
		{
		  struct _decode_opcode *op_sym;

		  // op_sym freed in insert_text()
		  op_sym = g_new(struct _decode_opcode,1);
		  op_sym->opcode_text = sym_name;
		  op_sym->machine_code = NULL;
		  op_sym->address = g_strdup_printf("0x%.4x",k);
		  line_no++;
		  g_ptr_array_add(textA,op_sym);
		}
	      g_array_append_val(word2line,line_no);
	    }
	}
      else
	{
	  // op freed in insert_text
	  op = g_new(struct _decode_opcode,1);
	  op->address = g_strdup_printf("0x%.4x",start);
	  op->opcode_text = g_strdup("Undefined");
	  op->machine_code = g_strdup_printf("0x%.2x",decode_nfo.mach_code.bop[0]);
	  g_array_append_val(word2line,line_no);
	  decode_nfo.length = 1;
	}
      g_ptr_array_add(textA,op);
	  
      start = start + decode_nfo.length;
      num_to_read = MAX_OP_LEN - decode_nfo.length;
      line_no++;
    }
}

