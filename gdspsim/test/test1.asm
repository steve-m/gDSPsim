	.sect ".data"
	
coef:	.word 1 
	.word 5 
	.word 45
x:	.word 7
	.word 6
	.word 3
	
	.sect ".text"

	.global _c_int00
_c_int00:	
	nop
	nop
	nop
	nop
	nop
	ld #2000h,0,a
	stm #2100h,18h	; end of stack 18h=SP stm #2100h,SP doesnt work.

	;; A=x[0]*coef[0] + x[1]*coef[1] + x[2]*coef[2] = 172
	ld *(coef),T
	stm #x,ar2
	stm #(coef+1),ar3
	mpy *ar2+,A
	mac *ar2+,*ar3+,A
	mac *ar2+,*ar3+,A	
	sub #172,0,A,A
	bc failed,ANEQ

	
        ld #86,1,B
        sub #172,0,B,B
        bc failed,BNEQ          ; now B=0

	
	;; A=x[0]*coef[0] + x[1]*coef[1] + x[2]*coef[2] = 172

        ld #1,B                 ; make sure B=0 at end
	stm #x,ar2
	stm #coef,ar3
	ld #0,A
	rpt #2
	mac *ar2+,*ar3+,A
	sub #86,1,A,B
	bc passed,BEQ
	nop
	nop
	nop
failed:
	b failed
	nop
	nop
	nop
	nop
	nop
	nop
passed:	
	b passed
	nop
	nop
	nop
	nop
	nop
	nop
	
