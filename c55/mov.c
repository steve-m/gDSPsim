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
#include <smem.h>


#if 0
0010001E FSSSFDDD    MOV src, dst
0011110E kkkkFDDD    MOV k4, dst
0011111E kkkkFDDD    MOV ­k4, dst
0100000E kkkkFDDD    ADD k4, dst
0100010E 00SSFDDD    MOV HI(ACx), TAx
0100010E 1000FDDD    MOV SP, TAx
0100010E 1001FDDD    MOV SSP, TAx
0100010E 1010FDDD    MOV CDP, TAx
0100010E 1100FDDD    MOV BRC0, TAx
0100010E 1101FDDD    MOV BRC1, TAx
0100010E 1110FDDD    MOV RPTC, TAx
0101001E FSSS00DD    MOV TAx, HI(ACx)
0101001E FSSS1000    MOV TAx, SP
0101001E FSSS1001    MOV TAx, SSP
0101001E FSSS1010    MOV TAx, CDP
0101001E FSSS1100    MOV TAx, CSR
0101001E FSSS1101    MOV TAx, BRC1
0101001E FSSS1110    MOV TAx, BRC0
01110101 KKKKKKKK KKKKKKKK xxDDSHFT    MOV K16 << #SHFT, ACx
01110110 KKKKKKKK KKKKKKKK FDDD10xx    MOV K16, dst
01111000 kkkkkkkk kkkkkkkk xxx0000x    MOV k16, DP
01111000 kkkkkkkk kkkkkkkk xxx0001x    MOV k16, SSP
01111000 kkkkkkkk kkkkkkkk xxx0010x    MOV k16, CDP
01111000 kkkkkkkk kkkkkkkk xxx0011x    MOV k16, BSA01
01111000 kkkkkkkk kkkkkkkk xxx0100x    MOV k16, BSA23
01111000 kkkkkkkk kkkkkkkk xxx0101x    MOV k16, BSA45
01111000 kkkkkkkk kkkkkkkk xxx0110x    MOV k16, BSA67
01111000 kkkkkkkk kkkkkkkk xxx0111x    MOV k16, BSAC
01111000 kkkkkkkk kkkkkkkk xxx1000x    MOV k16, SP
01111010 KKKKKKKK KKKKKKKK xxDD101x    MOV K16 << #16, ACx
10000000 XXXMMMYY YMMM00xx    MOV dbl(Xmem), dbl(Ymem)
10000000 XXXMMMYY YMMM01xx    MOV Xmem, Ymem
10000001 XXXMMMYY YMMM10DD    MOV Xmem, Ymem, ACx
10000111 XXXMMMYY YMMMSSDD 110xxxxx    MOV Xmem << #16, ACy
:: MOV HI(ACx << T2), Ymem
1010FDDD AAAAAAAI    MOV Smem, dst
101100DD AAAAAAAI    MOV Smem << #16, ACx
11011101 AAAAAAAI x%DDss11    MOV [rnd(]Smem << Tx[)], ACx
11011111 AAAAAAAI FDDD000u    MOV [uns(]high_byte(Smem)[)], dst
11011111 AAAAAAAI FDDD001u    MOV [uns(]low_byte(Smem)[)], dst
11011111 AAAAAAAI xxDD010u    MOV [uns(]Smem[)], ACx
11100001 AAAAAAAI DDSHIFTW    MOV low_byte(Smem) << #SHIFTW, ACx
11100010 AAAAAAAI DDSHIFTW    MOV high_byte(Smem) << #SHIFTW, ACx
11100101 AAAAAAAI 000010xx    MOV DP, Smem
11100101 AAAAAAAI 000110xx    MOV CDP, Smem
11100101 AAAAAAAI 001010xx    MOV BSA01, Smem
11100101 AAAAAAAI 001110xx    MOV BSA23, Smem
11100101 AAAAAAAI 010010xx    MOV BSA45, Smem
11100101 AAAAAAAI 010110xx    MOV BSA67, Smem
11100101 AAAAAAAI 011010xx    MOV BSAC, Smem
11100101 AAAAAAAI 011110xx    MOV SP, Smem
11100101 AAAAAAAI 100010xx    MOV SSP, Smem
11100101 AAAAAAAI 100110xx    MOV BK03, Smem
11100101 AAAAAAAI 101010xx    MOV BK47, Smem
11100101 AAAAAAAI 101110xx    MOV BKC, Smem
11100101 AAAAAAAI 110010xx    MOV DPH, Smem
11100101 AAAAAAAI 111110xx    MOV PDP, Smem
11100101 AAAAAAAI x00011xx    MOV CSR, Smem
11100101 AAAAAAAI x00111xx    MOV BRC0, Smem
11100101 AAAAAAAI x01011xx    MOV BRC1, Smem
11100101 AAAAAAAI x01111xx    MOV TRN0, Smem
11100101 AAAAAAAI x10011xx    MOV TRN1, Smem
11100110 AAAAAAAI KKKKKKKK    MOV K8, Smem
11101011 AAAAAAAI xxxx01xx    MOV RETA, dbl(Lmem)
11101101 AAAAAAAI xxxx011x    MOV dbl(Lmem), RETA
11101101 AAAAAAAI xxDD100g    MOV[40] dbl(Lmem), ACx
11101101 AAAAAAAI xxDD101x    MOV dbl(Lmem), pair(HI(ACx))
11101101 AAAAAAAI xxDD110x    MOV dbl(Lmem), pair(LO(ACx))
11101101 AAAAAAAI FDDD111x    MOV dbl(Lmem), pair(TAx)
11101111 AAAAAAAI xxxx00mm    MOV Cmem, Smem
11101111 AAAAAAAI xxxx01mm    MOV Smem, Cmem
11101111 AAAAAAAI xxxx10mm    MOV Cmem,dbl(Lmem)
11101111 AAAAAAAI xxxx11mm    MOV dbl(Lmem), Cmem
11111001 AAAAAAAI uxSHIFTW xxDD10xx    MOV [uns(]Smem[)] << #SHIFTW, ACx
11111011 AAAAAAAI KKKKKKKK KKKKKKKK    MOV K16, Smem

#endif



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
      set_k16_reg(r,kword,1);
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


