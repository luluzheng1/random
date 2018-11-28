#include "instructions.h"
#include "memory.h"
#include <stdio.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include "assert.h"

#define BYTESIZE 8
#define MEGABYTE 1000000000

//uint32_t read_next_instruction(Memory m, Register r, uint32_t *counter);
typedef enum Um_opcode {
        CMOV = 0, SLOAD, SSTORE, ADD, MUL, DIV,
        NAND, HALT, ACTIVATE, INACTIVATE, OUT, IN, LOADP, LV
} Um_opcode;

int main(int argc, char *argv[])
{
        if (argc != 2){
                fprintf(stderr, "Invalid usage: %s program_name.um\n", argv[0]);
                exit(EXIT_FAILURE);
        }
        FILE *um_file = fopen(argv[1], "r");
        
        struct stat s;
        stat(argv[1], &s);
        int file_size = s.st_size;
        assert(file_size < MEGABYTE);

        memory_new(file_size / 4);
        /* Fill 0 segment with all instructions */
        int num_inst = 0;
        uint32_t instruction = 0;
        for (int i = 0; i < file_size; i++){
                int c = getc(um_file);
                instruction = instruction << BYTESIZE;
                instruction += c;
                /* After reading in 4 chars, add the instruction to 0 segment */
                if ((i % 4 == 3)){
                        set_value_at(0, num_inst, instruction);
                        instruction = 0;
                        num_inst++;
                }
        }
        fclose(um_file);
        int op_code = 0;
	int index = 0;
	uint32_t counter = 0;
	instruction = get_value_at(0, counter);
	
        while (op_code != HALT){
                uint32_t instruction = get_value_at(0, counter);
                op_code = read_instruction(instruction, &counter);
		if(op_code != LOADP){
			counter++;
		}
        }
        memory_free();
}

/*uint32_t read_next_instruction(Memory m, Register r, uint32_t *counter)
{
	uint32_t instruction = get_value_at(m, 0, *counter);
        uint32_t op_code = read_instruction(r, instruction, m, counter);
        return op_code;
}*/