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
extern Instruction_Class ADD_Obj;
extern Instruction_Class DADD_Obj;
extern Instruction_Class DLD_Obj;
extern Instruction_Class DST_Obj;
extern Instruction_Class FRET_Obj;
extern Instruction_Class LD_Obj;
extern Instruction_Class MAR_Obj;
extern Instruction_Class MAC_Obj;
extern Instruction_Class MACSU_Obj;
extern Instruction_Class MVMM_Obj;
extern Instruction_Class NEG_Obj;
extern Instruction_Class NOP_Obj;
extern Instruction_Class SFTA_Obj;
extern Instruction_Class STLM_Obj;
extern Instruction_Class STM_Obj;

#define All_Objects_Len  16
static const Instruction_Class *All_Objects[All_Objects_Len]=
{
  &ABDST_Obj,
  &ADD_Obj,
  &DADD_Obj,
  &DLD_Obj,
  &DST_Obj,
  &FRET_Obj,
  &LD_Obj,
  &MAC_Obj,
  &MACSU_Obj,
  &MAR_Obj,
  &MVMM_Obj,
  &NEG_Obj,
  &NOP_Obj,
  &SFTA_Obj,
  &STLM_Obj,
  &STM_Obj,
};


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
  gchar *opcode,*ch,*chP,ans[MAX_OP_LEN],*ansP,*mask,head[23],*ans2,*ans2P,*headP;
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

  opcode = classP->opcode[subtype];
  mask = classP->mask[subtype];
  while ( *opcode )
    {
      if ( *opcode == 'a' )
	{
	  ch = a_decode(mask,'a',mach_code,location);
	  // Copy ch to ansP
	  chP = ch;
	  while ( *chP && (len < MAX_OP_LEN) )
	    {
	      len++;
	      *ansP++ = *chP++;
	    }
	  g_free( ch );

	  // done with this masking marker
	  while ( *opcode == 'a' )
	    opcode++;
	}
      else if ( *opcode =='m' || *opcode == 'w' )
	{
	  info = *opcode;
	  ch = mw_decode(mask,info,mach_code,location);
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
	    opcode++;
	  
	}
      else if ( *opcode =='z' )
	{
	  info = *opcode;
	  ch = z_decode(mask,info,mach_code,location);
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
	    opcode++;
	  
	}
      else if ( *opcode =='r' )
	{
	  info = *opcode;
	  ch = r_decode(mask,info,mach_code,location);
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
	    opcode++;
	  
	}
      else if ( *opcode =='s' || *opcode == 'd' )
	{
	  info = *opcode;
	  ch = sd_decode(mask,info,mach_code,location);
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
	    opcode++;
	  
	}
      else if ( *opcode =='x' || *opcode == 'y' )
	{
	  info = *opcode;
	  ch = xy_decode(mask,info,mach_code,location);
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
	    opcode++;
	  
	}
      else if ( *opcode =='u' )
	{
	  info = *opcode;
	  ch = u_decode(mask,info,mach_code,location);
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
	    opcode++;
	  
	}
      else if ( *opcode =='n' )
	{
	  info = *opcode;
	  ch = n_decode(mask,info,mach_code,location);
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
  *ansP = '\0';

  if ( passed_location == *location )
    {
      g_snprintf(head,MAX_OP_LEN,"0x%.4x 0x%.4x        ",*location,mach_code);
    }
  else if ( passed_location+1 == *location )
    {
      Word mach_code2;
      int wait_state;
      mach_code2 = read_program_mem(*location, &wait_state);
      g_snprintf(head,MAX_OP_LEN,"0x%.4x 0x%.4x 0x%.4x ",*location,mach_code,
		 mach_code2);
    }
  else
    {
      printf("Error! can't decode 3 word opcodes!!\n");
    }


  ans2 = g_new(gchar,len+23);
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

      pre_start=start;
      ch = mach_code_to_text(mach_code,instructO,subtype,&start);

      for(k=pre_start;k<start+1;k++)
	g_array_append_val(word2line,line_no);


      g_ptr_array_add(textA,ch);

      start = start + 1;
      line_no++;
    }
}

