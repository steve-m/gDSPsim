#ifndef __HARDWARE_H__
#define __HARDWARE_H__

#include "c54_core.h"

GP_Reg alu_add(GP_Reg op1, SWord op2, SWord op2_shift, struct _Registers *Registers);

// This converts op to a GP_Reg, then shifts it
GP_Reg word_to_GP_Reg(SWord op, SWord op_shift);

// This shifts op, then converts the low bits to a word
Word GP_Reg_Low_to_Word(GP_Reg op, SWord op_shift);


// This updates the auxillary registers and returns the read address
Word update_smem( int Smem, struct _Registers *Reg);

// Decodes arf and returns a pointer to the ARx pointer reg.
// CMPT is the compatibility mode flag
Word *get_pointer_reg(int arf, struct _Registers *Reg, int CMPT);

void set_pointer_reg(int arf, Word value, struct _Registers *Reg);

// This returns a pointer to a memory mapped register. The code
// can be from 0 to 8
Word *get_mmr(Word mm_code, struct _Registers *Reg);

// This sets a memory mapped register.
void set_mmr(Word mm_code, Word value, struct _Registers *Reg);
#endif // #ifndef __HARDWARE_H__
