//
// Part of traqmate SU and DU
//
// This infinite loop is placed before the code reflash routine in order to stop runaway
// software or a random program counter startup load from killing the code.
#pragma ASM
ROADBLOCK	SEGMENT	CODE
	RSEG	ROADBLOCK
	JMP	$	// endless loop to protect against accidental code erase
#pragma ENDASM