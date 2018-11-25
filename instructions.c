#include "instructions.h"
#include "uarray.h"
#include "assert.h"
#include <stdio.h>

#define REG_WIDTH 3
#define OP_WIDTH 4
#define LSB_OP 28
#define LSB_A 6
#define LSB_B 3
#define LSB_C 0

typedef enum Um_opcode {
        CMOV = 0, SLOAD, SSTORE, ADD, MUL, DIV,
        NAND, HALT, ACTIVATE, INACTIVATE, OUT, IN, LOADP, LV
} Um_opcode;
typedef uint32_t Um_instruction;

static inline void three_registers(Um_instruction word, uint32_t *r_a, 
                                   uint32_t *r_b, uint32_t *r_c);
static inline void cmov(Register r, Um_instruction word);
static inline void sload(Register r, Um_instruction word, Memory m, 
                         uint32_t *counter);
static inline void sstore(Register r, Um_instruction word, Memory m);
static inline void add(Register r, Um_instruction word);
static inline void mult(Register r, Um_instruction word);
static inline void div(Register r, Um_instruction word);
static inline void nand(Register r, Um_instruction word);
static inline void map(Register r, Um_instruction word, Memory m);
static inline void unmap(Register r, Um_instruction word, Memory m);
static inline void out(Register r, Um_instruction word);
static inline void in(Register r, Um_instruction word);
static inline void loadp(Register r, Um_instruction word, Memory m, 
                         uint32_t *counter);
static inline void lv(Register r, Um_instruction word);
static inline uint64_t Bitpack_getu(uint64_t word, unsigned width, unsigned lsb);

Um_instruction read_instruction(Register r, Um_instruction word, Memory m, 
				uint32_t *counter)
{
	Um_instruction op = Bitpack_getu(word, OP_WIDTH, LSB_OP);
        
	switch(op) {
		case CMOV :
                        cmov(r, word);
			break;
		case SLOAD :
                        sload(r, word, m, counter);
			break;
		case SSTORE :
                        sstore(r, word, m);
			break;
		case ADD :
                        add(r, word);
			break;
		case MUL :
                        mult(r, word);
			break;
		case DIV :
                        div(r, word);
			break;
		case NAND :
                        nand(r, word);
			break;
		case HALT :
			break;
		case ACTIVATE :
                        map(r, word, m);
			break;
		case INACTIVATE :
                        unmap(r, word, m);
			break;
		case OUT :
                        out(r, word);
			break;
		case IN :
                        in(r, word);
			break;
		case LOADP :
                        loadp(r, word, m, counter);
			break;
		case LV :
                        lv(r, word);
			break;
		default :
			break;
	}
	
	return op;
}

/* Function: three_registers
 * Gets register numbers A,B,C from a 32-bit word 
 * Parameters: Um_instruction, uint32_t*, uint32_t*, uint32_t*
 * Return: nothing
 */
static inline void three_registers(Um_instruction word, uint32_t *r_a, 
                                   uint32_t *r_b, uint32_t *r_c)
{
        *r_a = Bitpack_getu(word, REG_WIDTH, LSB_A);
        *r_b = Bitpack_getu(word, REG_WIDTH, LSB_B);
        *r_c = Bitpack_getu(word, REG_WIDTH, LSB_C);
}
	
static inline void cmov(Register r, Um_instruction word)
{
	Um_instruction r_c, r_a, r_b;
	
        three_registers(word, &r_a, &r_b, &r_c);
	uint32_t val_c = get_register_at(r, r_c); 
	if(val_c != 0)
	{
		uint32_t val_b = get_register_at(r, r_b);
		uint32_t val_a = get_register_at(r, r_a);
		set_register_at(r, r_a, val_b);
	}
}

static inline void sload(Register r, Um_instruction word, Memory m, 
                         uint32_t *counter)
{
	Um_instruction r_a, r_b, r_c;
	uint32_t val_b, val_c, val_m;
        
        three_registers(word, &r_a, &r_b, &r_c);
	
	val_b = get_register_at(r, r_b);
	val_c = get_register_at(r, r_c);
	val_m = get_value_at(m, val_b, val_c);
	
	set_register_at(r, r_a, val_m);
}

static inline void sstore(Register r, Um_instruction word, Memory m)
{
        Um_instruction r_a, r_b, r_c;
        uint32_t val_b, val_c, val_a;
        
        three_registers(word, &r_a, &r_b, &r_c);
        
        val_a = get_register_at(r, r_a);
        val_b = get_register_at(r, r_b);
        val_c = get_register_at(r, r_c);

        set_value_at(m, val_a, val_b, val_c);        
}

static inline void add(Register r, Um_instruction word)
{
        Um_instruction r_a, r_b, r_c;
        uint32_t val_b, val_c, val_a, sum;
        
        three_registers(word, &r_a, &r_b, &r_c);

        val_b = get_register_at(r, r_b);
        val_c = get_register_at(r, r_c);

        sum = val_b + val_c;           

        set_register_at(r, r_a, sum);
}

static inline void mult(Register r, Um_instruction word)
{
        Um_instruction r_a, r_b, r_c;
        uint32_t val_b, val_c, val_a, product;
        
        three_registers(word, &r_a, &r_b, &r_c);
        
        val_b = get_register_at(r, r_b);
        val_c = get_register_at(r, r_c);

        product = val_b * val_c;            

        set_register_at(r, r_a, product);
}

static inline void div(Register r, Um_instruction word)
{
        Um_instruction r_a, r_b, r_c;
        uint32_t val_b, val_c, val_a, quotient;
        
        three_registers(word, &r_a, &r_b, &r_c);
        
        val_b = get_register_at(r, r_b);
        val_c = get_register_at(r, r_c);

        quotient = val_b / val_c;          

        set_register_at(r, r_a, quotient);
}

static inline void nand(Register r, Um_instruction word)
{
        Um_instruction r_a, r_b, r_c;
        uint32_t val_b, val_c, val_a, nand;
        
        three_registers(word, &r_a, &r_b, &r_c);

        val_b = get_register_at(r, r_b);
        val_c = get_register_at(r, r_c);

        nand = ~(val_b & val_c);

        set_register_at(r, r_a, nand);
}

static inline void map(Register r, Um_instruction word, Memory m)
{
        Um_instruction r_b, r_c;
        uint32_t val_b, id, length;
        
        r_b = Bitpack_getu(word, REG_WIDTH, LSB_B);
        r_c = Bitpack_getu(word, REG_WIDTH, LSB_C);

        length = get_register_at(r, r_c);
        
        id = add_segment(m, length);

        set_register_at(r, r_b, id);
}

static inline void unmap(Register r, Um_instruction word, Memory m)
{
        Um_instruction r_c;
        uint32_t val_c;
        r_c = Bitpack_getu(word, REG_WIDTH, LSB_C);

        val_c = get_register_at(r, r_c);
        remove_segment(m, val_c);
}

static inline void out(Register r, Um_instruction word)
{
	Um_instruction r_c = Bitpack_getu(word, REG_WIDTH, LSB_C);
	uint32_t val_c = get_register_at(r, r_c);
	fprintf(stdout, "%c", val_c);
}

static inline void in(Register r, Um_instruction word)
{
	uint32_t value = fgetc(stdin);
	uint32_t r_c;
        r_c = Bitpack_getu(word, REG_WIDTH, LSB_C);
	if((int32_t)value != EOF) {
        	set_register_at(r, r_c, value);
	}
	else {
		uint32_t marker = ~0;
		set_register_at(r, r_c, marker);
	}
}

static inline void loadp(Register r, Um_instruction word, Memory m, 
                         uint32_t *counter)
{
	Um_instruction r_b, r_c;
	uint32_t val_b, val_c;
	r_b = Bitpack_getu(word, REG_WIDTH, LSB_B);
	r_c = Bitpack_getu(word, REG_WIDTH, LSB_C);
	val_b = get_register_at(r, r_b);
	val_c = get_register_at(r, r_c);
	if(val_b != 0) {
		UArray_T program = (UArray_T)get_segment(m, val_b);
		put_segment(m, program);
	}
	get_value_at(m, 0, val_c);
	*counter = val_c;
}

static inline void lv(Register r, Um_instruction word)
{
	Um_instruction r_a;
	uint32_t value;
	r_a = Bitpack_getu(word, REG_WIDTH, 25);
	value = Bitpack_getu(word, 25, 0);
	set_register_at(r, r_a, value);
}

static inline uint64_t Bitpack_getu(uint64_t word, unsigned width, unsigned lsb)
{
        unsigned hi = lsb + width; /* one beyond the most significant bit */
        unsigned bitsl = 64 - hi, bitsr = 64 - width;
        assert(hi <= 64);
        assert(bitsl <= 64);
        assert(bitsr <= 64);
        /* left shift */
        if (bitsl != 64)
                word = word << bitsl;
        /* right shift */
        if (bitsr != 64)
                word = word >> bitsr;
        /* ----- */
        return word;
}
