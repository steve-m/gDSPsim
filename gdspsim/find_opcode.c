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
#include "decode.h"
#include <stdio.h>
#include "string.h"
#include "memory.h"
#include "find_opcode.h"
#include "symbols.h"

extern Instruction_Class ABDST_Obj;
extern Instruction_Class ABS_Obj;
extern Instruction_Class ADD_Obj;
extern Instruction_Class ADDC_Obj;
extern Instruction_Class ADDM_Obj;
extern Instruction_Class ADDS_Obj;
extern Instruction_Class AND_Obj;
extern Instruction_Class ANDM_Obj;
extern Instruction_Class B_Obj;
extern Instruction_Class BACC_Obj;
extern Instruction_Class BANZ_Obj;
extern Instruction_Class BC_Obj;
extern Instruction_Class BIT_Obj;
extern Instruction_Class BITF_Obj;
extern Instruction_Class BITT_Obj;
extern Instruction_Class CALA_Obj;
extern Instruction_Class CALL_Obj;
extern Instruction_Class CMPL_Obj;
extern Instruction_Class CMPM_Obj;
extern Instruction_Class CMPR_Obj;
extern Instruction_Class CMPS_Obj;
extern Instruction_Class DADD_Obj;
extern Instruction_Class DADST_Obj;
extern Instruction_Class DELAY_Obj;
extern Instruction_Class DLD_Obj;
extern Instruction_Class DRSUB_Obj;
extern Instruction_Class DSADT_Obj;
extern Instruction_Class DST_Obj;
extern Instruction_Class DSUB_Obj;
extern Instruction_Class DSUBT_Obj;
extern Instruction_Class EXP_Obj;
extern Instruction_Class FB_Obj;
extern Instruction_Class FBACC_Obj;
extern Instruction_Class FCALA_Obj;
extern Instruction_Class FCALL_Obj;
extern Instruction_Class FIRS_Obj;
extern Instruction_Class FRAME_Obj;
extern Instruction_Class FRET_Obj;
extern Instruction_Class FRETE_Obj;
extern Instruction_Class IDLE_Obj;
extern Instruction_Class INTR_Obj;
extern Instruction_Class LD_Obj;
extern Instruction_Class LD_MAC_Obj;
extern Instruction_Class LD_MAS_Obj;
extern Instruction_Class LDR_Obj;
extern Instruction_Class LDU_Obj;
extern Instruction_Class LDM_Obj;
extern Instruction_Class LMS_Obj;
extern Instruction_Class LTD_Obj;
extern Instruction_Class MAC_Obj;
extern Instruction_Class MACA_Obj;
extern Instruction_Class MACD_Obj;
extern Instruction_Class MACP_Obj;
extern Instruction_Class MACSU_Obj;
extern Instruction_Class MAR_Obj;
extern Instruction_Class MAS_Obj;
extern Instruction_Class MASA_Obj;
extern Instruction_Class MAX_Obj;
extern Instruction_Class MIN_Obj;
extern Instruction_Class MPY_Obj;
extern Instruction_Class MPYA_Obj;
extern Instruction_Class MPYU_Obj;
extern Instruction_Class MVDD_Obj;
extern Instruction_Class MVDK_Obj;
extern Instruction_Class MVDM_Obj;
extern Instruction_Class MVKD_Obj;
extern Instruction_Class MVDP_Obj;
extern Instruction_Class MVMD_Obj;
extern Instruction_Class MVMM_Obj;
extern Instruction_Class MVPD_Obj;
extern Instruction_Class NEG_Obj;
extern Instruction_Class NOP_Obj;
extern Instruction_Class NORM_Obj;
extern Instruction_Class OR_Obj;
extern Instruction_Class ORM_Obj;
extern Instruction_Class POLY_Obj;
extern Instruction_Class POPD_Obj;
extern Instruction_Class POPM_Obj;
extern Instruction_Class PORTR_Obj;
extern Instruction_Class PORTW_Obj;
extern Instruction_Class PSHD_Obj;
extern Instruction_Class PSHM_Obj;
extern Instruction_Class READA_Obj;
extern Instruction_Class RET_Obj; // decodes same as RC with c=0
extern Instruction_Class RC_Obj;
extern Instruction_Class RESET_Obj;
extern Instruction_Class RETE_Obj;
extern Instruction_Class RETF_Obj;
extern Instruction_Class RND_Obj;
extern Instruction_Class ROL_Obj;
extern Instruction_Class ROLTC_Obj;
extern Instruction_Class ROR_Obj;
extern Instruction_Class RPT_Obj;
extern Instruction_Class RPTZ_Obj;
extern Instruction_Class RPTB_Obj;
extern Instruction_Class RSBX_Obj;
extern Instruction_Class SAT_Obj;
extern Instruction_Class SFTA_Obj;
extern Instruction_Class SFTC_Obj;
extern Instruction_Class SFTL_Obj;
extern Instruction_Class SQDST_Obj;
extern Instruction_Class SQUR_Obj;
extern Instruction_Class SQURA_Obj;
extern Instruction_Class SQURS_Obj;
extern Instruction_Class SRCCD_Obj;
extern Instruction_Class SSBX_Obj;
extern Instruction_Class ST_Obj;
extern Instruction_Class ST_ADD_Obj;
extern Instruction_Class ST_LD_Obj;
extern Instruction_Class ST_MAC_Obj;
extern Instruction_Class ST_MAS_Obj;
extern Instruction_Class ST_MPY_Obj;
extern Instruction_Class ST_SUB_Obj;
extern Instruction_Class STH_Obj;
extern Instruction_Class STL_Obj;
extern Instruction_Class STLM_Obj;
extern Instruction_Class STM_Obj;
extern Instruction_Class STRCD_Obj;
extern Instruction_Class SUB_Obj;
extern Instruction_Class SUBB_Obj;
extern Instruction_Class SUBC_Obj;
extern Instruction_Class SUBS_Obj;
extern Instruction_Class TRAP_Obj;
extern Instruction_Class WRITA_Obj;
extern Instruction_Class XC_Obj;
extern Instruction_Class XOR_Obj;
extern Instruction_Class XORM_Obj;

#define All_Objects_Len  127
static const Instruction_Class *All_Objects[All_Objects_Len]=
{
  &ABDST_Obj,
  &ABS_Obj,
  &ADD_Obj,
  &ADDC_Obj,
  &ADDM_Obj,
  &ADDS_Obj,
  &AND_Obj,
  &ANDM_Obj,
  &B_Obj,
  &BACC_Obj,
  &BANZ_Obj,
  &BC_Obj,
  &BIT_Obj,
  &BITF_Obj,
  &BITT_Obj,
  &CALA_Obj,
  &CALL_Obj,
  &CMPL_Obj,
  &CMPM_Obj,
  &CMPR_Obj,
  &CMPS_Obj,
  &DADD_Obj,
  &DADST_Obj,
  &DELAY_Obj,
  &DLD_Obj,
  &DRSUB_Obj,
  &DSADT_Obj,
  &DST_Obj,
  &DSUB_Obj,
  &DSUBT_Obj,
  &EXP_Obj,
  &FB_Obj,
  &FBACC_Obj,
  &FCALA_Obj,
  &FCALL_Obj,
  &FIRS_Obj,
  &FRAME_Obj,
  &FRET_Obj,
  &FRETE_Obj,
  &IDLE_Obj,
  &INTR_Obj,
  &LD_Obj,
  &LD_MAC_Obj,
  &LD_MAS_Obj,
  &LDR_Obj,
  &LDU_Obj,
  &LDM_Obj,
  &LMS_Obj,
  &LTD_Obj,
  &MAC_Obj,
  &MACA_Obj,
  &MACD_Obj,
  &MACP_Obj,
  &MACSU_Obj,
  &MAR_Obj,
  &MAS_Obj,
  &MASA_Obj,
  &MAX_Obj,
  &MIN_Obj,
  &MPY_Obj,
  &MPYA_Obj,
  &MPYU_Obj,
  &MVDD_Obj,
  &MVDK_Obj,
  &MVDM_Obj,
  &MVDP_Obj,
  &MVKD_Obj,
  &MVMD_Obj,
  &MVMM_Obj,
  &MVPD_Obj,
  &NEG_Obj,
  &NOP_Obj,
  &NORM_Obj,
  &OR_Obj,
  &ORM_Obj,
  &POLY_Obj,
  &POPD_Obj,
  &POPM_Obj,
  &PORTR_Obj,
  &PORTW_Obj,
  &PSHD_Obj,
  &PSHM_Obj,
  &RET_Obj,  // decodes same as RC with c=0
  &RC_Obj,
  &READA_Obj,
  &RESET_Obj,
  &RETE_Obj,
  &RETF_Obj,
  &RND_Obj,
  &ROL_Obj,
  &ROLTC_Obj,
  &ROR_Obj,
  &RPT_Obj,
  &RPTB_Obj,
  &RPTZ_Obj,
  &RSBX_Obj,
  &SAT_Obj,
  &SFTA_Obj,
  &SFTC_Obj,
  &SFTL_Obj,
  &SQDST_Obj,
  &SQUR_Obj,
  &SQURA_Obj,
  &SQURS_Obj,
  &SRCCD_Obj,
  &SSBX_Obj,
  &ST_Obj,
  &ST_ADD_Obj,
  &ST_LD_Obj,
  &ST_MAC_Obj,
  &ST_MAS_Obj,
  &ST_MPY_Obj,
  &ST_SUB_Obj,
  &STH_Obj,
  &STL_Obj,
  &STLM_Obj,
  &STM_Obj,
  &STRCD_Obj,
  &SUB_Obj,
  &SUBB_Obj,
  &SUBC_Obj,
  &SUBS_Obj,
  &TRAP_Obj,
  &WRITA_Obj,
  &XC_Obj,
  &XOR_Obj,
  &XORM_Obj,
};

#define NUM_MASK_CODE 17
static Decode_Func mask_function[NUM_MASK_CODE]=
{
  a_decode,
  b_decode,
  c_decode,
  sd_decode,
  h_decode,
  m_decode,
  n_decode,
  p_decode,
  r_decode,
  sd_decode,
  t_decode,
  u_decode,
  vw_decode,
  vw_decode,
  xy_decode,
  xy_decode,
  z_decode,

};
static gchar mask_code[NUM_MASK_CODE]={"abcdhmnprstuvwxyz"};


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
  printf("Error! couldn't decode object 0x%x\n",decode_nfo->mach_code[0]);

  return 1;
}

void mach_code_to_text( struct _decoded_opcode *decode_nfo, 
                        struct _decode_opcode *op )
{
#define MAX_OP_LEN 80
  gchar *opcode,*chP,ans[MAX_OP_LEN],*ansP,*mask;
  gchar ch[MAX_SUB_OP];
  int len,k,found_code;
  gchar info;

  op->address = g_strdup_printf("0x%.4x",decode_nfo->address);

  if ( decode_nfo->class == NULL )
    {
      op->opcode_text = g_strdup("Undefined");
      op->machine_code = g_strdup_printf("0x%.4x",decode_nfo->mach_code[0]);
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
                }
            }
          found_code=0;
	  for (k=0;k<NUM_MASK_CODE;k++)
	    {
	      if ( mask_code[k] == *opcode )
		{
		  info = *opcode;
		  mask_function[k](ch,mask,info,decode_nfo);
	  
		  // Copy ch to ansP
		  chP = ch;
		  while ( *chP && (len < MAX_OP_LEN) )
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
	      if ( len < MAX_OP_LEN )
		{
		  *ansP++ = *opcode++;
		  len++;
		}
	    }
	}
      *ansP = '\0';
      
      if ( decode_nfo->length == 1 )
        {
          op->machine_code = g_strdup_printf("0x%.4x",decode_nfo->mach_code[0]);
        }
      else if ( decode_nfo->length == 2 )
        {
          op->machine_code = g_strdup_printf("0x%.4x 0x%.4x",
                                             decode_nfo->mach_code[0],
                                             decode_nfo->mach_code[1]);
        }
      else
        {
          op->machine_code = g_strdup_printf("0x%.4x 0x%.4x 0x%.4x",
                                             decode_nfo->mach_code[0],
                                             decode_nfo->mach_code[1],
                                             decode_nfo->mach_code[2]);
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
#define MAX_OP_LEN 80
  gchar *opcode,*ch,*chP,ans[MAX_OP_LEN],*ansP,*mask,*a_ch;
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
		  if ( len < MAX_OP_LEN )
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
	  while ( *chP && (len < MAX_OP_LEN) )
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
		  while ( *chP && (len < MAX_OP_LEN) )
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
	      if ( len < MAX_OP_LEN )
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

  while (start < end)
    {
      op = g_new(struct _decode_opcode,1);
      mach_code = read_program_mem(start,&wait_state);

      if ( mach_code == 0x7ef8 )
        mach_code = 0x7ef8;

      decode_nfo.mach_code[0] = mach_code;
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
	  op->machine_code = g_strdup_printf("0x%.4x",mach_code);
	  g_array_append_val(word2line,line_no);
	}
      g_ptr_array_add(textA,op);
	  
      start = start + decode_nfo.length;
      line_no++;
    }
}

