#include "memory.h"
#include "assert.h"
#include "uarray.h"
#include <stdio.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdint.h>

#define BYTESIZE 8
#define MEGABYTE 1000000000
#define REG_WIDTH 3
#define OP_WIDTH 4
#define LSB_OP 28
#define LSB_A 6
#define LSB_B 3
#define LSB_C 0

//uint32_t read_next_instruction(Memory m, Register r, uint32_t *counter);
typedef enum Um_opcode {
        CMOV = 0, SLOAD, SSTORE, ADD, MUL, DIV,
        NAND, HALT, ACTIVATE, INACTIVATE, OUT, IN, LOADP, LV
} Um_opcode;

typedef uint32_t Um_instruction;
typedef enum Um_register {r0 = 0, r1, r2, r3, r4, r5, r6, r7} Um_register;
uint32_t r[8];

Um_instruction r_c, r_a, r_b;
uint32_t val_b, val_c, val_a;
static inline void cmov(Um_instruction word);
static inline void sload(Um_instruction word, uint32_t *counter);
static inline void sstore(Um_instruction word);
static inline void add(Um_instruction word);
static inline void mult(Um_instruction word);
static inline void divide(Um_instruction word);
static inline void nand(Um_instruction word);
static inline void map(Um_instruction word);
static inline void unmap(Um_instruction word);
static inline void out(Um_instruction word);
static inline void in(Um_instruction word);
static inline void loadp(Um_instruction word, uint32_t *counter);
static inline void lv(Um_instruction word);
static inline uint64_t Bitpack_getu(uint64_t word, unsigned width, unsigned lsb);
static inline Um_instruction read_instruction(Um_instruction word, 
					      uint32_t *counter);

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

static inline void cmov(Um_instruction word)
{
	
        r_a = Bitpack_getu(word, REG_WIDTH, LSB_A);
        r_b = Bitpack_getu(word, REG_WIDTH, LSB_B);
        r_c = Bitpack_getu(word, REG_WIDTH, LSB_C);
	uint32_t val_c = r[r_c]; 
	if(val_c != 0)
	{
		uint32_t val_b = r[r_b];
		uint32_t val_a = r[r_a];
		r[r_a] = val_b;
	}
}

static inline void sload(Um_instruction word, uint32_t *counter)
{
        
        r_a = Bitpack_getu(word, REG_WIDTH, LSB_A);
        r_b = Bitpack_getu(word, REG_WIDTH, LSB_B);
        r_c = Bitpack_getu(word, REG_WIDTH, LSB_C);
	
	val_b = r[r_b];
	val_c = r[r_c];
	val_a = get_value_at(val_b, val_c);
	
	r[r_a] = val_a;
}

static inline void sstore(Um_instruction word)
{
        
        r_a = Bitpack_getu(word, REG_WIDTH, LSB_A);
        r_b = Bitpack_getu(word, REG_WIDTH, LSB_B);
        r_c = Bitpack_getu(word, REG_WIDTH, LSB_C);
        
        val_a = r[r_a];
        val_b = r[r_b];
        val_c = r[r_c];

        set_value_at(val_a, val_b, val_c);        
}

static inline void add(Um_instruction word)
{
        uint32_t sum;
        r_a = Bitpack_getu(word, REG_WIDTH, LSB_A);
        r_b = Bitpack_getu(word, REG_WIDTH, LSB_B);
        r_c = Bitpack_getu(word, REG_WIDTH, LSB_C);

        val_b = r[r_b];
        val_c = r[r_c];

        sum = val_b + val_c;           

	r[r_a] = sum;
}

static inline void mult(Um_instruction word)
{
        uint32_t product;
        
        r_a = Bitpack_getu(word, REG_WIDTH, LSB_A);
        r_b = Bitpack_getu(word, REG_WIDTH, LSB_B);
        r_c = Bitpack_getu(word, REG_WIDTH, LSB_C);
        
        val_b = r[r_b];
        val_c = r[r_c];

        product = val_b * val_c;            

	r[r_a] = product;
}

static inline void divide(Um_instruction word)
{
        uint32_t quotient;
        
        r_a = Bitpack_getu(word, REG_WIDTH, LSB_A);
        r_b = Bitpack_getu(word, REG_WIDTH, LSB_B);
        r_c = Bitpack_getu(word, REG_WIDTH, LSB_C);
        
        val_b = r[r_b];
        val_c = r[r_c];

        quotient = val_b / val_c;          

	r[r_a] = quotient;
}

static inline void nand(Um_instruction word)
{
        uint32_t nand;
        
        r_a = Bitpack_getu(word, REG_WIDTH, LSB_A);
        r_b = Bitpack_getu(word, REG_WIDTH, LSB_B);
        r_c = Bitpack_getu(word, REG_WIDTH, LSB_C);

        val_b = r[r_b];
        val_c = r[r_c];

        nand = ~(val_b & val_c);

	r[r_a] = nand;
}

static inline void map(Um_instruction word)
{
        uint32_t id, length;
        
        r_b = Bitpack_getu(word, REG_WIDTH, LSB_B);
        r_c = Bitpack_getu(word, REG_WIDTH, LSB_C);

        length = r[r_c];
        
        id = add_segment(length);
	r[r_b] = id;
}

static inline void unmap(Um_instruction word)
{
        r_c = Bitpack_getu(word, REG_WIDTH, LSB_C);

        val_c = r[r_c];
        remove_segment(val_c);
}

static inline void out(Um_instruction word)
{
	r_c = Bitpack_getu(word, REG_WIDTH, LSB_C);
	val_c = r[r_c];
	fprintf(stdout, "%c", val_c);
}

static inline void in(Um_instruction word)
{
	uint32_t value = fgetc(stdin);
        r_c = Bitpack_getu(word, REG_WIDTH, LSB_C);
	if((int32_t)value != EOF) {
		r[r_c] = value;
	}
	else {
		uint32_t marker = ~0;
		r[r_c] = marker;
	}
}

static inline void loadp(Um_instruction word, uint32_t *counter)
{
	r_b = Bitpack_getu(word, REG_WIDTH, LSB_B);
	r_c = Bitpack_getu(word, REG_WIDTH, LSB_C);
	val_b = r[r_b];
	val_c = r[r_c];
	if(val_b != 0) {
		UArray_T program = (UArray_T)get_segment(val_b);
		put_segment(program);
	}
	get_value_at(0, val_c);
	*counter = val_c;
}

static inline void lv(Um_instruction word)
{
	uint32_t value;
	r_a = Bitpack_getu(word, REG_WIDTH, 25);
	value = Bitpack_getu(word, 25, 0);
	r[r_a] = value;
}

static inline uint64_t Bitpack_getu(uint64_t word, unsigned width, unsigned lsb)
{
        unsigned hi = lsb + width; /* one beyond the most significant bit */
        unsigned bitsl = 64 - hi, bitsr = 64 - width;
        /* left shift */
        if (bitsl != 64)
                word = word << bitsl;
        /* right shift */
        if (bitsr != 64)
                word = word >> bitsr;
        /* ----- */
        return word;
}

static inline Um_instruction read_instruction(Um_instruction word, 
					      uint32_t *counter)
{
	Um_instruction op = Bitpack_getu(word, OP_WIDTH, LSB_OP);
        
	switch(op) {
		case CMOV :
                        cmov(word);
			break;
		case SLOAD :
                        sload(word, counter);
			break;
		case SSTORE :
                        sstore(word);
			break;
		case ADD :
                        add(word);
			break;
		case MUL :
                        mult(word);
			break;
		case DIV :
                        divide(word);
			break;
		case NAND :
                        nand(word);
			break;
		case HALT :
			break;
		case ACTIVATE :
                        map(word);
			break;
		case INACTIVATE :
                        unmap(word);
			break;
		case OUT :
                        out(word);
			break;
		case IN :
                        in(word);
			break;
		case LOADP :
                        loadp(word, counter);
			break;
		case LV :
                        lv(word);
			break;
		default :
			break;
	}
	
	return op;
}