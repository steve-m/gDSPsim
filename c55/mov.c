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
     "1100 1ppp 000u uuu0", // MOV p,u
     "1010 rrrr 000u uuu0", // MOV u,ACr
     "1010 rrrr 111m mmm0", // MOV -m,ACr

      "1010 rrrr pppp pppp", // MOV p,r

     // 1010 rrrr ____ _uu0  // MOV *SP(#u),r
     // rrrr = 0100 for T0
     // rrrr = 1011 for AR3
     // rrrr = 
     "1110 1101 pppp pppp 00rr 1000", // MOV dbl(p),ACr
     "1110 1101 pppp pppp 00rr 1001", // MOV40 p,r
     "1110 1101 pppp pppp 00rr 1010", // mov dbl(p),pair(HI(r))
     "1110 1101 pppp pppp 00rr 1100", // mov dbl(p),pair(LO(r))

     "1101 1101 pppp pppp 00rr 1000", //mov dbl(p),r

     "0001 0110 uuuu uuuu uuuu 1001", // MOV #111111,BRC0
     "0111 0110 hhhh hhhh hhhh hhhh rrrr 1000", // MOV #h,r
     "0111 0101 hhhh hhhh hhhh hhhh rrrr uuuu", // MOV #h<<u,r
     "0111 1010 hhhh hhhh hhhh hhhh rrrr 1010", // MOV #h<<16,r
     
     "0010 0010 0rrr 0qqq", // mov ACx,ACy
     "1100 00rr pppp pppp", // mov r,p
     "1110 1011 pppp pppp 00rr 1000", // mov r,DBL(p)
     "1110 1011 pppp pppp 00rr 1001", // MOV SATURATE(r),DBL(p)

     "1101 1101 pppp pppp 00rr 0111", // mov p<<t1,r
     "1101 1101 pppp pppp 10rr 0111", // mov p<<t0,r
     "1101 1101 0011 0001 00rr 0011 0000 0000 hhhh hhhh hhhh hhhh" // MOV *(#h)<<T0,r
     "1101 1111 pppp pppp 00rr 0101", // mov uns(p),r


  };
static gchar *opcode[] = 
{ 
  "MOV p,u",
  "MOV #u,r",
  "MOV #m,r",
  "MOV p,r",
  "MOV DBL(p),r",
  "MOV40 DBL(p),r",
  "MOV DBL(p),PAIR(HI(r))",
  "MOV DBL(p),PAIR(LO(r))",
  "MOV DBL(p),r",
  "MOV #u,BRC0",
  "MOV #h,r",
  "MOV #h<<#u,r",
  "MOV #h<<16,r",
  "MOV r,q",
  "MOV p,r",
  "MOV r,DBL(p)",
  "MOV SATURATE(r),DBL(p)",
  "MOV p<<T1,r",
  "MOV p<<T0,r",
  "MOV *(#h)<<T0,r",
  "MOV UNS(p),r"
};

static void execute(struct _PipeLine *pipeP, struct _Registers *Reg);

Instruction_Class MOV_Obj =
{
  "MOV",
  NULL, // decode
  NULL, // address_stg
  NULL, // access_1
  NULL, // access_2
  NULL, // read_stg
  execute, // execute
  NULL, // write 
  NULL, // write_plus
  20,
  mask,
  opcode,
};



static void execute(struct _PipeLine *pipeP, struct _Registers *Reg)
{
  Word kword;
  int r;
  Opcode opcode;

  opcode = pipeP->decode_nfo.mach_code;

  // affected by C54CM,M40,RDM,SATD,SXMD
  // affect ACOVx

  switch ( pipeP->opcode_subType )
    {
    case 0:
      // MOV k4,dst
      // 1010 00rr 000u uuu0   mov u,ACr
      // 1010 1rrr 000u uuu0   mov u,ARr
      // 1010 010r 000u uuu0   mov u,Tr

      // 1010 0110 000u uuu0   mov u,?
      // 1010 0111 000u uuu0   mov u,?

      r = opcode.wop[0] & 3;
      kword = (opcode.wop[1] & 0x1e)>>1;
      set_s16_ACx(r,kword);
      break;
    case 1:
      // MOV -k4,dst
      // 1010 00rr 1111 1110 mov -1,ACr
      // 1010 00rr 1110 0010 mov -15,ACr   01111 10000 10001
      // 1010 00rr 111u uuu0 mov , u is 
      r = opcode.wop[0] & 3;
      kword = 0xfff0 | ((opcode.wop[1] & 0x1e)>>1);
      set_s16_ACx(r,kword);
      break;
    case 2:
      // MOV k16,dst
      // 0111 0110 uuuu uuuu uuuu uuuu rrrr 1000  mov #u,r
      //   00rr=ACr   1rrr=ARr 010r=Tr   011r=?

      //k = pipeP->current_opcode & 0xff;
      //d = pipeP->current_opcode & 0xf;
      //set_ACx(d,k,Reg);
      break;
    case 3:
      // MOV Smem,dst
      // 1010 00rr ppp0 0001  MOV *ARp,ACr
      // 1010 00rr ppp0 0011  MOV *ARp+,ACr
      // 1010 00rr ppp0 0011  1001 1101  MOV *ARp+%,ACr
      // 1010 00rr ppp0 0101  MOV *ARp-,ACr
      // 1010 00rr ppp0 0101  1001 1101 MOV *ARp-%,ACr
      // 1010 00rr ppp0 0111  MOV *(ARp+t0),ACr
      // 1010 00rr ppp0 1001  MOV *(ARp-t0),ACr
      // 1010 00rr ppp0 1011  MOV *ARp(t0),ACr
      // 1010 00rr ppp0 1101  uuuu uuuu uuuu uuuu MOV *ARp(u),ACr
      // 1010 00rr ppp0 1111  uuuu uuuu uuuu uuuu MOV *+ARp(u),ACr

      // 1010 00rr ppp1 0011  MOV *(ARp+t1),ACr
      // 1010 00rr ppp1 0101  MOV *(ARp-t1),ACr
      // 1010 00rr ppp1 0111  MOV *ARp(t1),ACr
      // 1010 00rr ppp1 1001  MOV *+ARp,ACr
      // 1010 00rr ppp1 1011  MOV *-ARp,ACr
      // 1010 00rr ppp1 1101  MOV *(ARp+t0b),ACr
      // 1010 00rr ppp1 1111  MOV *(ARp-t0b),ACr


      // 1010 0100 pppp ppp1  MOV *ARp,t0

      break;

    case 14:
      // mov40 dbl(Lmem),ACr
      // 1110 1101 pppp ppp1 00rr 1001 mov40 dbl(p),ACr

      // 1010 00rr ppp0 0011 1001 1101             MOV *ARp+%,ACr
      // 1110 1101 ppp0 0011 00rr 1001 1001 1101   mov40 dbl(*ARp+%),ACr
      
    case 30:
      // 0111 0110 0000 0000 1000 0001 0100 1000  mov #129,t0
      // 0111 0110 0000 0000 1000 0001 0101 1000  mov #129,t1
      // 0111 0110 0000 0000 uuuu uuuu 010t 1000  mov #u,t

      // 0001 0110 0000 0011 1111 1001  mov #63,BRC0

      // 1110 1011 0100 0001 0010 1000  mov ac2,dbl(*ar2)
      // 1110 1011 0101 0011 0011 1000  mov ac3,dbl(*(ar2+t1))

      // 1110 1101 0110 0011 0000 1010  mov dbl(*ar3+),pair(HI(ac0))
      // 1110 1101 0110 0011 0000 1100  mov dbl(*ar3+),pair(LO(ac0))

      // 1110 1101 1010 0001 0001 1000  mov dbl(*ar5),ac1
      // 1110 1101 1010 0011 0001 1000  mov dbl(*ar5+),ac1

      // 1110 11rr pppP PPP1 0001 1000

    case 114:
      // mov #k16<<#16,Reg
      // 0111 1010 uuuu uuuu uuuu uuuu rrrr 1010
    case 115:
      // mov #k16,<<#u,Reg
      // 0111 0101 uuuu uuuu uuuu uuuu rrrr uuuu
    }
}


