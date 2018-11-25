#include "registers.h"
#include "memory.h"
typedef uint32_t Um_instruction;
Um_instruction read_instruction(Register r, Um_instruction word, Memory m, 
				uint32_t *counter);