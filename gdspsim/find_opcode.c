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
extern Instruction_Class DADD_Obj;
extern Instruction_Class DLD_Obj;
extern Instruction_Class DST_Obj;
extern Instruction_Class FCALA_Obj;
extern Instruction_Class FCALL_Obj;
extern Instruction_Class FRAME_Obj;
extern Instruction_Class FRET_Obj;
extern Instruction_Class LD_Obj;
extern Instruction_Class LDM_Obj;
extern Instruction_Class MAC_Obj;
extern Instruction_Class MACSU_Obj;
extern Instruction_Class MAR_Obj;
extern Instruction_Class MVDD_Obj;
extern Instruction_Class MVDK_Obj;
extern Instruction_Class MVDP_Obj;
extern Instruction_Class MVMM_Obj;
extern Instruction_Class NEG_Obj;
extern Instruction_Class NOP_Obj;
extern Instruction_Class OR_Obj;
extern Instruction_Class READA_Obj;
extern Instruction_Class RPT_Obj;
extern Instruction_Class RPTB_Obj;
extern Instruction_Class RSBX_Obj;
extern Instruction_Class SFTA_Obj;
extern Instruction_Class SSBX_Obj;
extern Instruction_Class ST_LD_Obj;
extern Instruction_Class STL_Obj;
extern Instruction_Class STLM_Obj;
extern Instruction_Class STM_Obj;

#define All_Objects_Len  44
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
  &DADD_Obj,
  &DLD_Obj,
  &DST_Obj,
  &FCALA_Obj,
  &FCALL_Obj,
  &FRAME_Obj,
  &FRET_Obj,
  &LD_Obj,
  &LDM_Obj,
  &MAC_Obj,
  &MACSU_Obj,
  &MAR_Obj,
  &MVDD_Obj,
  &MVDK_Obj,
  &MVDP_Obj,
  &MVMM_Obj,
  &NEG_Obj,
  &NOP_Obj,
  &OR_Obj,
  &READA_Obj,
  &RPT_Obj,
  &RPTB_Obj,
  &RSBX_Obj,
  &SFTA_Obj,
  &SSBX_Obj,
  &ST_LD_Obj,
  &STL_Obj,
  &STLM_Obj,
  &STM_Obj,
};

typedef gchar *(*Decode_Func)(gchar *mask, gchar info, Word start_code, WordA *location);

#define NUM_MASK_CODE 15
static Decode_Func mask_function[NUM_MASK_CODE]=
{
  b_decode,
  c_decode,
  sd_decode,
  h_decode,
  m_decode,
  n_decode,
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
static gchar mask_code[NUM_MASK_CODE]={"bcdhmnrstuvwxyz"};


// Returns pointer to object type given opcode and sets subtype
const Instruction_Class *find_object(Word mach_code, int *subtype)
{
  const Instruction_Class *object;
  int k,n;

  for (k=0;k<All_Objects_Len;k++)
    {
      for (n=0;n<All_Objects[k]->size;n++)
	{
	  if ( check_mask(All_Objects[k]->mask[n],mach_code) )
	    {
      	      object = All_Objects[k];
	      *subtype=n;
	      return object;
	    }
	}
    }
  printf("Error! couldn't decode object 0x%x\n",mach_code);

  return NULL;
}

gchar *mach_code_to_text(Word mach_code, const Instruction_Class *classP, 
			 int subtype, WordA *location)
{
#define MAX_OP_LEN 80
  gchar *opcode,*ch,*chP,ans[MAX_OP_LEN],*ansP,*mask,head[30],*ans2,*ans2P,*headP,*a_ch;
  int len;
  gchar info;
  WordA passed_location;

  passed_location = *location;
  ansP = ans;
  len = 1; // count null termination.

  if (classP==NULL)
    {
      g_snprintf(ans,MAX_OP_LEN,"0x%.4x 0x%.4x        Undefined",*location,mach_code);
      return g_strdup(ans);
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

  if ( passed_location == *location )
    {
      g_snprintf(head,MAX_OP_LEN,"0x%.4x 0x%.4x        ",passed_location,mach_code);
    }
  else if ( passed_location+1 == *location )
    {
      Word mach_code2;
      int wait_state;
      mach_code2 = read_program_mem(*location, &wait_state);
      g_snprintf(head,MAX_OP_LEN,"0x%.4x 0x%.4x 0x%.4x ",passed_location,mach_code,
		 mach_code2);
    }
  else
    {
      Word mach_code2,mach_code3;
      int wait_state;

      mach_code2 = read_program_mem(passed_location+1,&wait_state);
      mach_code3 = read_program_mem(passed_location+2,&wait_state);

      g_snprintf(head,MAX_OP_LEN,"0x%.4x 0x%.4x 0x%.4x 0x%.4x ",passed_location,mach_code,mach_code2,mach_code3);
    }


  ans2 = g_new(gchar,len+30);
  ans2P = ans2;
  headP = head;
  while ( *headP )
    {
      *ans2P++=*headP++;
    }
  ansP = ans;
  while ( *ansP )
    {
      *ans2P++=*ansP++;
    }
  *ans2P = '\0';

  return ans2;

}

/* Returns an array of strings of decoded opcodes */
void decoded_opcodes(GPtrArray *textA,WordA start,WordA end, GArray *word2line)
{
  gchar *ch;
  const Instruction_Class *instructO;
  int subtype;
  int wait_state;
  Word mach_code;
  int line_no=0;
  WordA pre_start,k;
  
  while (start < end)
    {
      mach_code = read_program_mem(start,&wait_state);

      instructO = find_object(mach_code,&subtype);

      if ( instructO )
	{
	  pre_start=start;

	  ch = mach_code_to_text(mach_code,instructO,subtype,&start);
	  
	  // Used to mach word location to line number
	  for(k=pre_start;k<start+1;k++)
	    g_array_append_val(word2line,line_no);
	  
	}
      else
	{
	  ch=g_new(gchar,25);
	  g_snprintf(ch,25,"0x%.4x 0x%.4x Undefined",start,mach_code);
	}
      g_ptr_array_add(textA,ch);
	  
      start = start + 1;
      line_no++;
    }
}

